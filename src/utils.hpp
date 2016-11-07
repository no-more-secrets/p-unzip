/****************************************************************
 * General utilities
 ***************************************************************/

#pragma once

#include "macros.hpp"
#include "ptr_resource.hpp"

#include <string>
#include <memory>

/****************************************************************
 * Convenience methods
 ***************************************************************/
// Does the set contain the given key.
template<typename ContainerT, typename KeyT>
inline bool has_key( ContainerT const& s, KeyT const& k ) {
    return s.find( k ) != s.end();
}

// Does the string start with the character?
inline bool starts_with( std::string const& s, char c ) {
    return s.size() > 0 && s[0] == c;
}

// Does the string end with the character?
inline bool ends_with( std::string const& s, char c ) {
    return s.size() > 0 && s[s.size()-1] == c;
}

// This functino will find the maximum over an iterable given a
// key function.  The key function will be applied to each
// element of the iterable to yield a key, then the keys will be
// compared with the < operator to find the maximum.  The return
// value will be value of the iterable whose key was found to
// be maximum (but not the value of that key itself).
template<typename It, typename KeyF>
auto maximum( It start, It end, KeyF f ) -> decltype( *start ) {
    using elem_type = decltype( *start );
    auto cmp = [&f]( elem_type const& l, elem_type const& r ) {
        return f( l ) < f( r );
    };
    auto max_iter = max_element( start, end, cmp );
    return *max_iter;
}

/****************************************************************
 * Range class for turning pairs of iterators into iterables.
 * The future ranges library will probably do this better.  At
 * the moment this will only work for random access iterators
 * because of the size().
 ***************************************************************/
template<typename T>
class Range {

public:
    explicit Range( T begin, T end ) : begin_( begin )
                                     , end_( end ) {}

    // Should return these iterators by value so that the
    // caller doesn't change them.
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
 ***************************************************************/
class Buffer : public PtrRes<void, Buffer> {

    size_t length;

public:
    typedef std::shared_ptr<Buffer> SP;

    Buffer( size_t length );

    void destroyer();

    size_t size() const { return length; }

};

/****************************************************************
 * Resource manager for C FILE handles
 ***************************************************************/
class File : public PtrRes<FILE, File> {

    std::string mode;

public:
    File( std::string const& s, char const* mode );

    void destroyer();

    // Will read the entire contents of the file from the current
    // File position and will leave the file position at EOF.
    Buffer read();

    // Will write `count` bytes of buffer to file starting
    // from the file's current position.  Will throw if not all
    // bytes written.
    void write( Buffer const& buffer, size_t count );
};

/****************************************************************
 * Optional: Struct for holding a value that either is there or
 * isn't.  This could be replaced with std::optional when we
 * have C++17 compilers available.
 ***************************************************************/
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

};
