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

    size_t size() { return length; }
};

/****************************************************************
 * Resource manager for C FILE handles
 ***************************************************************/
class File : public PtrRes<FILE, File> {

public:
    File( std::string const& s, char const* mode );

    void destroyer();

    Buffer read();
};
