/****************************************************************
* File-system operations
****************************************************************/
#pragma once

#include "handle.hpp"
#include "utils.hpp"

#include <string>
#include <vector>

/****************************************************************
* Resource manager for C FILE handles
****************************************************************/
class File : public Handle<FILE, File> {

    std::string mode;

public:
    File( std::string const& s, char const* mode );

    void destroyer();

    // Will read the entire contents of the file from the current
    // File position and  will  leave  the  file  position at EOF.
    Buffer read();

    // Will  write  `count` bytes of buffer to file starting from
    // the file's current position. Will  throw  if not all bytes
    // written.
    void write( Buffer const& buffer, uint64_t count );
};

/****************************************************************
* FilePath
*****************************************************************
* This class is an immutable representation of a  file  path.  It
* only holds relative paths,  as  opposed  to absolute paths that
* are rooted at / (Posix) or  a drive letter (Windows). Note that
* these  are  not  constrained to represent real paths, and their
* components may even be empty. */
class FilePath {

public:
    FilePath() {}

    FilePath( std::string const& path );

    FilePath( FilePath const& from )
        : m_components( from.m_components ) {}

    FilePath( FilePath&& from )
        : m_components( std::move( from.m_components ) ) {}

    // Assemble the components into a string where the components
    // are separated by slashes.
    std::string str() const;

    // True if there are zero components.
    bool empty() const { return m_components.empty(); }

    // Remove leading component,  throw  if  there  are  no  more.
    FilePath dirname() const;

    // Get basename if one exists; this means basically just  the
    // last component of the path.
    std::string const& basename() const;

    // Adds the given string to  the  last component. Creates one
    // if  there  is no last component. Note: this does not add a
    // dot automatically.
    FilePath add_ext( std::string const& ext ) const;

    // Mutating join with another FilePath
    FilePath join( FilePath const& fp ) const {
        FilePath res( *this );
        for( auto const& p : fp.m_components )
            res.m_components.push_back( p );
        assert_invariants();
        return res;
    }

    // Lexicographical comparison.
    bool operator<( FilePath const& right ) const {
        return m_components < right.m_components;
    }

private:
    // Check if everything is kosher and throw if not.
    void assert_invariants() const;

    std::vector<std::string> m_components;

};

// For convenience
using OptPairFilePath = Optional<std::pair<FilePath,FilePath>>;

// Similiar to the std::string variant of  split_ext,  but  takes
// FilePaths, and only considers  dots  in  the last component of
// the path.
OptPairFilePath split_ext( FilePath const& fp );

// Join two paths efficiently with moving
FilePath operator/( FilePath const& left, FilePath const& right );

// For convenience. Will just call str() and then output.
std::ostream& operator<<( std::ostream& out, FilePath const& path );

/****************************************************************
* Utilities
****************************************************************/

// For convenience
using OptPairStr = Optional<std::pair<std::string,std::string>>;

// If the string contains at least one '.' then it will split the
// string on the last dot and  return  the substrings that are to
// the left and right of it. The dot on which the string is split
// is removed; this means that this dot will not appear in either
// of the output strings, although the "left" component  may  con-
// tain other dots.
OptPairStr split_ext( std::string const& s );

/****************************************************************
* High-level file system functions
****************************************************************/

// Create  folder  and all parents, and do not fail if it already
// exists.  Will  throw  on any other error. Note: if you are cre-
// ating multiple  folders  in  succession  then  you  should use
// mkdirs_p below as it will be more efficient.
void mkdir_p( FilePath const& path );

// Has the effect of calling mkdir_p on each of the  elements  in
// the  list. Implementation is efficient in that it will use a a
// cache to avoid redundant calls to the filesystem.
void mkdirs_p( std::vector<FilePath> const& paths );

// Set  the  time  stamp of a file given a path. Will throw if it
// fails. Will set both mod  time  and  access time to this value.
// Since  we're using time_t this means the resolution is only at
// the level of one second, however this is fine here because zip
// files only have a resolution  of  two  seconds. The time is in-
// terpreted as the epoch time  (so  it  implicitly  has  a  time
// zone). However, note that zip files do not carry any time zone
// information, so interpreting a timestamp from a zip file as an
// epoch time can  cause  inconsistencies  when  dealing with zip
// files  that  are  zipped  and  unzipped in different timezones.
void set_timestamp( std::string const& path, time_t time );

// Rename a file. Will  detect  when  arguments  are equal and do
// nothing.
void rename_file( std::string const& path,
                  std::string const& path_new );
