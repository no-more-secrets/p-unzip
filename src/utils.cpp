/****************************************************************
 * General utilities
 ***************************************************************/
#include "macros.hpp"
#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <cstdint>

using namespace std;

/****************************************************************
 * File
 ***************************************************************/
File::File( string const& s, char const* mode_ )
    : mode( mode_ ) {
    ERR_IF( mode != "rb" && mode != "wb",
        "unrecognized mode " << mode );
    ERR_IF( !(p = fopen( s.c_str(), mode_ )),
        "failed to open file " << s << " with mode " << mode );
    own = true;
}

void File::destroyer() {
    LOG( "closing file" );
    fclose( p );
}

// Will read the entire contents of the file from the current
// File position and will leave the file position at EOF.
Buffer File::read() {
    ERR_IF_( fseek( p, 0, SEEK_END ) != 0 );
    size_t length = ftell( p );
    rewind( p );

    Buffer buffer( length );

    auto length_read = fread( buffer.get(), 1, length, p );
    ERR_IF_( length != length_read );

    return buffer;
}

// Will write the entire contents of buffer to file starting
// from the file's current position.  Will throw if not all
// bytes written.
void File::write( Buffer const& buffer ) {
    ERR_IF( mode != "wb", "attempted write in mode " << mode );
    size_t written = fwrite( buffer.get(), 1, buffer.size(), p );
    ERR_IF_( written != buffer.size() );
}

/****************************************************************
 * Buffer
 ***************************************************************/
Buffer::Buffer( size_t length ) : length( length ) {
    ERR_IF_( !(p = (void*)( new uint8_t[length] )) );
    own = true;
}

void Buffer::destroyer() {
    LOG( "releasing buffer" );
    delete[] (uint8_t*)( p );
}
