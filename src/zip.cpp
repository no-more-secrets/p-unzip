#include "zip.hpp"

#include <stdexcept>

using namespace std;

/****************************************************************
* Zip
****************************************************************/
Zip::Zip( Buffer::SP& b_ ) : b( b_ ) {
    // First creat a "zip source" from the raw pointer to the
    // buffer containing the binary data of the zip file.  The
    // zip source will not take ownership of the buffer.
    // TODO: need to call error APIs to extract error msg
    zip_error_t   error;
    zip_source_t* zs;
    zs = zip_source_buffer_create( b->get(), b->size(), 0, &error );
    FAIL( !zs, "failed to create zip source from buffer" );
    // Now create a zip_t object which attaches to (and owns)
    // the zip source.
    p = zip_open_from_source( zs, ZIP_RDONLY, &error );
    FAIL( !p, "failed to open zip from source" );
    own = true;
    // Lastly, count number of files in the archive and get
    // each of their stats and cache them.
    size_t size = zip_get_num_entries( p, ZIP_FL_UNCHANGED );
    for( size_t i = 0; i < size; ++i ) {
        zip_stat_t stat;
        FAIL( zip_stat_index( p, i, ZIP_FL_UNCHANGED, &stat ),
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

// Uncompress a file directly to disk and allow caller
// to supply a buffer to hold the chunks and to control
// chunk size.
void Zip::extract_to( size_t  idx,
                      string  const& file,
                      Buffer& buf ) const {
    FAIL_( buf.size() == 0 );
    // First open the file to which we will write the result.
    File out( file, "wb" );
    // Get uncompressed file size
    zip_int64_t fsize = at( idx ).size();
    zip_file_t* zf;
    // "open" the zip file; this is not really opening a file.
    FAIL_( !(zf = zip_fopen_index( p, idx, 0 )) );
    // !! Should not throw until zip_fclose is called
    zip_int64_t total = 0;
    while( true ) {
        auto read = zip_fread( zf, buf.get(), buf.size() );
        // May return -1 on error; not clear whether it will
        // return zero if no error but no bytes read.  In
        // either case, it's probably correct to just break
        // out of the loop instead of throwing if read <= 0.
        if( read <= 0 ) break;
        out.write( buf, read );
        // We need to keep a running total of bytes written
        // so that we can check at the end if they were all
        // written.  This is because it is not guaranteed
        // that, if we break out of the loop, the entire file
        // was written.
        total += read;
    }
    // !! Close immediately to avoid resource leak.
    zip_fclose( zf );
    // If we haven't read a number of bytes equal to the
    // reported size of the uncompressed file then throw.
    // Otherwise succeed.  This should be adequate no
    // matter how we got here and/or whether zip_fread
    // returned -1 or not.
    FAIL_( total != fsize );
}

// Uncompress file into existing buffer.  Throws if the
// buffer is not big enough.
void Zip::extract_in( size_t idx, Buffer& buffer ) const {
    size_t fsize = at( idx ).size();
    FAIL_( fsize > buffer.size() );
    zip_file_t* zf;
    FAIL_( !(zf = zip_fopen_index( p, idx, 0 )) );
    // !! Should not throw until zip_fclose is called
    zip_int64_t count = zip_fread( zf, buffer.get(), fsize );
    // !! Close immediately to avoid resource leak.
    zip_fclose( zf );
    // zip_fread can return -1 on error
    FAIL_( count < 0 );
    // If we haven't read a number of bytes equal to the
    // reported size of the uncompressed file then throw.
    FAIL_( zip_uint64_t( count ) != fsize );
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
    FAIL_( idx >= stats.size() );
    return stats[idx];
}

/****************************************************************
* ZipStat
****************************************************************/
// This is the zero-based index within the archive of the
// element represented by this ZipStat.
zip_uint64_t ZipStat::index() const {
    FAIL_( !(stat.valid & ZIP_STAT_INDEX) );
    return stat.index;
}

// File/folder name of entry.  Folder names end with /
string ZipStat::name() const {
    FAIL_( !(stat.valid & ZIP_STAT_NAME) );
    return string( stat.name );
}

// Uncompressed size of entry.
zip_uint64_t ZipStat::size() const {
    FAIL_( !(stat.valid & ZIP_STAT_SIZE) );
    return stat.size;
}

// Compressed size of entry.
zip_uint64_t ZipStat::comp_size() const {
    FAIL_( !(stat.valid & ZIP_STAT_COMP_SIZE) );
    return stat.comp_size;
}

// Last mode time.  This will be rounded to the nearest
// two-second boundary and contains no timezone.  Also,
// zip files do not store timezone.  So the time returned
// by this function must be interpreted based on the
// known timezone of the machine that created the zip.
time_t ZipStat::mtime() const {
    FAIL_( !(stat.valid & ZIP_STAT_MTIME) );
    return stat.mtime;
    //return chrono::system_clock::from_time_t( stat.mtime );
}

// Will return true if the entry represents a folder,
// which is if the name ends in a forward slash.
bool ZipStat::is_folder() const {
    return ends_with( name(), '/' );
}

// If the entry is a folder then it will return the name
// in the entry itself, otherwise it will strip off the
// filename and return the parent folders.
FilePath ZipStat::folder() const {
    FilePath res( name() );
    if( !is_folder() )
        return res.dirname();
    return res;
}
