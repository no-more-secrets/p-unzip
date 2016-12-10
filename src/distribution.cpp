/****************************************************************
* Interfaces for taking a list of zip entries and distributing
* the files among a number of threads.
****************************************************************/
#include "macros.hpp"
#include "distribution.hpp"

#include <algorithm>
#include <set>
#include <vector>

using namespace std;

// Take a distribution function and wrap it with a wrapper
// function that will just call it and then perform some sanity
// checking.
#define WRAP( with )                                         \
    [] ( size_t _1, files_range const& _2 ) -> index_lists { \
        return wrapper( _1, _2, distribution_ ## with );     \
    }

// This macro will register a strategy (function) in the global
// map so it can be referred to by name.  The registration will
// happen automatically when the program loads.
#define STRATEGY( name ) \
    STARTUP() { distribute[TO_STRING( name )] = WRAP( name ); }

// Global dictionary is located and populated in this module.
map<string, distributor_t> distribute;

// This is a wrapper around each of the distribution functions
// that will perform some sanity checking post facto.  All the
// distribution functions get run by way of this wrapper.
index_lists wrapper( size_t             threads,
                     files_range const& files,
                     distributor_t      func ) {
    // Call the actual distribution function.
    vector<vector<size_t>> thread_idxs = func( threads, files );
    // Now a sanity check to make sure we got pricisely the
    // right number of files.
    size_t count = 0;
    for( auto const& ti : thread_idxs )
        count += ti.size();
    FAIL_( count != files.size() );
    // Another sanity check to make sure that each index appears
    // only once both within a single thread and across threads.
    set<size_t> idxs;
    for( auto const& ti : thread_idxs ) {
        for( size_t idx : ti ) {
            FAIL_( has_key( idxs, idx ) );
            idxs.insert( idx );
        }
    }
    return thread_idxs;
}

/****************************************************************
* The functions below will take a number of threads and a list
* of zip entries and will distribute them according to the
* given strategy.
****************************************************************/

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
    return thread_idxs;
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
    return thread_idxs;
}
STRATEGY( sliced ) // Register this strategy

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
    return thread_idxs;
}
STRATEGY( bytes ) // Register this strategy

//________________________________________________________________

// This function, which is a template for a strategy, will
// compile a list of all folders along with metrics computed on
// each folder, which are calculated using the metric function
// supplied as an argument.  It will then sort the list of
// folders by the magnitude of the their associated metrics.
// It will then iterate through the list of folders and assign
// each to a thread in a manner such as to try to keep total
// metric of the threads as uniform as possible.  The idea
// behind this strategy is to never assign files from the same
// folder to more than one thread, but while trying to give
// each thread roughly the same metric.
//
// In practice, the idea is that the metric (which is a number)
// should be proportional to the runtime necessary to extract
// the given zip entry.  This could be computed in various ways.
// The signature of MatricFunc is: size_t( ZipStat const& ).
template<typename MetricFunc>
index_lists by_folder( size_t             threads,
                       files_range const& files,
                       MetricFunc         metric ) {
    // This structure will hold information about a single
    // folder: this includes the metric and list of indexes
    // of files inside this folder.  Note, it is important
    // that the metric be proportional to runtime to the
    // greatest extent possible.
    struct Data {
        Data() : m_metric( 0 ) {}
        // Add a new file and update the metric.
        void add( ZipStat const& zs, size_t delta ) {
            m_idxs.push_back( zs.index() );
            m_metric += delta;
        }
        // These are indexes of files that are in this folder.
        vector<size_t> m_idxs;
        uint64_t       m_metric;
    };
    // First we need to aggregate files that are in the same
    // folder.
    map<FilePath, Data> folder_map;
    for( auto const& zs : files )
        folder_map[zs.folder()].add( zs, metric( zs ) );
    // Now copy all the Data objects into a vector for sorting.
    vector<Data> folder_infos;
    for( auto p : folder_map )
        folder_infos.push_back( p.second );
    // Sort that in descending order (it is important that they
    // are descending and not ascending) by the metric.
    auto by_metric = []( Data const& l, Data const& r ) {
        return l.m_metric > r.m_metric;
    };
    sort( folder_infos.begin(), folder_infos.end(), by_metric );
    // At this point we have a list of folders along with the
    // total metric of each folder, so now just do an equitable
    // distribution of folders among the threads.
    vector<vector<size_t>> thread_idxs( threads );
    vector<uint64_t>       metrics( threads );

    for( auto const& info : folder_infos ) {
        auto idx = min_element( metrics.begin(), metrics.end() )
                 - metrics.begin();
        FAIL_( idx < 0 || size_t( idx ) >= threads );
        auto& ti = thread_idxs[idx];
        // Give all the indexes from this folder to this thread.
        ti.insert( ti.end(), info.m_idxs.begin(), info.m_idxs.end() );
        metrics[idx] += info.m_metric;
    }
    // At this point the files in a given folder should be
    // assigned exclusively to a single thread and the
    // metric for each thread should be about the same.
    return thread_idxs;
}

//________________________________________________________________

// This is a "by_folder" strategy whose metric for a given zip
// entry is 1.  This means that we assume runtime is proportional
// to file creation time, since we are weighting all zip entries
// equally.
index_lists distribution_folder_files( size_t             threads,
                                       files_range const& files ) {
    return by_folder( threads, files, []( ZipStat const& ) {
        return 1;
    } );
}
STRATEGY( folder_files ) // Register this strategy

//________________________________________________________________

// This is a "by_folder" strategy whose metric for a given zip
// entry is the number of uncompressed bytes it will contain.
// This means that we assume runtime is proportional to the
// time needed to write the uncompressed file contents to disk.
index_lists distribution_folder_bytes( size_t             threads,
                                       files_range const& files ) {
    return by_folder( threads, files, []( ZipStat const& zs ) {
        return zs.size();
    } );
}
STRATEGY( folder_bytes ) // Register this strategy
