/****************************************************************
 * General utilities
 ***************************************************************/

#pragma once

#include "ptr_resource.hpp"

#include <string>

/****************************************************************
 * Logging
 ***************************************************************/
void log( std::string const& s );

/****************************************************************
 * Resource manager for C FILE handles
 ***************************************************************/
class File : public PointerResource<FILE, File> {

public:
    File( std::string const& s, char const* mode );

    void destroyer();
};
