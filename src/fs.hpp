/****************************************************************
 * Platform-specific implementations of file system operations.
 ***************************************************************/

#include <string>

/* Returns true if path exists. */
bool exists( std::string const& path );

/* Returns true if path exists and is a folder. */
bool is_folder( std::string const& path );

/* Create folder and fail if it already exists or if one of
 * the parents in the path does not exist. */
bool create_folder( std::string const& path );
