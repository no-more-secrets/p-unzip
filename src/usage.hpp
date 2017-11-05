/****************************************************************
* Application-specific data describing the kinds of command  line
* arguments and flags that  are  expected/allowed/required.  This
* header  is  only  intended  to  be  included by a single module.
****************************************************************/
#pragma once

namespace usage {

static char const* info =
    "p-unzip: multithreaded unzipper."                       "\n"
    "Usage: p-unzip [options] file.zip"                      "\n"
    ""                                                       "\n"
    "   -h          : Show usage and exit"                   "\n"
    ""                                                       "\n"
    "   -q          : Quiet -- do not print files names"     "\n"
    ""                                                       "\n"
    "   -t          : Timestamps policy. Can be one of:"     "\n"
    "                 current - use timestamp at time of"    "\n"
    "                           writing."                    "\n"
    "                 <sec>   - where <sec> is the epoch"    "\n"
    "                           time in seconds."            "\n"
    "                 Default behavior is to use timestamps" "\n"
    "                 stored in zip file."                   "\n"
    ""                                                       "\n"
    "   -j N        : Use N threads.  In addition to a num-" "\n"
    "                 erical value, N can be one of:"        "\n"
    "                 { max, auto }.  max is max threads"    "\n"
    "                 available, auto is optimal number for" "\n"
    "                 this system."                          "\n"
    ""                                                       "\n"
    "   -d strategy : Specify distribution strategy"         "\n"
    "                 Can be: cyclic, sliced, bytes,"        "\n"
    "                 folder_bytes, or folder_files."        "\n"
    "                 Default is cyclic."                    "\n"
    ""                                                       "\n"
    "   -c size     : Specify chunk size in bytes.  These"   "\n"
    "                 are the blocks in which data is"       "\n"
    "                 decompressed and written to disk."     "\n"
    "                 Default is some sensible value."       "\n"
    ""                                                       "\n"
    "   -o          : Specify output folder.  This folder"   "\n"
    "                 will be prepended to all files in the" "\n"
    "                 archive before extraction."            "\n"
    ""                                                       "\n"
    "   -a          : Avoid creating files with long"        "\n"
    "                 extensions.  Instead, create them"     "\n"
    "                 with short extensions then rename."    "\n"
    "                 This option was designed for use only" "\n"
    "                 on Windows machines running AV, in"    "\n"
    "                 which case this flag has been"         "\n"
    "                 observed to significantly improve"     "\n"
    "                 extraction times.  All other users"    "\n"
    "                 or platforms should ignore it."        "\n"
    ""                                                       "\n"
    "   -g          : Output diagnostic info to stderr."     "\n";

// Options that do not take a value
static auto options_no_val   = { 'h', 'q', 'a', 'g' };
// Options that must take a value
static auto options_val      = { 'j', 'd', 'c', 't', 'o' };

// Minimum number of positional arguments  that any valid command-
// line must have.
static size_t min_positional = 1;

} // namespace usage
