/****************************************************************
* p-unzip: the multithreaded unzipper
*
* author:  David P. Sicilia
*
* This program will take a zip file as input and a number of
* threads and it will distribute the files in the archive among
* the threads in the specified way in order to take advantage of
* the opportunity for parallelism while unzipping an archive.
****************************************************************/
//#include "config.hpp"
//#include "distribution.hpp"
//#include "fs.hpp"
#include "options.hpp"
//#include "utils.hpp"
#include "unzip.hpp"
//#include "zip.hpp"

//#include <algorithm>
//#include <array>
//#include <chrono>
#include <cstdlib>
//#include <iomanip>
//#include <mutex>
//#include <numeric>
//#include <string>
#include <thread>

using namespace std;

// Positional args should always be referred to by these.
size_t const ARG_FILE_NAME = 0;

/****************************************************************
* Entrypoint into program from the arg parsing framework.  This
* is called by a wrapper that handles parsing commandline
* parameters which are then delivered, already syntax checked, as
* data structures.
****************************************************************/
int main_( options::positional positional,
           options::options    options )
{
    /************************************************************
    * Get miscellaneous options
    ************************************************************/
    bool quiet = has_key( options, 'q' );

    /************************************************************
    * Determine timestamp (TS) policy
    *************************************************************
    * The `t` option allows the user to control how timestamps
    * are set on the extracted files.  If the timestamps are of
    * no concern then it might be advantageous to specify
    * "current" which will not set them at all -- it will leave
    * them to take on the time that they are created/written.
    * If timestamps are important then "stored" will try to
    * reproduce those stored in the zip file.  However, note that
    * zip files do not store timezone, so there will be certain
    * discrepancies associated with that (local timezone is
    * assumed when extracting).  If all the files are to have
    * the same timestamp then we can simply supply an integer
    * value for the t argument. */
    enum class TSPolicy {
        current,    // Use time when file is decompressed
        stored,     // Use time in zip file (no timezone)
        stored_alt, // Use stored time but alter it
        fixed
    };
    // By default we just use the "id" function that will use
    // the exact timestamp stored in the zip.  Note however that
    // this does not account for time zone.
    auto id_transform = []( time_t t ){ return t; };
    TSXFormer ts_transform( id_transform );
    if( has_key( options, 't' ) ) {
        string t = options['t'].get();
        if( t == "current" )
            // Just let the timestamps fall where they may.
            ts_transform = []( time_t ){ return 0; };
        else if( t == "stored" )
            // Use timestmaps stored inside zip file.
            ts_transform = id_transform;
        else {
            // All extracted files should have this timestamp.
            time_t fixedStamp = atoi( t.c_str() );
            FAIL( fixedStamp == 0, "invalid integer for t arg" );
            ts_transform = [=]( time_t ){ return fixedStamp; };
        }
    }

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

    /************************************************************
    * Get zip file name
    ************************************************************/
    // The zip file name will be one of the positional arguments.
    string filename = positional[ARG_FILE_NAME];

    /************************************************************
    * Determine the chunk size
    *************************************************************
    * When extracting a file from the zip archive, the chunk
    * size is the number of bytes that are decompressed and
    * written to the output file at a time.  The user can
    * specify "max" which will write the entire file at once,
    * however this requires being able to hold the entire
    * decompressed file in memory at once (and for every thread).
    * With zip files that contain large files together with
    * multithreaded execution it is desireable to limit the
    * chunk size to save memory.  Also, this allows you to
    * control size of the blocks that are written to disk which
    * may have an effect on disk write throughput. */
    size_t chunk_size = DEFAULT_CHUNK;
    if( has_key( options, 'c' ) ) {
        string c = options['c'].get();
        if( c == "max" )
            // Use the max file size in the zip as chunk size.
            // (the zero will be replaced with max file size
            // later when its value is known).  WARNING: this
            // can cause allocation failures.
            chunk_size = 0;
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
    * Distribution of files to the threads
    *************************************************************
    * See if the user has specified a distribution strategy. */
    string strategy = has_key( options, 'd' ) ? options['d'].get()
                                              : DEFAULT_DIST;

    /************************************************************
    * Unzip
    ************************************************************/
    UnzipSummary summary(
        p_unzip( filename, quiet, jobs, strategy, chunk_size,
                 ts_transform )
    );

    /************************************************************
    * Print out diagnostics
    ************************************************************/
    #define BYTES( a ) left << setw(11) << a <<        \
                       left << setw(11) <<             \
                       (" (" + human_bytes( a ) + ")")

    if( !chunk_size )
        chunk_size = summary.max_size;

    LOGP( "file",       filename       );
    LOGP( "jobs",       jobs << " / " << num_threads );
    LOGP( "files",      summary.files   );
    LOGP( "folders",    summary.folders );
    LOGP( "strategy",   strategy       );
    LOGP( "chunk",      chunk_size     );
    LOGP( "chunks_mem", BYTES( chunk_size*jobs ) );
    LOGP( "max size",   BYTES( summary.max_size ) );

    LOG( "" );
    for( size_t i = 0; i < jobs; ++i ) {
        LOGP( "files: thread " << i+1,
            left << setw(22) << summary.files_ts[i] <<
            " [" << summary.watches[i].human( "unzip" ) << "]" );
    }

    LOGP( "files: total", summary.files );

    LOG( "" );
    for( size_t i = 0; i < jobs; ++i ) {
        LOGP( "bytes: thread " << i+1,
            BYTES( summary.bytes_ts[i] ) <<
            " [" << summary.watches[i].human( "unzip" ) << "]" );
    }

    LOGP( "bytes: total", BYTES( summary.bytes ) );

    LOG( "" );
    // Log all the times that we measured but put "total" last.
    for( auto&& result : summary.watch.results() )
        if( result.first != "total" )
            LOGP( "time: " << result.first, result.second );
    LOGP( "time: total", summary.watch.human( "total" ) );

    return 0;
}
