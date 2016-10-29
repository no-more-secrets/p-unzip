#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <zip.h>

using namespace std;

int main()
{
    cout << "Hello Unzip" << endl;

    char const* filename = "project.zip";
    FILE* fp;
    if( !(fp = fopen( filename, "rb" )) )
        throw runtime_error( "failed to open file" );
    cout << "Opened file " << filename << endl;

    if( fseek( fp, 0, SEEK_END ) != 0 )
        throw runtime_error( "fialed to seek to end of file" );
    auto length = ftell( fp );
    cout << "Length: " << length << endl;
    rewind( fp );

    void* buffer;
    if( !(buffer = malloc( length )) )
        throw runtime_error( "failed to allocate initial buffer" );

    auto length_read = fread( buffer, 1, length, fp );
    if( length != static_cast<decltype( length )>( length_read ) )
        throw runtime_error( "failed to read zip file" );
    cout << "Read " << length_read << " bytes" << endl;

    /////////////////////////////////////////////////////////////

    zip_error_t error;
    zip_source_t* zs;
    if( !(zs = zip_source_buffer_create( buffer, length, 0, &error ) ) )
        throw runtime_error( "failed to create zip source from buffer" );

    zip_t* z;
    if( !(z = zip_open_from_source( zs, ZIP_RDONLY, &error )) )
        throw runtime_error( "failed to open zip from source" );

    auto count = zip_get_num_entries( z, ZIP_FL_UNCHANGED );
    cout << "Found " << count << " entries:" << endl;

    for( auto i = 0; i < count; ++i ) {

        zip_stat_t s;
        if( zip_stat_index( z, i, ZIP_FL_UNCHANGED, &s ) )
            throw runtime_error( "failed to stat file" );

        if( s.valid & ZIP_STAT_INDEX     ) cout << "  index: " << s.index     << endl;
        if( s.valid & ZIP_STAT_NAME      ) cout << "    name:      " << s.name      << endl;
        if( s.valid & ZIP_STAT_SIZE      ) cout << "    size:      " << s.size      << endl;
        if( s.valid & ZIP_STAT_COMP_SIZE ) cout << "    comp_size: " << s.comp_size << endl;
        if( s.valid & ZIP_STAT_MTIME     ) cout << "    mtime:     " << s.mtime     << endl;
        if( s.valid & ZIP_STAT_FLAGS     ) cout << "    flags:     " << s.flags     << endl;
    }

    zip_close( z );

    zip_source_close( zs );

    /////////////////////////////////////////////////////////////

    free( buffer );

    fclose( fp );
}
