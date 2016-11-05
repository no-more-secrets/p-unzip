#include "zip.hpp"

#include <stdexcept>

using namespace std;

/****************************************************************
 * Zip
 ***************************************************************/
Zip::Zip( Buffer::SP& b_ ) : b( b_ ) {
    // First creat a "zip source" from the raw pointer to the
    // buffer containing the binary data of the zip file.  The
    // zip source will not take ownership of the buffer.
    zip_error_t   error;
    zip_source_t* zs;
    if( !(zs = zip_source_buffer_create(
        b->get(), b->size(), 0, &error ) ) ) {
        throw runtime_error(
            "failed to create zip source from buffer" );
    }
    // Now create a zip_t object which attaches to (and owns)
    // the zip source.
    if( !(p = zip_open_from_source( zs, ZIP_RDONLY, &error )) )
        throw runtime_error( "failed to open zip from source" );
    own = true;
    // Lastly, count number of files in the archive and get
    // each of their stats and cache them.
    size_t size = zip_get_num_entries( p, ZIP_FL_UNCHANGED );
    for( size_t i = 0; i < size; ++i ) {
        zip_stat_t stat;
        if( zip_stat_index( p, i, ZIP_FL_UNCHANGED, &stat ) )
            throw runtime_error( "failed to stat item" );
        stats.emplace_back( stat );
    }
}

// Create a new buffer of the size necessary to hold the
// uncompressed contents, then do the uncompression and
// return the buffer.
Buffer Zip::extract( size_t idx ) const {
    Buffer out( at( idx ).size() );
    extract_in( idx, out );
    return out;
}

// Uncompress file into existing buffer.  Throws if the
// buffer is not big enough.
void Zip::extract_in( size_t idx, Buffer& buffer ) const {
    (void)idx;
    (void)buffer;
}

// This will release the underlying zip source, but not the
// buffer from which the zip source was created.  However, when
// all Zip objects that refer to a certain Buffer go out of
// scope then the buffer will be freed because we are holding
// the buffer in a shared pointer.
void Zip::destroyer() {
    zip_close( p );
}

// Access a given element of the archive.
ZipStat const& Zip::at(size_t idx) const {
    if( idx >= stats.size() )
        throw runtime_error( "idx out of bounds" );
    return stats[idx];
}

/****************************************************************
 * ZipStat
 ***************************************************************/
// This is the zero-based index within the archive of the
// element represented by this ZipStat.
zip_uint64_t ZipStat::index() const {
    if( stat.valid & ZIP_STAT_INDEX )
        return stat.index;
    throw runtime_error( "index not available" );
}

// File/folder name of entry.  Folder names end with /
string ZipStat::name() const {
    if( stat.valid & ZIP_STAT_NAME )
        return string( stat.name );
    throw runtime_error( "name not available" );
}

// Uncompressed size of entry.
zip_uint64_t ZipStat::size() const {
    if( stat.valid & ZIP_STAT_SIZE )
        return stat.size;
    throw runtime_error( "size not available" );
}

// Compressed size of entry.
zip_uint64_t ZipStat::comp_size() const {
    if( stat.valid & ZIP_STAT_COMP_SIZE )
        return stat.comp_size;
    throw runtime_error( "comp_size not available" );
}

// Last mode time.  This will be rounded to the nearest
// two-second boundary and contains no timezone.  Also,
// zip files do not store timezone.  So the time returned
// by this function must be interpreted based on the
// known timezone of the machine that created the zip.
time_t ZipStat::mtime() const {
    if( stat.valid & ZIP_STAT_MTIME )
        return stat.mtime;
        //return chrono::system_clock::from_time_t( stat.mtime );
    throw runtime_error( "mtime not available" );
}
