/****************************************************************
 * Platform-specific implementations of file system operations.
 ***************************************************************/
#include <string>

namespace fs {

/* Returns true if path exists. */
bool exists( std::string const& path ) {

}

/* Returns true if path exists and is a folder. */
bool is_folder( std::string const& path ) {

}

/* Create folder and fail if it already exists or if one of
 * the parents in the path does not exist. */
void create_folder( std::string const& path ) {

}

/* Create folder and all parents, and do not fail if it already
 * exists.  Will throw if a directory creation fails. */
void mkdir_p( FilePath const& fp ) {

}

} // namespace fs
