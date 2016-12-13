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
// is the return value indicating success/failure.
struct thread_output {
    // Note that `ret` defaults to false, which means that it
    // assumes failure unless explicitely changed to true.
    thread_output()
        : watch(), files( 0 ), bytes( 0 ), tmp_files( 0 )
        , ret( false ) {}
    // The thread will record timing info here so that we can
    // e.g. understand the runtime actually spent in each thread.
    StopWatch            watch;
    // Total number of files written by this thread.  This is
    // for diagnostics and sanity checking.
    size_t               files;
    // Total number of bytes written by this thread.  This is
    // for diagnostics and sanity checking.
    uint64_t             bytes;
    // Total number of files that were written to temporary
    // files during extraction;
    size_t               tmp_files;
    // The return value of the thread (success/failure) is here.
    // This is used because we don't allow exceptions to leave
    // the threads.
    bool                 ret;
};

// Function signature to use to map an archived file name onto
// a temporary file name used when extracting the data.
using NameMap = function<string( string const& )>;

/****************************************************************
* This is the function that will be given to each of the thread
* objects.  It will create a new zip_source_t and zip_t objects
* out of the original zip-file buffer (since we don't know if
* we can share a zip_t object among threads) and it will then
* proceed to extract the files that it has been assigned,
* represented by the vector of indices into the list of
* archived files.
****************************************************************/
void unzip_worker( size_t                  thread_idx,
                   Buffer::SP&             zip_buffer,
                   vector<uint64_t> const& idxs,
                   size_t                  chunk_size,
                   bool                    quiet,
                   TSXFormer               ts_xform,
                   NameMap const&          get_tmp_name,
                   thread_output&          data )
{
    // This mutex protects logging of file names during unzip.
    // Without this lock the various threads' printing would
    // conflict and lead to messy output.
    static mutex log_name_mtx;

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
        uint64_t size = zip[idx].size();
        // If the caller chooses, we log the name of the file
        // being unzipped by protect the logging with a mutex
        // otherwise different threads will step on each other
        // causing jumbled output.
        if( !quiet ) {
            lock_guard<mutex> lock( log_name_mtx );
            cerr << left << setw( 4 ) <<
                to_string( thread_idx ) + "> " << name << endl;
        }
        // Allow the caller to specify a temporary name for
        // the file while it is being extracted.  If the
        // callback function returns a name different from
        // the input name then the file will be written to
        // the temporary name while being extracted, then
        // will be renamed afterward.  It could be used to
        // support atomicity of extraction as well as the
        // "small extension optimization."
        auto tmp_name( get_tmp_name( name ) );
        // Keep track of how many we're actually renaming.
        data.tmp_files += ( tmp_name == name ) ? 0 : 1;
        // Decompress the data and write it to the file in
        // chunks of size equal to uncompressed.size().
        zip.extract_to( idx, tmp_name, uncompressed );
        // This function guarantees that it will do nothing if
        // the two file names are equal.
        rename_file( tmp_name, name );
        // Now take the time stored in the zip archive, pass
        // it through the user supplied transformation function,
        // and store the result if there is one.
        time_t time = ts_xform( zip[idx].mtime() );
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
    , folders( 0 )
    , num_temp_names( 0 )
    , watch()
    , watches( jobs )
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
    if( us.folders > 0 )
        key( "ratio " ) << double( us.files ) / us.folders << endl;
    key( "tmp names" )  << us.num_temp_names << endl;
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

// The idea of this function is to take an arbitrary input
// string and then hash it, where the hash is a three-character
// string made up only of characters suitable for a file
// extension.  Basically we just hash the string to a 32 bit
// number, then use each of the first three bytes in the hash
// to select three characters from a list to form a three
// character string.  There are about 46k possible results.
string ext3( string const& s ) {
    // List of all chars that we will use when generating file
    // extensions.  We don't include uppercase letters because on
    // Windows/OSX file names are case-insensitive.
    static string const cs( "abcdefghijklmnopqrstuvwxyz0123456789" );
    static size_t const size( cs.size() );
    // Helper to get the nth element of string modulo its size.
    static auto get = [&]( size_t n ){ return cs[n % size]; };
    // Compute the hash
    uint32_t h = string_hash( s );
    return string( { get( h ), get( h>>8 ), get( h>>16 ) } );
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

    // If this is zero then we'll go into an endless loop writing
    // chunks of size zero to disk.
    FAIL( chunk_size < 1, "Invalid chunk size: " << chunk_size );
    res.chunk_size_used = chunk_size;

    /************************************************************
    * Create the `temp name map` function
    *************************************************************
    * Establish a function that takes a archived file name and
    * maps it to another name.  This new name is used as the
    * temporary location to use when extracting/writing the file.
    * After extraction is complete it will be renamed to the
    * proper name.
    *
    * There could potentially be multiple uses for this mapping
    * function, but here we're using it accomplish something
    * kind of odd.
    *
    * We're going to map files to temp names, but only ones
    * whose names meet certain criteria.  Various empirical
    * observations were made on Windows machines running
    * Symantec Anti-Virus software.  It appears that this AV
    * software has a negative affect on file creation time in
    * general, but particularly so for files whose names
    * contain extensions longer than three characters.
    *
    * So, when an archived file has an extension longer than
    * three chars we will, instead of extracting it directly
    * as for most other files, we will extract it to a temporary
    * file with an extention == three chars.  Then, after we
    * are finished extracting, we will rename it to the original
    * name.  For some mysterious reason, this can significantly
    * boost performance on the Windows desktop machines on which
    * measurements were taken (at the time of this writing).
    *
    * And it gets even stranger... if the filename begins
    * with a dot we will keep it as is... this comes from
    * empirical observations that suggest that mapping files
    * that start with a dot (even when they have an extension
    * > 3 chars) actually slows it down again. */

    // The default mapping does nothing.
    NameMap get_tmp_name = id<string>;

    if( short_exts ) {
        // This function must be thread safe!
        get_tmp_name = []( string const& input ) {
            // Must use FilePath variant of split_ext here because
            // the string variant could potential split on a dot
            // in a parent folder.  NOTE: first component (if
            // there is one) contains the dot at the end!
            auto opt_p = split_ext( FilePath( input ) );
            if( !opt_p ) return input;
            auto const& fst = opt_p.get().first;
            string snd( opt_p.get().second.str() );
            return ( fst.basename() != "." && snd.size() > 3 )
                   ? fst.add_ext( ext3( snd ) ).str()
                   : input;
        };
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
                             ref( get_tmp_name ),
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
        res.files          += o.files;
        res.bytes          += o.bytes;
        res.num_temp_names += o.tmp_files;
        // Per-thread stuff
        res.files_ts[job]   = o.files;
        res.bytes_ts[job]   = o.bytes;
        res.watches[job]    = move( o.watch );
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

    uint64_t total_bytes_in_zip = 0;
    for( auto const& zs : files )
        total_bytes_in_zip += zs.size();

    FAIL_( total_bytes_in_zip != res.bytes );

    return res;
}
