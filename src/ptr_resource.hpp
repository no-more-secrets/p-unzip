#pragma once

#include "macros.hpp"

#include <stdexcept>
#include <typeinfo>

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
    PtrRes& operator=( PtrRes&& right ) = delete;

public:

    PtrRes( PtrRes&& right )
        : p( right.p )
        , own( right.own ) {
        right.release();
    }

    /*PtrRes& operator=( PtrRes&& right ) {
        if( this == &right )
            return *this;
        destroy();
        p = right.p;
        own = right.own;
        right.release();
    }*/

    void destroy() {
        if( own ) {
            static_cast<Child*>( this )->destroyer();
            release();
        }
    }

    ~PtrRes() { destroy(); }

    PtrT* get() {
        FAIL( !p, "child: " << typeid(Child).name() );
        return p;
    }

    PtrT const* get() const {
        FAIL( !p, "child: " << typeid(Child).name() );
        return p;
    }

};
