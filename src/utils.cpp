/****************************************************************
* General utilities
****************************************************************/
#include "macros.hpp"
#include "utils.hpp"

#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>

using namespace std;

/****************************************************************
* Utilities
****************************************************************/

// Compute a primitive but "good enough" hash of a string.
uint32_t string_hash( string const& s ) {
    // Initialize some variables with some primes.
    uint32_t const A = 54059, B = 76963;
    uint32_t hash = 37;
    for( auto c : s )
        hash = (hash * A)^( uint32_t( c ) * B );
    return hash;
}

/****************************************************************
* Convenience methods
****************************************************************/
// Format a quantity of bytes in human readable form.
std::string human_bytes( uint64_t bytes ) {
    ostringstream out;
    size_t GB = 1024*1024*1024;
    size_t MB = 1024*1024;
    size_t KB = 1024;
    if( bytes >= GB )
        out << fixed << setprecision(1) << double(bytes)/GB << "GB";
    else if( bytes >= MB )
        out << fixed << setprecision(1) << double(bytes)/MB << "MB";
    else if( bytes >= KB )
        out << fixed << setprecision(1) << double(bytes)/KB << "KB";
    else
        out << bytes << "B";
    return out.str();
}

/****************************************************************
* StopWatch
****************************************************************/
// Start the clock for a given event name.  If an event with
// this name already exists then it will be overwritten and
// any end times for it will be deleted.
void StopWatch::start( string const& name ) {
    start_times[name] = chrono::system_clock::now();
    if( has_key( end_times, name ) )
        end_times.erase( name );
}

// Register an end time for an event.  Will throw if there
// was no start time for the event.
void StopWatch::stop( string const& name ) {
    FAIL_( !has_key( start_times, name ) );
    end_times[name] = chrono::system_clock::now();
}

// Get results for an even in the given units.  If either a
// start or end time for the event has not been registered
// then these will throw.
int64_t StopWatch::milliseconds( string const& name ) const {
    FAIL_( !event_complete( name ) );
    return chrono::duration_cast<chrono::milliseconds>(
        end_times.at( name ) - start_times.at( name ) ).count();
}
int64_t StopWatch::seconds( string const& name ) const {
    FAIL_( !event_complete( name ) );
    return chrono::duration_cast<chrono::seconds>(
        end_times.at( name ) - start_times.at( name ) ).count();
}
int64_t StopWatch::minutes( string const& name ) const {
    FAIL_( !event_complete( name ) );
    return chrono::duration_cast<chrono::minutes>(
        end_times.at( name ) - start_times.at( name ) ).count();
}

// Gets the results for an event and then formats them in
// a way that is most readable given the duration.
string StopWatch::human( string const& name ) const {
    FAIL_( !event_complete( name ) );
    ostringstream out;
    // Each of these represent the same time, just in
    // different units.
    auto m  = minutes( name );
    auto s  = seconds( name );
    auto ms = milliseconds( name );
    if( m > 0 )
        out << m << "m" << s % 60 << "s";
    else if( s > 0 ) {
        out << s;
        if( s < 10 )
            out << "." << ms % 1000;
        out << "s";
    }
    else
        out << ms << "ms";
    return out.str();
}

// Get a list of all results in human readable form.
vector<StopWatch::result_pair> StopWatch::results() const {
    vector<result_pair> res;
    for( auto p : start_times ) {
        FAIL( !event_complete( p.first.c_str() ),
            "event " << p.first << " is not complete." );
        res.push_back( make_pair(
            p.first, human( p.first ) ) );
    }
    return res;
}

// Will simply check if an event is present in both the
// start and end time sets, i.e., it is ready for computing
// results.
bool StopWatch::event_complete( string const& name ) const {
    return has_key( start_times, name ) &&
           has_key( end_times,   name );
}

/****************************************************************
* Buffer
****************************************************************/
Buffer::Buffer( size_t length ) : length( length ) {
    FAIL_( !(p = (void*)( new uint8_t[length] )) );
    own = true;
}

void Buffer::destroyer() {
    delete[] (uint8_t*)( p );
}
