/****************************************************************
* Main interface to the parallel unzip functionality.  This
* header contains the API function as well as definitions of
* data structures for communication information to and from that
* function.
****************************************************************/
#include "utils.hpp"

#include <functional>
#include <vector>

// This is the default distribution strategy to use if the
// user does not specify one.
#define DEFAULT_DIST "cyclic"
// If chunk size is not specified then this will be the default.
// Chunk size is the size of the chunk in which which data will
// be written to disk as it is decompressed.
#define DEFAULT_CHUNK 1024

// This enum represents the possible policies for dealing with
// timestamps when extracting zip files.
using TSXFormer = std::function<time_t ( time_t )>;

/****************************************************************
* This structure is used to return statistics and diagnostic info
* collected during the parallel unzip process which can aid in
* optimization and debugging.
****************************************************************/
struct UnzipSummary {

    UnzipSummary( size_t jobs );
    // VS 2013 can't generate default move constructors
    UnzipSummary( UnzipSummary&& from );

    // No harm in copying, but let's do this to prevent copying
    // which is not really necessary.
    UnzipSummary( UnzipSummary const& )            = delete;
    UnzipSummary& operator=( UnzipSummary const& ) = delete;

    // Total number of files in the zip archive
    size_t                 files;
    // Number of files extracted by each thread (ts = threads).
    std::vector<size_t>    files_ts;
    // Total bytes written (i.e., total uncompressed size).
    size_t                 bytes;
    // Number of bytes extracted by each thread (ts = threads).
    std::vector<size_t>    bytes_ts;
    // Total number of folders in the zip archive
    size_t                 folders;
    // Uncompressed size of largest file in archive.
    size_t                 max_size;
    // This holds timing info for the top-level process.
    StopWatch              watch;
    // These hold timeing info for the individual threads.
    std::vector<StopWatch> watches;

};

/****************************************************************
* Main interface to invoke a parallel unzip.
*****************************************************************
*   filename: path of zip file to be opened relative to CWD.
*
*      quiet: when true, the unzipper will suppress echoing to
*             stdout the names of each archived file as they are
*             extracted.
*
*       jobs: this many threads will be spawned
*
*   strategy: this is the name of the strategy to use to
*             distribute the archived files among the individual
*             threads.
*
* chunk_size: files will be decompressed and written to disk in
*             chunks of this size.  Note that an amount of heap
*             space will be allocated whose total size in bytes
*             is (jobs*chunk_size).  If zero is given here then
*             then chunk_size will be set equal to the size of
*             the largest file in the archive, which can lead to
*             allocation failures unless all the files are small
*             and/or there are few threads.
*
*   ts_xform: This is a callable from time_t -> time_t.  Each
*             time a file is decompressed and written to disk,
*             this function will be called with argument equal
*             to the timestamp of the archived file as it is
*             stored in the zip.  Whatever is returned from this
*             function will then be set as the timestamp of the
*             file. This gives the caller the opportunity to alter
*             the time stamp if desired.  Most of the time the
*             caller will want to pass the "identity" function,
*             effectively causing the timestamps archived in the
*             zip to be used (erasing time zone, as usual).
*
* This function will throw on any error, so if it returns, then
* hopefully that means that everything went according to plan. */
UnzipSummary p_unzip( std::string filename,
                      bool        quiet,
                      size_t      jobs,
                      std::string strategy,
                      size_t      chunk_size,
                      TSXFormer   ts_xform );

// This variant will call the above with sensible defaults.
void p_unzip_basic( std::string filename, size_t jobs = 1 );
