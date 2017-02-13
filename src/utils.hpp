/****************************************************************
* General utilities
****************************************************************/
#pragma once

#include "macros.hpp"
#include "handle.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

/****************************************************************
* Convenience methods
****************************************************************/
// Format a quantity of bytes in human readable form.
std::string human_bytes( uint64_t bytes );

// Does the set contain the given key.
template<typename ContainerT, typename KeyT>
bool has_key( ContainerT const& s, KeyT const& k ) {
    return s.find( k ) != s.end();
}

// Get value for key from map;  if  map  does not contain the key
// then simply return  the  default  value  specified WITHOUT in-
// serting it into the map.
template<typename KeyT, typename ValT>
ValT const& map_get( std::map<KeyT, ValT> const& m,
                     KeyT const& k,
                     ValT const& def ) {
    auto found = m.find( k );
    if( found == m.end() )
        return def;
    return found->second;
}

// Does the string start with the character?
inline bool starts_with( std::string const& s, char c ) {
    return s.size() > 0 && s[0] == c;
}

// Does the string end with the character?
inline bool ends_with( std::string const& s, char c ) {
    return s.size() > 0 && s[s.size()-1] == c;
}

// Return a new string with all chars lowercase.
std::string to_lower( std::string const& s );

template<typename T>
std::string to_string( T const& x ) {
    std::ostringstream ss;
    ss << x;
    return ss.str();
}

// Computes a primitive but "good enough"  hash of a string. This
// is not even close to cryptographically  secure, but it is fine
// for this program. That said,  tests  have  been done to verify
// that, over the domain of  inputs  typical of this program, the
// hashing algorithm produces  almost  perfectly uniform results.
uint32_t string_hash( std::string const& s );

// Convert s to a positive integer  and throw if conversion fails
// or if conversion succeeds but number is < 0.
template<typename T>
T to_uint( std::string const& s ) {
    std::istringstream ss( s );
    long res; ss >> res;
    FAIL( !ss, "failed to convert \"" << s << "\" to number" );
    FAIL( res < 0, "number " << res << " must not be negative." );
    return static_cast<T>( res );
}

// "Identity" function (returns argument by value)
template<typename T>
auto id( T t ) -> T { return t; }

// This function will find the  maximum  over an iterable given a
// key function. The key function will be applied to each element
// of the iterable to yield a key, then the keys will be compared
// with the < operator to find the maximum. The return value will
// be value of the iterable  whose  key  was  found to be maximum
// (but not the value of that key itself).
template<typename It, typename KeyF>
auto maximum( It start, It end, KeyF f ) -> decltype( *start ) {
    using elem_type = decltype( *start );
    auto cmp = [&f]( elem_type const& l, elem_type const& r ) {
        return f( l ) < f( r );
    };
    auto max_iter = max_element( start, end, cmp );
    // If for some reason we do  not find a maximum element (such
    // as in the case that the input  list is empty) then we must
    // fail because the function signature  requires us to return
    // a value of the iterable which,  in that case, we would not
    // have. I think this should only  happen when the input list
    // is empty.
    FAIL( max_iter == end, "cannot call maximum on empty list" );
    return *max_iter;
}

/****************************************************************
* StopWatch
****************************************************************/
/* This class can be used to mark start/stop times of various
 * events and to get the  durations  in  various useful forms. */
class StopWatch {

public:
    // For convenience: will start, run, stop.
    template<typename FuncT>
    void run( std::string const& name, FuncT func ) {
        start( name ); func(); stop( name );
    }

    // Start the clock for a given  event  name. If an event with
    // this name already exists then  it  will be overwritten and
    // any end times for it will be deleted.
    void start( std::string const& name );
    // Register an end time for an event. Will throw if there was
    // no start time for the event.
    void stop( std::string const& name );

    // Get results for an even  in  the  given units. If either a
    // start or end time for  the  event  has not been registered
    // then these will throw.
    int64_t milliseconds( std::string const& name )const ;
    int64_t seconds( std::string const& name ) const;
    int64_t minutes( std::string const& name ) const;

    // Gets the results for an event  and  then formats them in a
    // way that is most readable given the duration.
    std::string human( std::string const& name ) const;
    // Get a list of all  results  in  human readable form. First
    // element of pair is the  event  name  and the second is the
    // result of calling human() for that event.
    using result_pair = std::pair<std::string, std::string>;
    std::vector<result_pair> results() const;

private:
    using clock        = std::chrono::system_clock;
    using time_point   = std::chrono::time_point<clock>;
    using events_timer = std::map<std::string, time_point>;

    bool event_complete( std::string const& name ) const;

    events_timer start_times;
    events_timer end_times;

};

/****************************************************************
* Range class for turning pairs  of iterators into iterables. The
* future ranges library will probably do  this better. At the mo-
* ment this will only work for random access iterators because of
* the size().
****************************************************************/
template<typename T>
class Range {

public:
    explicit Range( T begin, T end ) : begin_( begin )
                                     , end_( end ) {}

    // Should return these iterators by  value so that the caller
    // doesn't change them.
    T begin() const { return begin_; }
    T end()   const { return end_;   }

    // Will always return a positive size.
    size_t size() const {
        auto s = end_ - begin_;
        return (s >= 0) ? s : -s;
    }

private:
    T begin_;
    T end_;

};

template<typename T>
Range<T> make_range( T begin, T end ) {
    return Range<T>( begin, end );
}

/****************************************************************
* Resource manager for raw buffers
****************************************************************/
class Buffer : public Handle<void, Buffer> {

    size_t length;

public:
    typedef std::shared_ptr<Buffer> SP;

    Buffer( size_t length );

    // This is for VS 2013  which  does not supply implicite move
    // constructors (which, if  it  did,  should  be identical to
    // this one below).
    Buffer( Buffer&& from )
        : Handle<void, Buffer>( std::move( from ) )
        , length( from.length )
    {}

    void destroyer();

    size_t size() const { return length; }

};

/****************************************************************
* Optional: Struct for holding a  value  that  either is there or
* isn't. This could be replaced  with  std::optional when we have
* C++17 compilers available.
****************************************************************/
template<typename T>
struct Optional {

    bool has_value;
    T value;

public:
    Optional() : has_value( false ) {}

    // Perfect forwarding
    template<typename V>
    explicit Optional( V&& s )
        : has_value( true )
        , value( std::forward<V>( s ) )
    {}

    T const& get() const {
        FAIL( !has_value, "Optional has no value." );
        return value;
    }

    // `explicit` to prevent  accidental  conversions which might
    // allow code to compile that should not compile.
    explicit operator bool() const { return has_value; }

};
