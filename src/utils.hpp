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

    T const& get() {
        FAIL( !has_value, "Optional has no value." );
        return value;
    }

};
