/****************************************************************
* Application-specific data describing the kinds of command line
* arguments and flags that are expected/allowed/required.  This
* header is only intended to be included by a single module.
****************************************************************/
#pragma once

namespace usage {

static char const* info =
    "p-unzip: multithreaded unzipper."                       "\n"
    "Usage: p-unzip [options] file.zip"                      "\n"
    ""                                                       "\n"
    "   -h          : show usage and exit"                   "\n"
    ""                                                       "\n"
    "   -q          : quiet: do not print files names"       "\n"
    ""                                                       "\n"
    "   -j N        : use N threads.  In addition to a num-" "\n"
    "                 erical value, N can be one of:"        "\n"
    "                 { MAX, OPT }.  MAX is max threads"     "\n"
    "                 available, OPT is optimal number for"  "\n"
    "                 this system."                          "\n"
    ""                                                       "\n"
    "   -d strategy : specify distribution strategy"         "\n"
    "                 cyclic|sliced|folder|bytes"            "\n"
    "                 Default is cyclic."                    "\n"
    ""                                                       "\n"
    "   -c size     : specify chunk size."                   "\n";

// Options that do not take a value
static auto options_no_val   = { 'h', 'q' };
// Options that must take a value
static auto options_val      = { 'j', 'd', 'c' };

// Minimum number of positional arguments that any valid
// commandline must have.
static size_t min_positional = 1;

} // namespace usage
