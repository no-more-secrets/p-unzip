/****************************************************************
 * General utilities
 ***************************************************************/

#pragma once

#include "ptr_resource.hpp"

#include <string>
#include <memory>

/****************************************************************
 * Logging
 ***************************************************************/
void log( std::string const& s );

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

    // Will write the entire contents of buffer to file starting
    // from the file's current position.  Will throw if not all
    // bytes written.
    void write( Buffer const& buffer );
};
