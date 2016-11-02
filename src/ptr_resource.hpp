#pragma once

#include <stdexcept>

/****************************************************************
 * Resource manager for raw pointers
 ***************************************************************/
template<typename PtrT, typename Child>
class PtrRes {

protected:

    PtrT* p;
    bool  own;

    void release() { own = false; }

    PtrRes() : p( NULL ), own( false ) {}

    PtrRes( PtrRes const& )            = delete;
    PtrRes& operator=( PtrRes const& ) = delete;
    PtrRes& operator=( PtrRes&& )      = delete;

public:

    PtrRes( PtrRes&& right )
        : p( std::move( right.p ) )
        , own( right.own ) {
        right.release();
    }

    void destroy() {
        if( own ) {
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
