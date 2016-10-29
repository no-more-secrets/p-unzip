/****************************************************************
 * OO Wrappers for libzip types
 ***************************************************************/
#pragma once

#include "ptr_resource.hpp"

#include <zip.h>

/****************************************************************
 * ZipSource
 ***************************************************************/
class ZipSource : public PointerResource<zip_source_t, ZipSource> {

public:
    ZipSource( void* buffer, size_t length );

    void destroyer();
};

/****************************************************************
 * Zip
 ***************************************************************/
class Zip : public PointerResource<zip_t, Zip> {

public:
    Zip( ZipSource& zs );

    void destroyer();
};
