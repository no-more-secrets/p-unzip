#include "zip.hpp"

#include <stdexcept>

using namespace std;

/****************************************************************
 * Zip
 ***************************************************************/
Zip::Zip( Buffer::SP& b_ ) : b( b_ ) {
    zip_error_t   error;
    zip_source_t* zs;
    if( !(zs = zip_source_buffer_create(
        b->get(), b->size(), 0, &error ) ) ) {
        throw runtime_error(
            "failed to create zip source from buffer" );
    }
    if( !(p = zip_open_from_source( zs, ZIP_RDONLY, &error )) )
        throw runtime_error( "failed to open zip from source" );
    own = true;
    // Evidentally zip_t takes ownership of the zip_source_t
    // and will try to free it, so we don't have to release it.
}

void Zip::destroyer() {
    zip_close( p );
}
