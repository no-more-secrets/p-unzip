/****************************************************************
 * General utilities
 ***************************************************************/
#include "utils.hpp"

#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <cstdint>

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

Buffer File::read() {
    if( fseek( p, 0, SEEK_END ) != 0 )
        throw runtime_error( "failed to seek to end of file" );
    size_t length = ftell( p );
    rewind( p );

    Buffer buffer( length );

    auto length_read = fread( buffer.get(), 1, length, p );
    if( length != length_read )
        throw runtime_error( "failed to read file" );

    return buffer;
}

/****************************************************************
 * Buffer
 ***************************************************************/
Buffer::Buffer( size_t length ) : length( length ) {
    if( !(p = (void*)( new uint8_t[length] )) )
        throw runtime_error( "failed to allocate buffer" );
}

void Buffer::destroyer() {
    delete[] (uint8_t*)( p );
}
