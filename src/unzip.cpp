/****************************************************************
* Implementation of the API for the parallel unzip functionality.
****************************************************************/
#include "distribution.hpp"
#include "unzip.hpp"
#include "zip.hpp"

#include <algorithm>
#include <iomanip>
#include <mutex>
#include <thread>

using namespace std;

namespace {

// This is the structure that is used to return various pieces
// of data to the caller from a single thread.  Among this data
// is the return value.
struct thread_output {

    thread_output()
        : watch(), files( 0 ), bytes( 0 ), ret( false ) {}

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
void unzip_worker( size_t                    thread_idx,
                   Buffer::SP&               zip_buffer,
                   vector<size_t> const&     idxs,
                   size_t                    chunk_size,
                   bool                      quiet,
                   TSXFormer                 ts_xform,
                   map<string,string> const& tmp_names,
                   thread_output&            data )
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
            LOG( left << setw( 4 ) <<
                 to_string( thread_idx ) + "> " << name );
        }
        // Allow the caller to specify a temporary name for the
        // file while it is being extracted.  If there is such a
        // temp name specified then the temporary file will be
        // renamed to the correct name after that file's extraction
        // is complete.  This functionality is optional and is
        // effectively turned off by leaving the map empty.  It
        // could be used to support atomicity of extraction as
        // well as the "small extension optimization."
        auto tmp_name( map_get( tmp_names, name, name ) );
        // Decompress the data and write it to the file in
        // chunks of size equal to uncompressed.size().
        zip.extract_to( idx, tmp_name, uncompressed );
        // This function guarantees that it will do nothing if
        // the two file names are equal.
        rename_file( tmp_name, name );
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
    : filename()
    , jobs_used( jobs )
    , strategy_used()
    , chunk_size_used( 0 )
    , files( 0 )
    , files_ts( jobs )
    , bytes( 0 )
    , bytes_ts( jobs )
    , watch()
    , watches( jobs )
{}

// VS 2013 can't generate default move constructors
UnzipSummary::UnzipSummary( UnzipSummary&& from )
    : filename( move( from.filename ) )
    , jobs_used( from.jobs_used )
    , strategy_used( move( from.strategy_used ) )
    , chunk_size_used( from.chunk_size_used )
    , files( from.files )
    , files_ts( move( from.files_ts ) )
    , bytes( from.bytes )
    , bytes_ts( move( from.bytes_ts ) )
    , watch( move( from.watch ) )
    , watches( move( from.watches ) )
{}

// For convenience; will print out all the fields in a nice
// human readable/pretty format.
ostream& operator<<( ostream& out, UnzipSummary const& us ) {

    #define BYTES( a ) left << setw(11) << a <<        \
                       left << setw(11) <<             \
                       (" (" + human_bytes( a ) + ")")

    auto key = [&]( string const& s ) -> ostream& {
        out << left << setw(17) << s << ": ";
        return out;
    };

    key( "file" )       << us.filename << endl;
    key( "jobs" )       << us.jobs_used << endl;
    key( "strategy" )   << us.strategy_used << endl;
    key( "files" )      << us.files << endl;
    key( "folders" )    << us.folders << endl;
    key( "ratio " )     << double( us.files ) / us.folders << endl;
    key( "max size" )   << BYTES( us.max_size ) << endl;
    key( "chunk" )      << us.chunk_size_used << endl;
    key( "chunks_mem" ) << BYTES( us.chunk_size_used*us.jobs_used )
                           << endl;

    size_t jobs = us.watches.size();

    out << endl;
    for( size_t i = 0; i < jobs; ++i ) {
        key( "files: thread " + to_string( i+1 ) ) <<
            left << setw(22) << us.files_ts[i] << " [" <<
            us.watches[i].human( "unzip" ) << "]" << endl;
    }

    key( "files: total") << us.files << endl;

    out << endl;
    for( size_t i = 0; i < jobs; ++i ) {
        key( "bytes: thread " + to_string( i+1 ) ) <<
            BYTES( us.bytes_ts[i] ) << " [" <<
            us.watches[i].human( "unzip" ) << "]" << endl;
    }

    key( "bytes: total" ) << BYTES( us.bytes ) << endl;

    out << endl;
    // Output all the times that were measured but put "total"
    // last.
    for( auto&& result : us.watch.results() )
        if( result.first != "total" )
            key( "time: " + result.first ) << result.second <<
            endl;
    key( "time: total" ) << us.watch.human( "total" ) << endl;
    return out;
}

/****************************************************************
* Main interface for parallel unzip.
****************************************************************/
UnzipSummary p_unzip( string    filename,
                      size_t    jobs,
                      bool      quiet,
                      string    strategy,
                      size_t    chunk_size,
                      TSXFormer ts_xform,
                      bool      short_exts )
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
    res.chunk_size_used = chunk_size;

    /************************************************************
    * Create the `temp name map`
    *************************************************************
    * The name map allows us to map the name of an archived entry
    * to a new name.  When the file is created it will be created
    * with the new name, then will be renamed to the original
    * name after having been extracted. */
    map<string, string> tmp_names;

    // There could potentially be multiple uses for this map, but
    // here we're using it accomplish something kind of odd. When
    // an archived file has an extension longer than three chars
    // we will, instead of extracting it directly as for most
    // other files, we will extract it to a temporary file with
    // an extention <= three chars.  For some mysterious reason,
    // this can boost performance on Windows versus due to some
    // strange dependence of the CreateFile performance on the
    // number of characters in the extension.
    if( short_exts ) {
        res.watch.start( "short_exts" );
        // Map used to hold mappings from long to short
        // extensions so that we always use the same mapping
        // for a given extension.
        map<string, string> m;

        size_t long_exts = 0;
        for( auto const& f : files ) {
            FilePath fp( f.name() );
            if( starts_with( fp.basename(), '.' ) )
                continue;
            // Must use FilePath variant of split_ext here because
            // the string variant could potential split on a dot
            // in a parent folder.  NOTE: first component (if
            // there is one) contains the dot at the end!
            auto opt_p = split_ext( fp );
            if( !opt_p )
                // No extension in file name.
                continue;
            auto const& ext = opt_p.get().second.str();
            if( ext.size() <= 3 )
                continue;
            if( !has_key( m, ext ) ) {
                FAIL( long_exts >= 1000,
                    "number of unique long extensions > 1000" );
                m[ext] = to_string( long_exts++ );
            }
            tmp_names[f.name()] =
                opt_p.get().first.add_ext( m[ext] ).str();
        }
        res.watch.stop( "short_exts" );
    }

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
    res.strategy_used = strategy;

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
                             ref( zip_buffer ),
                             ref( thread_idxs[i] ),
                             chunk_size,
                             quiet,
                             ts_xform,
                             ref( tmp_names ),
                             ref( outputs[i] ) );

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
    // Ensure that both files and bytes were initialized to zero.
    FAIL_( res.files != 0 || res.bytes != 0 );
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

    res.folders   = folders.size();
    res.filename  = filename;
    res.jobs_used = jobs;

    size_t total_bytes_in_zip = 0;
    for( auto const& zs : files )
        total_bytes_in_zip += zs.size();

    FAIL_( total_bytes_in_zip != res.bytes );

    return res;
}
