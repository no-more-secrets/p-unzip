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
    // TODO: need to call error APIs to extract error msg
    zip_error_t   error;
    zip_source_t* zs;
    zs = zip_source_buffer_create( b->get(), b->size(), 0, &error );
    ERR_IF( !zs, "failed to create zip source from buffer" );
    // Now create a zip_t object which attaches to (and owns)
    // the zip source.
    p = zip_open_from_source( zs, ZIP_RDONLY, &error );
    ERR_IF( !p, "failed to open zip from source" );
    own = true;
    // Lastly, count number of files in the archive and get
    // each of their stats and cache them.
    size_t size = zip_get_num_entries( p, ZIP_FL_UNCHANGED );
    for( size_t i = 0; i < size; ++i ) {
        zip_stat_t stat;
        ERR_IF( zip_stat_index( p, i, ZIP_FL_UNCHANGED, &stat ),
           "failed to stat item" << i );
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
    size_t fsize = at( idx ).size();
    ERR_IF_( fsize > buffer.size() );
    zip_file_t* zf;
    ERR_IF_( !(zf = zip_fopen_index( p, idx, 0 )) );
    // !! Should not throw until zip_fclose is called
    zip_uint64_t count = zip_fread( zf, buffer.get(), fsize );
    // !! Close immediately to avoid resource leak.
    zip_fclose( zf );
    // If we haven't read a number of bytes equal to the
    // reported size of the uncompressed file then throw.
    ERR_IF_( count != fsize );
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
    ERR_IF_( idx >= stats.size() );
    return stats[idx];
}

/****************************************************************
 * ZipStat
 ***************************************************************/
// This is the zero-based index within the archive of the
// element represented by this ZipStat.
zip_uint64_t ZipStat::index() const {
    ERR_IF_( !(stat.valid & ZIP_STAT_INDEX) );
    return stat.index;
}

// File/folder name of entry.  Folder names end with /
string ZipStat::name() const {
    ERR_IF_( !(stat.valid & ZIP_STAT_NAME) );
    return string( stat.name );
}

// Uncompressed size of entry.
zip_uint64_t ZipStat::size() const {
    ERR_IF_( !(stat.valid & ZIP_STAT_SIZE) );
    return stat.size;
}

// Compressed size of entry.
zip_uint64_t ZipStat::comp_size() const {
    ERR_IF_( !(stat.valid & ZIP_STAT_COMP_SIZE) );
    return stat.comp_size;
}

// Last mode time.  This will be rounded to the nearest
// two-second boundary and contains no timezone.  Also,
// zip files do not store timezone.  So the time returned
// by this function must be interpreted based on the
// known timezone of the machine that created the zip.
time_t ZipStat::mtime() const {
    ERR_IF_( !(stat.valid & ZIP_STAT_MTIME) );
    return stat.mtime;
    //return chrono::system_clock::from_time_t( stat.mtime );
}
