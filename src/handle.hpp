/****************************************************************
* Resource manager for raw pointers
****************************************************************/
#pragma once

#include "macros.hpp"

#include <stdexcept>
#include <typeinfo>

/****************************************************************
* This is a "moveable handle".  It will hold/manage a handle to
* a resource and ensure that the resource gets released.  It is
* implemented to disallow the holding of NULL pointer values,
* since if it holds such a value and you try to get() it, the
* class will throw.  This container can also hold weak references
* in the sense that it can be marked as non-owning by setting
* the appropriate flag.  Implementation uses the recursive
* template pattern (CRTP), so must be subclassed and templatized
* on the subclass.
****************************************************************/
template<typename PtrT, typename Child>
class Handle {

protected:
    PtrT* p;
    bool  own;

    void release() { own = false; }

    Handle() : p( NULL )
             , own( false ) {}

    Handle( Handle const& )             = delete;
    Handle& operator=( Handle const& )  = delete;
    Handle& operator=( Handle&& right ) = delete;

public:
    // Necessary for VS 2013 which does not provide auto-
    // generated default move constructors.
    Handle( Handle&& right ) : p( right.p )
                             , own( right.own ) {
        right.release();
    }

    // This object may contain a valid pointer to a live
    // resource, but will only release it if it has
    // ownership over it.  Furthermore, it will use the
    // CRTP to release it by calling the derived class's
    // destroyer() method.
    void destroy() {
        if( !own )
            return;
        static_cast<Child*>( this )->destroyer();
        release();
    }

    ~Handle() { destroy(); }

    // These objects may or may not hold a NULL value.  However,
    // we decide that if someone tries to get() the pointer
    // from this container and it is NULL then that is likely
    // an indication of a problem, so throw an exception.
    // There could be some use cases for getting a NULL pointer
    // out of these objects, but there are probably better
    // ways to achieve whatever is trying to be achieved there.
    PtrT* get() {
        FAIL( !p, "child: " << typeid(Child).name() );
        return p;
    }

    // const version of above
    PtrT const* get() const {
        FAIL( !p, "child: " << typeid(Child).name() );
        return p;
    }

};
