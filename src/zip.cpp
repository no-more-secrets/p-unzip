#include "zip.hpp"

#include <stdexcept>

using namespace std;

/****************************************************************
 * ZipSource
 ***************************************************************/
ZipSource::ZipSource( Buffer&& buf )
    : b( std::move( buf ) ) {
    zip_error_t error;
    if( !(p = zip_source_buffer_create(
        b.get(), b.size(), 0, &error ) ) ) {
        throw runtime_error(
            "failed to create zip source from buffer" );
    }
}

void ZipSource::destroyer() {
    zip_source_close( p );
}

/****************************************************************
 * Zip
 ***************************************************************/
Zip::Zip( ZipSource::SP& zs_ ) : zs( zs_ ) {
    zip_error_t error;
    if( !(p = zip_open_from_source( zs->get(), ZIP_RDONLY, &error )) )
        throw runtime_error( "failed to open zip from source" );
}

void Zip::destroyer() {
    zip_close( p );
}
