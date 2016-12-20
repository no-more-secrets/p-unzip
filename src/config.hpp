/****************************************************************
* Basic program configuration.
****************************************************************/
#pragma once

#include <cstddef>

// This macro will be used by  the  rest  of the code to know the
// platform (among those that are supported).
#undef PLATFORM
// For the moment, this must be set,  because we need the user to
// tell us how to fill in  the  PLATFORM variable, although it is
// possible that we could eventually auto detect this.
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

// This is for convenience  when  small  bits  of  code depend on
// platform. The first argument will be used if we are on a posix
// system, and the second otherwise.
#ifdef POSIX
#    define OS_SWITCH( a, b ) a
#elif PLATFORM == WIN
#    define OS_SWITCH( a, b ) b
#endif
