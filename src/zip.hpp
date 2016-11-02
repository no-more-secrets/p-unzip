/****************************************************************
 * OO Wrappers for libzip types
 ***************************************************************/
#pragma once

#include "ptr_resource.hpp"
#include "utils.hpp"

#include <zip.h>

/****************************************************************
 * Zip
 ***************************************************************/
class Zip : public PtrRes<zip_t, Zip> {

    Buffer::SP b;

public:
    Zip( Buffer::SP& zs );

    void destroyer();
};
