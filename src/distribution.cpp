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
 * given strategy.  See header file for explanations.
 ***************************************************************/

index_lists distribution_cyclic( size_t             threads,
                                 files_range const& files ) {
    vector<vector<size_t>> thread_idxs( threads );
    size_t count = 0;
    for( auto& zs : files )
        thread_idxs[count++ % threads].push_back( zs.index() );
    return thread_idxs;
}
STRATEGY( cyclic ) // Register this strategy

//________________________________________________________________
//

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
    return thread_idxs;
}
STRATEGY( sliced ) // Register this strategy

//________________________________________________________________
//

index_lists distribution_folder( size_t             threads,
                                 files_range const& files ) {
    FAIL( true, "folder distribution not implemented" );
    (void)threads;
    (void)files;
    return {};
}
STRATEGY( folder ) // Register this strategy

//________________________________________________________________
//

index_lists distribution_bytes(  size_t             threads,
                                 files_range const& files ) {
    FAIL( true, "bytes distribution not implemented" );
    (void)threads;
    (void)files;
    return {};
}
STRATEGY( bytes ) // Register this strategy
