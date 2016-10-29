/****************************************************************
 * General utilities
 ***************************************************************/
#include "utils.hpp"

#include <cstdio>
#include <stdexcept>
#include <iostream>

using namespace std;

/****************************************************************
 * Logging
 ***************************************************************/
void log( string const& s ) {
    cout << s << endl;
}

/****************************************************************
 * File
 ***************************************************************/
File::File( string const& s, char const* mode ) {
    if( !(p = fopen( s.c_str(), mode ) ) )
        throw runtime_error( "failed to open file " + s
            + " with mode " + mode );
}

void File::destroyer() {
    fclose( p );
}
