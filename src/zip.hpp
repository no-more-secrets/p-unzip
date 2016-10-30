/****************************************************************
 * OO Wrappers for libzip types
 ***************************************************************/
#pragma once

#include "ptr_resource.hpp"
#include "utils.hpp"

#include <zip.h>
#include <memory>

/****************************************************************
 * ZipSource
 ***************************************************************/
class ZipSource : public PtrRes<zip_source_t, ZipSource> {

    Buffer b;

public:
    using SP = std::shared_ptr<ZipSource>;

    ZipSource( Buffer&& buffer );

    void destroyer();
};

/****************************************************************
 * Zip
 ***************************************************************/
class Zip : public PtrRes<zip_t, Zip> {

    ZipSource::SP zs;

public:
    Zip( ZipSource::SP& zs );

    void destroyer();
};
