#pragma once

#include <stdexcept>

/****************************************************************
 * Resource manager for raw pointers
 ***************************************************************/
template<typename PtrT, typename Child>
class PtrRes {

protected:

    PtrT* p;

    void release() { p = NULL; }

    PtrRes() : p( NULL ) {}

    PtrRes( PtrRes const& )            = delete;
    PtrRes& operator=( PtrRes const& ) = delete;
    PtrRes& operator=( PtrRes&& )      = delete;

public:

    PtrRes( PtrRes&& right )
        : p( std::move( right.p ) ) {
        right.release();
    }

    void destroy() {
        if( p ) {
            static_cast<Child*>( this )->destroyer();
            release();
        }
    }

    ~PtrRes() { destroy(); }

    PtrT* get() {
        if( !p )
            throw std::runtime_error(
                "attempted to get() reference NULL pointer" );
        return p;
    }

};
