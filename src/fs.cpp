/****************************************************************
* File system related functionality.  Both high-level methods
* as well as platform-dependent function calls.
****************************************************************/
#include "config.hpp"
#include "macros.hpp"
#include "fs.hpp"
#include "utils.hpp"

#include <string>
#include <set>
#include <algorithm>

using namespace std;

/****************************************************************
* Functions that require platform-specific implementations.
****************************************************************/
#ifndef POSIX
#   include <windows.h>
#endif

// Apparently these work on Windows as well as posix.
#include <sys/types.h>
#include <sys/stat.h>

// These are for setting time stamps of files
#ifdef POSIX
#    include <utime.h>
#else
#    include <sys/utime.h>
#endif

namespace {

/* Holder for file info in a platform-independent format. */
struct Stat {
    bool exists;
    bool is_folder;
};

/* Return a platform-independent structure that will give info
 * about the supplied path.  Note that if the returned structure
 * has exists==false then all other fields are undefined. */
Stat stat( char const* path ) {
    Stat res;
    // The real OS-specific structure.
    struct OS_SWITCH( stat, _stat ) buf;
    auto ret = ::OS_SWITCH( stat, _stat )( path, &buf );
    if( ret != 0 ) {
        ret = errno;
        FAIL( ret != ENOENT, "stat encountered an error other "
            "than ENOENT: " << ret );
        res.exists = false;
        return res;
    }
    res.exists = true;
    res.is_folder = bool(
        buf.st_mode & OS_SWITCH( S_IFDIR, _S_IFDIR ) );
    return res;
}

/* Create folder and fail if it already exists or if one of
 * the parents in the path does not exist. */
void create_folder( char const* path ) {
#ifdef POSIX
    auto mode = S_IRUSR | S_IWUSR | S_IXUSR |
                S_IRGRP |           S_IXGRP |
                S_IROTH |           S_IXOTH;
    FAIL( mkdir( path, mode ),
        "create folder failed on path: " << path );
#else
    FAIL( !CreateDirectory( path, NULL),
        "create folder failed on path: " << path );
#endif
}

} // namespace

/****************************************************************
* File
****************************************************************/
File::File( string const& s, char const* m ) : mode( m ) {
    FAIL( mode != "rb" && mode != "wb",
        "unrecognized mode " << mode );
    p = fopen( s.c_str(), m );
    FAIL( !p, "failed to open " << s << " with mode " << mode );
    own = true;
}

void File::destroyer() { fclose( p ); }

// Will read the entire contents of the file from the current
// File position and will leave the file position at EOF.
Buffer File::read() {
    FAIL_( fseek( p, 0, SEEK_END ) != 0 );
    size_t length = ftell( p );
    rewind( p );

    Buffer buffer( length );

    auto length_read = fread( buffer.get(), 1, length, p );
    FAIL_( length != length_read );

    return buffer;
}

// Will write the entire contents of buffer to file starting
// from the file's current position.  Will throw if not all
// bytes written.
void File::write( Buffer const& buffer, size_t count ) {
    FAIL( mode != "wb", "attempted write in mode " << mode );
    FAIL_( count > buffer.size() );
    size_t written = fwrite( buffer.get(), 1, count, p );
    FAIL_( written != count );
}

/****************************************************************
* FilePath class
****************************************************************/
/* Here we will basically split the path at the forward slashes
 * and store each component in the m_components vector.  We will
 * throw if we are given an absolute path or a path with back-
 * slashes.  Note that an empty string is a valid FilePath and
 * will result in a FilePath with an empty list of components.
 * This has the meaning of the "." folder. */
FilePath::FilePath( string const& path ) {
    if( path.empty() )
        return;
    FAIL( starts_with( path, '/' ),
        "Rooted path " << path << " not supported." );
    FAIL( find( path.begin(), path.end(), ':' ) != path.end(),
        "Rooted path " << path << " not supported." );
    FAIL( find( path.begin(), path.end(), '\\' ) != path.end(),
        "backslashes in path are not supported" );
    // TODO: This should be replaced with a generic "split"
    //       algorithm.
    string::const_iterator next = path.begin();
    while( true ) {
        string::const_iterator first = next;
        next = find( first, path.end(), '/' );
        string c( first, next );
        if( !c.empty() )
            m_components.push_back( c );
        if( next == path.end() )
            break;
        ++next;
    }
}

// Assemble the components into a string where the components
// are separated by slashes.  There will never be a slash at
// the beginning or a slash at the end.
string FilePath::str() const {
    string res;
    if( m_components.empty() )
        return res;
    for( auto const& c : m_components )
        res += c + "/";
    res.pop_back();
    return res;
}

// If there is at least one component then this will return the
// FilePath representing the parent path.  Note that when only
// one component remains then it will return the "current"
// folder which is represented as an empty path/string (i.e., it
// is not a dot ".").  If dirname is called on an empty path
// then it will throw.
FilePath FilePath::dirname() const {
    FAIL( m_components.size() == 0,
        "no more parent folders available in dirname" );
    FilePath dir( *this );
    dir.m_components.pop_back();
    return dir;
}

// For convenience.  Will just call str() and then output.
std::ostream& operator<<( std::ostream& out,
                          FilePath const& path ) {
    return (out << path.str());
}

/****************************************************************
* High-level file system utilities
*****************************************************************
* This is a helper function which will consult a cache
* before hitting the file system in order to help implement
* mkdirs_p.  Any FilePath in the cache is assumed to exist. It
* uses recursion to ensure that a parent path is constructed
* before its child.  It will not throw if one or more folders
* already exist, but it will throw if one of them exists but
* is not a folder. */
void mkdir_p( set<FilePath>& cache, FilePath const& path ) {
    if( path.empty() || has_key( cache, path ) )
        return;
    mkdir_p( cache, path.dirname() );
    cache.insert( path );
    string s_path( path.str() );
    char const* c_path  = s_path.c_str();
    Stat info( stat( c_path ) );
    if( info.exists && info.is_folder )
        return;
    FAIL( info.exists,
        "Path " << s_path << " exists but is not a folder." );
    create_folder( c_path );
}

/* Create folder and all parents, and do not fail if it already
 * exists.  Will throw on any other error.  Note: if you are
 * creating multiple folders in succession then you should use
 * mkdirs_p below as it will be more efficient. */
void mkdir_p( FilePath const& path ) {
    set<FilePath> empty;
    mkdir_p( empty, path );
}

/* Has the effect of calling mkdir_p on each of the elements in
 * the list.  Implementation is efficient in that it will use a
 * a cache to avoid redundant calls to the filesystem. */
void mkdirs_p( std::vector<FilePath> const& paths ) {
    set<FilePath> cache;
    for( auto const& path : paths )
        mkdir_p( cache, path );
}

// Set the time stamp of a file given a path.  Will throw if it
// fails.  Will set both mod time and access time to this value.
// Since we're using time_t this means the resolution is only at
// the level of one second, however this is fine here because zip
// files only have a resolution of two seconds.  The time is
// interpreted as the epoch time (so it implicitly has a time
// zone).  However note that zip files do not carry any time zone
// information, so interpreting a timestamp from a zip file as
// an epoch time can cause inconsistencies when dealing with
// zip files that are zipped and unzipped in different timezones.
void set_timestamp( string const& path, time_t time ) {
    OS_SWITCH( utimbuf, _utimbuf ) times;
    times.actime  = time;
    times.modtime = time;
    auto res = OS_SWITCH( utime, _utime )( path.c_str(), &times );
    FAIL( res == -1, "failed to set timestamp on " << path );
}

// Rename a file.  Will detect when arguments are equal and do
// nothing.
void rename_file( string const& path, string const& path_new ) {
    if( path == path_new )
        return;
    auto func = OS_SWITCH( ::rename, MoveFile );
    FAIL_( !func( path.c_str(), path_new.c_str() ) );
}