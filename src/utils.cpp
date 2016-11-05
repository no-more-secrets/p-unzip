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
File::File( string const& s, char const* mode_ )
    : mode( mode_ ) {
    if( mode != "rb" && mode != "wb" )
        throw runtime_error( "unrecognized mode " + mode );
    if( !(p = fopen( s.c_str(), mode_ ) ) )
        throw runtime_error( "failed to open file " + s
            + " with mode " + mode );
    own = true;
}

void File::destroyer() {
    cout << "closing file" << endl;
    fclose( p );
}

// Will read the entire contents of the file from the current
// File position and will leave the file position at EOF.
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

// Will write the entire contents of buffer to file starting
// from the file's current position.  Will throw if not all
// bytes written.
void File::write( Buffer const& buffer ) {
    if( mode != "wb" )
        throw runtime_error( "Can't write to file in mode " + mode );
    size_t written = fwrite( buffer.get(), 1, buffer.size(), p );
    if( written != buffer.size() )
        throw runtime_error( "Failed to write all bytes to file" );
}

/****************************************************************
 * Buffer
 ***************************************************************/
Buffer::Buffer( size_t length ) : length( length ) {
    if( !(p = (void*)( new uint8_t[length] )) )
        throw runtime_error( "failed to allocate buffer" );
    own = true;
}

void Buffer::destroyer() {
    cout << "releasing buffer" << endl;
    delete[] (uint8_t*)( p );
}
