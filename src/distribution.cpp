/****************************************************************
 * Interfaces for taking a list of zip entries and distributing
 * the files among a number of threads.
 ***************************************************************/
#include "macros.hpp"
#include "distribution.hpp"

#include <algorithm>

using namespace std;

// This macro will create a dummy struct with a unique name that
// exists solely for the purpose of running some code at startup.
// Might want to refactor this into a generic "at startup".
#define STRATEGY( name )                                \
    struct STRING_JOIN( register_strategy, __LINE__ ) { \
        STRING_JOIN( register_strategy, __LINE__ )() {  \
            distribute[TOSTRING(name)] =                \
                distribution_ ## name;                  \
        }                                               \
    } STRING_JOIN( obj, __LINE__ );

// Global dictionary is located and populated in this module.
map<string, distribution_t> distribute;

/****************************************************************
 * The functions below will take a number of threads and a list
 * of zip entries and will distribute them according to the
 * given strategy.
 ***************************************************************/

// The "cyclic" strategy will first sort the files by path name,
// then will iterate through the sorted list while cycling
// through the list of threads, i.e., the first file will be
// assigned to the first thread, the second file to the second
// thread, the Nth file to the (N % threads) thread.
index_lists distribution_cyclic( size_t             threads,
                                 files_range const& files ) {
    vector<vector<size_t>> thread_idxs( threads );
    size_t count = 0;
    for( auto& zs : files )
        thread_idxs[count++ % threads].push_back( zs.index() );
    return move( thread_idxs );
}
STRATEGY( cyclic ) // Register this strategy

//________________________________________________________________

// The "sliced" strategy will first sort the files by path name,
// then will divide the resulting list into threads pieces and
// will assign each slice to the corresponding thread, i.e., if
// there are two threads, then the first half of the files will
// go to the first thread and the second half to the second.
index_lists distribution_sliced( size_t             threads,
                                 files_range const& files ) {
    // First we copy the zip stats and sort them by name.
    // Typically they will already be sorted, but just in
    // case they're not, we do it here.  This is important
    // because we want to minimize the number of folders
    // whose files are split among multiple threads.
    vector<ZipStat> stats( files.begin(), files.end() );
    auto by_name = []( ZipStat const& l, ZipStat const& r ) {
        return l.name() < r.name();
    };
    sort( stats.begin(), stats.end(), by_name );
    // Now calculate how many files we can give to each thread.
    // Each thread is given an equal number of files, minus
    // a few (< threads) that are residual at the end.  These
    // residual ones will be distributed as in the cyclic
    // strategy since there are so few of them that it doesn't
    // really matter how they're distributed.
    vector<vector<size_t>> thread_idxs( threads );
    size_t chunk = max( stats.size()/threads, size_t( 1 ) );
    size_t residual   = stats.size() % threads;
    size_t sliced_end = stats.size() - residual;
    FAIL_( chunk < 1 );
    FAIL_( chunk > stats.size() );
    FAIL_( residual > threads );
    FAIL_( sliced_end > stats.size() );
    // Now proceed to distribute to the threads.
    size_t count = 0;
    for( auto const& zs : stats ) {
        // This branch will be true most of the time.  It will
        // be false at the very end of the range when we hit
        // the residual items.
        size_t where = (count < sliced_end) ? count / chunk
                                            : count % threads;
        FAIL_( where >= threads );
        thread_idxs[where].push_back( zs.index() );
        ++count;
    }
    // Now a sanity check to make sure we got pricisely the
    // right number of files.
    count = 0;
    for( auto const& ti : thread_idxs )
        count += ti.size();
    FAIL_( count != files.size() );
    return move( thread_idxs );
}
STRATEGY( sliced ) // Register this strategy

//________________________________________________________________

// The "folder" strategy will compile a list of all folders
// along with the number of files they contain.  It will then
// sort the list of folders by the number of files they contain,
// and will then iterate through the list of folders and assign
// each to a thread in a cyclic manner.  The idea behind this
// strategy is to never assign files from the same folder to
// more than one thread, but while trying to give each thread
// roughly the same number of files.
index_lists distribution_folder( size_t             threads,
                                 files_range const& files ) {
    FAIL( true, "folder distribution not implemented" );
    (void)threads;
    (void)files;
    return {};
}
STRATEGY( folder ) // Register this strategy

//________________________________________________________________

// The "bytes" strategy will try to assign each thread roughly
// the same number of total bytes to write.  However, in
// practice the number bytes written by each thread will not
// be exactly the same because a given file must be assigned
// to a single thread in its entirety.
index_lists distribution_bytes(  size_t             threads,
                                 files_range const& files ) {
    // First we copy the zip stats and sort in descending
    // order by size.  This is because, if we distribute the
    // large files first, it is more likely in the end that
    // we will be able to balance them out using the small ones.
    vector<ZipStat> stats( files.begin(), files.end() );
    auto by_size = []( ZipStat const& l, ZipStat const& r ) {
        return l.size() > r.size();
    };
    sort( stats.begin(), stats.end(), by_size );
    vector<vector<size_t>> thread_idxs( threads );
    // These will hold the running sums of total (uncompressed)
    // bytes that each thread will have to extract.  We want
    // ideally (in this strategy at least) to balance them.
    vector<size_t> totals( threads, 0 );
    for( auto const& zs : stats ) {
        auto where = min_element( totals.begin(), totals.end() )
                   - totals.begin();
        FAIL_( where < 0 || size_t( where ) >= threads );
        thread_idxs[where].push_back( zs.index() );
        totals[where] += zs.size();
    }
    // Now a sanity check to make sure we got pricisely the
    // right number of files.
    size_t count = 0;
    for( auto const& ti : thread_idxs )
        count += ti.size();
    FAIL_( count != files.size() );
    return move( thread_idxs );
}
STRATEGY( bytes ) // Register this strategy

//________________________________________________________________

// The "runtime" strategy will try to estimate (up to a
// proportionality constant) the runtime of a thread by weighting
// the cost of file creation against the cost of writing the file
// contents.  More precisely, it will calculate a weighted sum
// of a threads file count and total bytes and hope that this is
// proportional to its runtime.  Using this metric, it will try to
// balance runtime among each of the threads.  The proportionality
// constants need to be calibrated and will be platform-dependent.
index_lists distribution_runtime(  size_t             threads,
                                   files_range const& files ) {
    uint64_t const size_weight = 1;
    uint64_t const file_weight = 5000000;
    // First we copy the zip stats and sort in descending
    // order by size.  This is because, if we distribute the
    // large files first, it is more likely in the end that
    // we will be able to balance them out using the small ones.
    vector<ZipStat> stats( files.begin(), files.end() );
    auto by_size = []( ZipStat const& l, ZipStat const& r ) {
        return l.size() > r.size();
    };
    sort( stats.begin(), stats.end(), by_size );
    vector<vector<size_t>> thread_idxs( threads );
    // These will hold the running sums of total estimated
    // runtime that each thread will take.  We want ideally
    // (in this strategy at least) to balance them.
    vector<uint64_t> totals( threads, 0 );
    for( auto const& zs : stats ) {
        auto where = min_element( totals.begin(), totals.end() )
                   - totals.begin();
        FAIL_( where < 0 || size_t( where ) >= threads );
        thread_idxs[where].push_back( zs.index() );
        totals[where] += size_weight * zs.size()
                      +  file_weight * 1;
    }
    // Now a sanity check to make sure we got pricisely the
    // right number of files.
    size_t count = 0;
    for( auto const& ti : thread_idxs )
        count += ti.size();
    FAIL_( count != files.size() );
    return move( thread_idxs );
}
STRATEGY( runtime ) // Register this strategy
