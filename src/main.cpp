/****************************************************************
 * p-unzip: the multithreaded unzipper
 *
 * author:  David P. Sicilia
 *
 * This program will take a zip file as input and a number of
 * threads and it will distribute the files in the archive among
 * the threads in the specified way in order to take advantage of
 * the opportunity for parallelism while unzipping an archive.
 ***************************************************************/
#include "config.hpp"
#include "distribution.hpp"
#include "fs.hpp"
#include "options.hpp"
#include "utils.hpp"
#include "zip.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <numeric>
#include <string>
#include <thread>
#include <iomanip>

using namespace std;

// Positional args should always be referred to by these.
size_t const ARG_FILE_NAME = 0;

/****************************************************************
 * This is the function that will be given to each of the thread
 * objects.  It will create a new zip_source_t and zip_t objects
 * out of the original zip-file buffer (since we don't know if
 * we can share a zip_t object among threads) and it will then
 * proceed to extract the files that it has been assigned,
 * represented by the vector of indices into the list of
 * archived files.
 ***************************************************************/
void unzip( size_t                thread_idx, // input
            Buffer::SP&           zip_buffer, // input
            vector<size_t> const& idxs,       // input
            size_t                chunk_size, // input
            bool                  quiet,      // input
            StopWatch&            watch,      // output
            size_t&               files,      // output
            size_t&               bytes,      // output
            bool&                 ret )       // output
{
    ret = false; // assume we fail unless we succeed.
    TRY
    // Start the clock.  Each thread reports its total runtime.
    watch.start("unzip");
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
        // This logging might be turned off since it is probably
        // unnecessary and unreliable anyway.
        if( !quiet ) LOG( thread_idx << "> " << name );
        // Decompress the data and write it to the file in
        // chunks of size equal to uncompressed.size().
        zip.extract_to( idx, name, uncompressed );
        // For auditing / sanity checking purposes.
        files++; bytes += size;
    }

    ret = true; // return success
    CATCH_ALL
    // Stop the clock; we will always get here even on exception.
    watch.stop("unzip");
}

/****************************************************************
 * Entrypoint into program.  This is called by a wrapper that
 * handles parsing commandline parameters which are then
 * delivered, already syntax checked, as data structures.
 ***************************************************************/
int main_( options::positional positional,
           options::options    options )
{
    // This will be used to register start/end times for each
    // of the tasks.
    StopWatch watch;
    watch.start( "total" ); // Even representing total runtime.

    /************************************************************
     * Get miscellaneous options
     ************************************************************/
    bool quiet = has_key( options, 'q' );

    /************************************************************
     * Determine the number of jobs to use
     ************************************************************/
    // First initialize the number of jobs to its default value.
    size_t jobs = 1;
    // Next, let's get the number of threads that the machine
    // naturally supports (this will include hyperthreads).  Even
    // if we don't need this, we might want to log it at the end.
    auto num_threads = thread::hardware_concurrency();
    // Now let's see if the user has specified the `-j` parameter
    // and get its value.  Note that, for this parameter, we are
    // guaranteed to have a value by the options parsing framework
    // (which knows that this parameter must have a value),
    // although it may not be valid.
    if( has_key( options, 'j' ) ) {
        string j = options['j'].get();
        // max  - use the max number of threads available.
        // auto - choose an "optimal" number of threads.
        // Otherwise, must be a positive integer.
        if( j == "max" )
            // Assume that num_threads includes the hyperthreads,
            // so in that case we probably don't want to go above
            // that.
            jobs = num_threads;
        else if( j == "auto" )
            // Assume we have hyper-threading; use all of the
            // "true" threads and then half of the hyperthreads.
            jobs = size_t( num_threads*0.75 + 0.5 );
        else
            // atoi should apparently return zero when the
            // conversion cannot be performed, which, luckily
            // for us, is an invalid value.
            jobs = atoi( j.c_str() );
    }
    // No matter how we got number of jobs, check it one last time.
    FAIL( jobs < 1, "invalid number of jobs: " << jobs );
    FAIL( jobs > MAX_JOBS, "max allowed jobs is " << MAX_JOBS );

    /************************************************************
     * Load the zip file and parse list of entries (but do not
     * yet start decompressing).
     ************************************************************/
    // The zip file name will be one of the positional arguments.
    string filename = positional[ARG_FILE_NAME];

    watch.start( "load_zip" );
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
    auto getter   = []( ZipStat const& zs ) { return zs.size(); };
    auto max_size = getter( maximum( z.begin(), z.end(), getter ) );

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
    watch.stop( "load_zip" );

    /************************************************************
     * Determine the chunk size
     ************************************************************/
    // When extracting a file from the zip archive, the chunk
    // size is the number of bytes that are decompressed and
    // written to the output file at a time.  The user can
    // specify "max" which will write the entire file at once,
    // however this requires being able to hold the entire
    // decompressed file in memory at once (and for every thread).
    // With zip files that contain large files together with
    // multithreaded execution it is desireable to limit the
    // chunk size to save memory.  Also, this allows you to
    // control size of the blocks that are written to disk which
    // may have an effect on disk write throughput.
    size_t chunk_size = DEFAULT_CHUNK;
    if( has_key( options, 'c' ) ) {
        string c = options['c'].get();
        if( c == "max" )
            // Use the max file size in the zip as chunk size.
            // Warning: this can cause allocation failures.
            chunk_size = max_size;
        else
            // atoi should apparently return zero when the
            // conversion cannot be performed, which, luckily
            // for us, is an invalid value.
            chunk_size = atoi( c.c_str() );
    }
    // At the very least, we need to check if this is zero which
    // it will be if it is invalid value on the command line.
    // However, it is not recommended to choose something as low
    // as one since that it likely an inefficient way to write to
    // disk.
    FAIL( chunk_size < 1, "Invalid chunk size" );

    /************************************************************
     * Pre-create folder structure
     ************************************************************/
    // In a parallel unzip we must pre-create all of the folders
    // that are mentioned in the zip file either explicitely or
    // implicitely.  If we attempt to do this within the worker
    // threads then we would likely have race conditions and
    // write conflicts.  So this way, when the worker threads
    // start, all of the necessary folders are already there.
    // First, we will gather all the folder names that are
    // mentioned both in the archive explicitly through folder
    // entries or implicitely as the paths to files.
    vector<FilePath> fps;
    for( auto const& zs : stats )
        fps.push_back( zs.folder() );
    // Now we ensure that each one exists.
    watch.run( "folders", [&fps]{ mkdirs_p( fps ); } );

    /************************************************************
     * Distribution of files to the threads
     ************************************************************/
    // See if the user has specified a distribution strategy.
    string strategy = has_key( options, 'd' ) ? options['d'].get()
                                              : DEFAULT_DIST;
    FAIL( !has_key( distribute, strategy ), "strategy " <<
        strategy << " is invalid." );
    // Do the distribution.  The result should be a vector of
    // length equal to the number of jobs.  Each element should
    // itself be a vector if indexs representing files assigned
    // to that thread for extraction.
    watch.start( "distribute" );
    auto thread_idxs = distribute[strategy]( jobs, files );
    watch.stop( "distribute" );
    FAIL_( thread_idxs.size() != jobs );

    /************************************************************
     * Start multithreaded unzip
     ************************************************************/
    // These will be populated by the threads as the work and
    // and then checked at the end as a sanity check.
    vector<size_t> files_count( jobs, 0 );
    vector<size_t> bytes_count( jobs, 0 );
    // vector<bool> is no good here since we need to extract
    // references to the elements.  This will hold the return
    // values from each thread.
    array<bool, MAX_JOBS> results;
    // Used for each thread to time their events.
    vector<StopWatch> watches( jobs );

    vector<thread> threads( jobs );

    watch.start( "unzip" );
    for( size_t i = 0; i < jobs; ++i )
        threads[i] = thread( unzip,
                             i,
                             std::ref( zip_buffer ),
                             std::ref( thread_idxs[i] ),
                             chunk_size,
                             quiet,
                             std::ref( watches[i] ),
                             std::ref( files_count[i] ),
                             std::ref( bytes_count[i] ),
                             std::ref( results[i] ) );

    for( auto& t : threads ) t.join();
    watch.stop( "unzip" );

    for( size_t i = 0; i < jobs; ++i )
        FAIL_( !results[i] );

    /************************************************************
     * Print out diagnostics
     ************************************************************/
    watch.stop( "total" ); // End program runtime.

    #define BYTES( a ) left << setw(11) << a <<        \
                       left << setw(11) <<             \
                       (" (" + human_bytes( a ) + ")")

    LOGP( "file",       filename       );
    LOGP( "jobs",       jobs           );
    LOGP( "threads",    num_threads    );
    LOGP( "entries",    z.size()       );
    LOGP( "files",      files.size()   );
    LOGP( "folders",    folders.size() );
    LOGP( "strategy",   strategy       );
    LOGP( "chunk",      chunk_size     );
    LOGP( "chunks_mem", BYTES( chunk_size*jobs ) );
    LOGP( "max size",   BYTES( max_size ) );

    LOG( "" );
    for( size_t i = 0; i < jobs; ++i )
        LOGP( "thread " << i+1 << " files",
            left << setw(22) << files_count[i] <<
            " [" << watches[i].human( "unzip" ) << "]" );

    // Make sure that the sum of files counts for each thread
    // equals the total number of files in the zip.
    auto files_written = accumulate(
        files_count.begin(), files_count.end() , size_t( 0 ) );
    LOGP( "total files", files_written );
    FAIL_( files_written != files.size() );

    LOG( "" );
    for( size_t i = 0; i < jobs; ++i )
        LOGP( "thread " << i+1 << " bytes",
            BYTES( bytes_count[i] ) <<
            " [" << watches[i].human( "unzip" ) << "]" );

    auto bytes = accumulate( bytes_count.begin(),
                             bytes_count.end(),
                             size_t( 0 ) );
    LOGP( "total bytes", BYTES( bytes ) );
    size_t total_in_zip = 0;
    for( size_t i = 0; i < z.size(); ++i )
        total_in_zip += z[i].size();
    FAIL_( total_in_zip != bytes );

    LOG( "" );
    // Log all the times that we measured but put "total" last.
    for( auto&& result : watch.results() )
        if( result.first != "total" )
            LOGP( result.first << " time", result.second );
    LOGP( "total time", watch.human( "total" ) );

    return 0;
}
