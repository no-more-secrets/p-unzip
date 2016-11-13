/****************************************************************
 * Interfaces for taking a list of zip entries and distributing
 * the files among a number of threads.
 ***************************************************************/
#include "macros.hpp"
#include "distribution.hpp"

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
    FAIL( true, "sliced distribution not implemented" );
    (void)threads;
    (void)files;
    return {};
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
