/****************************************************************
* Macros
****************************************************************/
#pragma once

#include <sstream>
#include <iomanip>
#include <iostream>

#define TO_STR1NG(x) #x
#define TO_STRING(x) TO_STR1NG(x)

#define STRING_JO1N(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) STRING_JO1N(arg1, arg2)

// FAIL is the converse of assert. It will throw if the condition
// passed to it evaluates to true.
#define FAIL( a, b ) if( a ) {                                 \
    std::ostringstream out;                                    \
    out << "error:" __FILE__ ":";                              \
    out << TO_STRING(__LINE__) ": " << #a;                     \
    std::ostringstream out_msg;                                \
    out_msg << b;                                              \
    if( !out_msg.str().empty() )                               \
        out << std::endl << b;                                 \
    throw std::logic_error( out.str() );                       \
}

#define FAIL_( a ) FAIL( a, "" )

#define TRY try {

#define CATCH_ALL                                              \
    } catch( std::exception const& e ) {                       \
        cerr << e.what();                                      \
    } catch( ... ) {                                           \
        cerr << "unknown error";                               \
    }

// This can be used  to  execute  an  arbitrary  block of code at
// startup (when the binary  is  loaded).  It  is used like this:
// STARTUP() { cout << "some code here"; }
#define STARTUP()                                              \
    struct STRING_JOIN( register_, __LINE__ ) {                \
        STRING_JOIN( register_, __LINE__ )();                  \
    }   STRING_JOIN( obj, __LINE__ );                          \
    STRING_JOIN( register_, __LINE__ )::                       \
        STRING_JOIN( register_, __LINE__ )()
