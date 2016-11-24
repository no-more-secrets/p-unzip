/****************************************************************
* File-system operations
****************************************************************/
#pragma once

#include "ptr_resource.hpp"
#include "utils.hpp"

#include <string>
#include <vector>

/****************************************************************
* Resource manager for C FILE handles
****************************************************************/
class File : public PtrRes<FILE, File> {

    std::string mode;

public:
    File( std::string const& s, char const* mode );

    void destroyer();

    // Will read the entire contents of the file from the current
    // File position and will leave the file position at EOF.
    Buffer read();

    // Will write `count` bytes of buffer to file starting
    // from the file's current position.  Will throw if not all
    // bytes written.
    void write( Buffer const& buffer, size_t count );
};

/****************************************************************
* FilePath
*****************************************************************
* This class is an immutable representation of a file path.
* It only holds relative paths, as opposed to abosolute paths
* that are rooted at / (Posix) or a drive letter (Windows). */
class FilePath {

public:
    FilePath( std::string const& path );

    FilePath( FilePath const& from )
        : m_components( from.m_components ) {}

    FilePath( FilePath&& from )
        : m_components( std::move( from.m_components ) ) {}

    FilePath& operator=( FilePath&& from ) {
        m_components = std::move( from.m_components );
        return *this;
    }

    // Assemble the components into a string where the components
    // are separated by slashes.
    std::string str() const;

    // True if there are zero components.
    bool empty() const { return m_components.empty(); }

    // Remove leading component, throw if there are no more.
    FilePath dirname() const;

    // Lexicographical comparison.
    bool operator<( FilePath const& right ) const {
        return m_components < right.m_components;
    }

private:
    std::vector<std::string> m_components;

};

// For convenience.  Will just call str() and then output.
std::ostream& operator<<( std::ostream& out, FilePath const& path );

/****************************************************************
* High-level file system functions
****************************************************************/

// Create folder and all parents, and do not fail if it already
// exists.  Will throw on any other error.  Note: if you are
// creating multiple folders in succession then you should use
// mkdirs_p below as it will be more efficient.
void mkdir_p( FilePath const& path );

// Has the effect of calling mkdir_p on each of the elements in
// the list.  Implementation is efficient in that it will use a
// a cache to avoid redundant calls to the filesystem.
void mkdirs_p( std::vector<FilePath> const& paths );
