#pragma once

#include <stdexcept>

/****************************************************************
 * Resource manager for raw pointers
 ***************************************************************/
template<typename PtrT, typename Child>
class PointerResource {

protected:

    PtrT* p;

    void release() { p = NULL; }

    PointerResource() : p( NULL ) {}

    PointerResource( PointerResource const& )            = delete;
    PointerResource& operator=( PointerResource const& ) = delete;
    PointerResource( PointerResource&& right )           = delete;
    /*{
        p = right.p;
        right.release();
    }*/

public:

    void destroy() {
        if( p ) {
            static_cast<Child*>( this )->destroyer();
            release();
        }
    }

    ~PointerResource() {
        destroy();
    }

    PtrT* get() {
        if( !p )
            throw std::runtime_error( "attempted to get() reference NULL pointer" );
        return p;
    }

};
