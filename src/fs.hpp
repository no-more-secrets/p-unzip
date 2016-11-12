/****************************************************************
 * File-system operations
 ***************************************************************/
#pragma once

#include <string>
#include <vector>

/****************************************************************
 * FilePath
 ***************************************************************/
/* This class is an immutable representation of a file path.
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
 ***************************************************************/
/* Create folder and all parents, and do not fail if it already
 * exists.  Will throw on any other error.  Note: if you are
 * creating multiple folders in succession then you should use
 * mkdirs_p below as it will be more efficient. */
void mkdir_p( FilePath const& path );

/* Has the effect of calling mkdir_p on each of the elements in
 * the list.  Implementation is efficient in that it will use a
 * a cache to avoid redundant calls to the filesystem. */
void mkdirs_p( std::vector<FilePath> const& paths );

/****************************************************************
 * Functions that require platform-specific implementations
 ***************************************************************/
namespace fs {

/* A place for holding results of a file system stat which will
 * be platform independent.  If the `exists` field is false then
 * all other fields are undefined. */
struct Stat {
    bool exists;
    bool is_folder;
};

/* Fills out a Stat structure with info about path.  Note that
 * this will not throw if the path does not exist, it will
 * simply set the appropriate flag in the structure. */
Stat stat( char const* path );

/* Create folder and fail if it already exists or if one of
 * the parents in the path does not exist. */
void create_folder( char const* path );

} // namespace fs
