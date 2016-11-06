/****************************************************************
 * Macros
 ***************************************************************/
#include <sstream>
#include <iostream>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// ERR_IF is the converse of assert.  It will throw if the
// condition passed to it evaluates to true.
#define ERR_IF( a, b )                        \
    if( (a) ) {                               \
        std::ostringstream out;               \
        out << "assertion:" __FILE__ ":";     \
        out << TOSTRING(__LINE__) ": " << #a; \
        std::ostringstream out_msg;           \
        out_msg << b;                         \
        if( !out_msg.str().empty() )          \
            out << std::endl << b;            \
        throw std::logic_error( out.str() );  \
    }

#define ERR_IF_( a ) ERR_IF( a, "" )

// Braces to avoid variable name conflicts.
#define LOG( a )                        \
    {                                   \
        std::ostringstream out;         \
        out << a;                       \
        cerr << out.str() << std::endl; \
    }
