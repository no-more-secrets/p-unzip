/****************************************************************
 * Macros
 ***************************************************************/
#pragma once

#include <sstream>
#include <iostream>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define STRING_JOIN(arg1, arg2) STRING_JOIN2(arg1, arg2)

// FAIL is the converse of assert.  It will throw if the
// condition passed to it evaluates to true.
#define FAIL( a, b )                        \
    if( (a) ) {                               \
        std::ostringstream out;               \
        out << "error:" __FILE__ ":";         \
        out << TOSTRING(__LINE__) ": " << #a; \
        std::ostringstream out_msg;           \
        out_msg << b;                         \
        if( !out_msg.str().empty() )          \
            out << std::endl << b;            \
        throw std::logic_error( out.str() );  \
    }

#define FAIL_( a ) FAIL( a, "" )

// Braces to avoid variable name conflicts.
#define LOG( a )                        \
    {                                   \
        std::ostringstream out;         \
        out << a;                       \
        cerr << out.str() << std::endl; \
    }

// Log a key/value pair
#define LOGP( a, b )                       \
    {                                      \
        std::ostringstream out_k,out_v;    \
        out_k << a; out_v << b;            \
        LOG( std::setw( 18 ) << std::left  \
             << out_k.str() << ": " <<     \
             out_v.str() );                \
    }

#define TRY try {

#define CATCH_ALL                        \
    } catch( std::exception const& e ) { \
        LOG( e.what() );                 \
    } catch( ... ) {                     \
        LOG( "unknown error" );          \
    }
