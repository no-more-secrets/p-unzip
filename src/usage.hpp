/****************************************************************
 * Application-specific data describing the kinds of command line
 * arguments and flags that are expected/allowed/required.
 ***************************************************************/
#pragma once

namespace usage {

static char const* info =
    "p-unzip: multithreaded unzipper."                "\n"
    "Usage: p-unzip [options] file.zip"               "\n"
    ""                                                "\n"
    "   -h          : show usage and exit"            "\n"
    "   -j N        : use N threads"                  "\n"
    "   -d strategy : specify distribution strategy"  "\n"
    "                 cyclic|sliced|folder|bytes"     "\n"
    "                 Default is cyclic."             "\n";

// Options that do not take a value
static auto options_no_val   = { 'h' };
// Options that must take a value
static auto options_val      = { 'j', 'd' };

// Minimum number of positional arguments that any valid
// commandline must have.
static size_t min_positional = 1;

} // namespace usage
