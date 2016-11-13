/****************************************************************
 * Basic program configuration.
 ***************************************************************/
#pragma once

#include <cstddef>

// This macro will be used by the rest of the code to know
// the platform (among those that are supported).
#undef PLATFORM
// For the moment, this must be set, because we need the user
// to tell us how to fill in the PLATFORM variable, although it
// is possible that we could eventually auto detect this.
#ifdef OS_LINUX
#   define PLATFORM Linux
#   define POSIX
#else
#ifdef OS_OSX
#   define PLATFORM OSX
#   define POSIX
#else
#ifdef OS_WIN
#   define PLATFORM WIN
#   undef POSIX
#else
#   error "None of OS_LINUX, OS_OSX, OS_WIN have been defined."
#endif
#endif
#endif

/****************************************************************
 * Compile-time program parameters
 ***************************************************************/
// The maximum value that the `-j` parameter can take.
size_t const MAX_JOBS    = 64;
// This is the default distribution strategy to use if the
// user does not specify on the commandline.
char const* default_dist = "cyclic";
