/****************************************************************
* Implementation of the API for the parallel unzip functionality.
****************************************************************/
#include "distribution.hpp"
#include "unzip.hpp"
#include "zip.hpp"

#include <algorithm>
#include <iomanip>
#include <mutex>

using namespace std;

namespace {

// This is the structure that is used to return various pieces
// of data to the caller from a single thread.  Among this data
// is the return value.
struct thread_output {
    // The thread will record timing info here so that we can
    // e.g. understand the runtime actually spent in each thread.
    StopWatch            watch;
    // Total number of files written by this thread.  This is
    // for diagnostics and sanity checking.
    size_t               files;
    // Total number of bytes written by this thread.  This is
    // for diagnostics and sanity checking.
    size_t               bytes;
    // The return value of the thread (success/failure) is here.
    // This is used because we don't allow exceptions to leave
    // the threads.
    bool                 ret;
};

/****************************************************************
* This is the function that will be given to each of the thread
* objects.  It will create a new zip_source_t and zip_t objects
* out of the original zip-file buffer (since we don't know if
* we can share a zip_t object among threads) and it will then
* proceed to extract the files that it has been assigned,
* represented by the vector of indices into the list of
* archived files.
****************************************************************/
void unzip_worker( size_t                thread_idx,
                   Buffer::SP&           zip_buffer,
                   vector<size_t> const& idxs,
                   size_t                chunk_size,
                   bool                  quiet,
                   TSXFormer             ts_xform,
                   thread_output&        data )
{
    // This mutex protects logging of file names during unzip.
    // Without this lock the various threads' printing would conflict
    // and lead to messy output.
    static mutex log_name_mtx;

    data.ret = false; // assume we fail unless we succeed.
    TRY
    // Start the clock.  Each thread reports its total runtime.
    data.watch.start( "unzip" );
    // Create the zip here because we don't know if libzip or
    // zlib are thread safe.  All that the Zip creation will
    // do here is to change the ref count on the buffer which
    // is thread safe since it's a shared_ptr.
    Zip zip( zip_buffer );
    // Allocate a new buffer for use only within this thread
    // that is large enough to hold any single uncompressed
    // file in the archive.
    Buffer uncompressed( chunk_size );
    // Now just loop over each entry
    for( auto idx : idxs ) {
        // This will be the file name.  It should never be
        // a folder name (i.e., ending in forward slash)
        // since those should have already been filtered out
        // and pre-created.
        string name( zip[idx].name() );
        // Get size of the uncompressed data of entry.
        size_t size = zip[idx].size();
        // If the caller chooses, we log the name of the file
        // being unzipped by protect the logging with a mutex
        // otherwise different threads will step on each other
        // causing jumbled output.
        if( !quiet ) {
            lock_guard<mutex> lock( log_name_mtx );
            LOG( thread_idx << setw( 2 ) << "> " << name );
        }
        // Decompress the data and write it to the file in
        // chunks of size equal to uncompressed.size().
        zip.extract_to( idx, name, uncompressed );
        // Now take the time stored in the zip archive, pass
        // it through the user supplied transformation function,
        // and store the result if there is one.
        size_t time = ts_xform( zip[idx].mtime() );
        if( time )
            set_timestamp( name, time );
        // For auditing / sanity checking purposes.
        data.files++; data.bytes += size;
    }

    data.ret = true; // return success
    CATCH_ALL
    // Stop the clock; we will always get here even on exception.
    data.watch.stop("unzip");
}

} // anon namespace
 
/****************************************************************
* UnzipSummary: structure used for communicating diagnostic info
* collected during the unzip process back to the caller.
****************************************************************/
UnzipSummary::UnzipSummary( size_t jobs )
    : files( 0 )
    , files_ts( jobs )
    , bytes( 0 )
    , bytes_ts( jobs )
    , watch()
    , watches( jobs )
{}

// VS 2013 can't generate default move constructors
UnzipSummary::UnzipSummary( UnzipSummary&& from )
    : files( from.files )
    , bytes( from.bytes )
    , watch( move( from.watch ) )
    , watches( move( from.watches ) )
{}

/****************************************************************
* Main interface for parallel unzip.
****************************************************************/
UnzipSummary p_unzip( string    filename,
                      bool      quiet,
                      size_t    jobs,
                      string    strategy,
                      size_t    chunk_size,
                      TSXFormer ts_xform )
{
    // This will collect info and will be returned at the end.
    UnzipSummary res( jobs );

    // Start the clock to measure total unzip time including the
    // preparation work.
    res.watch.start( "total" ); // End program runtime.

    res.watch.start( "load_zip" );
    // Open the zip file, read it completely into a buffer,
    // then manage the buffer with a shared pointer.  This is
    // because the buffer will be used possibly by many zip
    // objects, and we must ensure that it stays alive until
    // they are all finished.
    Buffer::SP zip_buffer =
        make_shared<Buffer>( File( filename, "rb" ).read() );

    // This will take the contents of the zip file (which are
    // now in the buffer) and will scan the tables at the end
    // of the zip to gather information about the stats of
    // the files in the archive.  However, it will not do any
    // decompression or extraction.
    Zip z( zip_buffer );

    // Now find the maximum (uncompressed) size of all the
    // entries in the archive.
    auto getter  = []( ZipStat const& zs ) { return zs.size(); };
    res.max_size = getter( maximum( z.begin(), z.end(), getter ) );

    // Create a copy of z's underlying vector of ZipStats so
    // that we can reorder them.  Note that the resultant vector
    // will have views taken off of it so must remain alive.
    vector<ZipStat> stats( z.begin(), z.end() );
    auto folders_end = partition( stats.begin(), stats.end(),
        []( ZipStat const& zs ){ return zs.is_folder(); });
    auto folders = make_range( stats.begin(), folders_end );
    auto files   = make_range( folders_end,   stats.end() );

    // Time how long it takes to load the zip and handle the
    // ZipStat data structures.
    res.watch.stop( "load_zip" );

    if( !chunk_size )
        // Use the max file size in the zip as chunk size.
        // Warning: this can cause allocation failures.
        chunk_size = res.max_size;
    // If the archive consists of only empty elements then we
    // are allowed to have a chunk size of zero, otherwise it
    // must be non-zero.
    FAIL( res.max_size > 0 && chunk_size < 1,
        "Invalid chunk size" );

    /************************************************************
    * Pre-create folder structure
    *************************************************************
    * In a parallel unzip we must pre-create all of the folders
    * that are mentioned in the zip file either explicitely or
    * implicitely.  If we attempt to do this within the worker
    * threads then we would likely have race conditions and
    * write conflicts.  So this way, when the worker threads
    * start, all of the necessary folders are already there.
    * First, we will gather all the folder names that are
    * mentioned both in the archive explicitly through folder
    * entries or implicitely as the paths to files. */
    vector<FilePath> fps;
    for( auto const& zs : stats )
        fps.push_back( zs.folder() );
    // Now we ensure that each one exists.
    res.watch.run( "folders", [&]{ mkdirs_p( fps ); } );

    /************************************************************
    * Distribution of files to the threads
    ************************************************************/
    FAIL( !has_key( distribute, strategy ),
        "strategy " << strategy << " is invalid." );
    // Do the distribution.  The result should be a vector of
    // length equal to the number of jobs.  Each element should
    // itself be a vector if indexs representing files assigned
    // to that thread for extraction.
    index_lists thread_idxs;
    res.watch.run( "distribute", [&]{
        thread_idxs = distribute[strategy]( jobs, files );
    });
    FAIL_( thread_idxs.size() != jobs );

    /************************************************************
    * Start multithreaded unzip
    *************************************************************
    * These will be populated by the threads as the work and
    * and then checked at the end as a sanity check. */
    vector<thread_output> outputs( jobs );
    vector<thread>        threads( jobs );

    res.watch.start( "unzip" );

    // Spawn each thread
    for( size_t i = 0; i < jobs; ++i )
        threads[i] = thread( unzip_worker,
                             i,
                             std::ref( zip_buffer ),
                             std::ref( thread_idxs[i] ),
                             chunk_size,
                             quiet,
                             ts_xform,
                             std::ref( outputs[i] ) );
    // Wait for everything to finish.
    for( auto& t : threads )
        t.join();

    res.watch.stop( "unzip" );

    for( size_t i = 0; i < jobs; ++i )
        FAIL_( !outputs[i].ret );

    /************************************************************
    * Sanity/Error checking
    ************************************************************/
    res.watch.stop( "total" ); // End program runtime.

    size_t job = 0;
    for( auto const& o : outputs ) {
        // Aggregate stuff
        res.files += o.files;
        res.bytes += o.bytes;
        // Per-thread stuff
        res.files_ts[job] = o.files;
        res.bytes_ts[job] = o.bytes;
        res.watches[job]  = move( o.watch );
        ++job;
    }

    // Make sure that the sum of files counts for each thread
    // equals the total number of files in the zip.  This is the
    // reason that we are not just writing files.size() to get
    // res.files.
    FAIL_( res.files != files.size() );

    res.folders = folders.size();

    size_t total_bytes_in_zip = 0;
    for( auto const& zs : files )
        total_bytes_in_zip += zs.size();

    FAIL_( total_bytes_in_zip != res.bytes );

    return res;
}

/****************************************************************
* This is an interface to the parallel unzip functionality with
* commonly used defaults for most arguments.
****************************************************************/
void p_unzip_basic( std::string filename, size_t jobs ) {
    // Do not print file names are they are unzipped.
    bool quiet      = true;
    // Strategy for distibuting archived files to the threads.
    string strategy = DEFAULT_DIST;
    // Files are decompressed/written in chunks of this size.
    size_t chunk    = DEFAULT_CHUNK;
    // Use the identity function for transforming timestamps.
    // This means use the time archived in the zip directly,
    // which ignores timezone.
    auto id = []( time_t t ){ return t; };
    // Run a full parallel unzip and ignore return value (which
    // is diagnostic info).
    p_unzip( filename, quiet, jobs, strategy, chunk, id );
}
