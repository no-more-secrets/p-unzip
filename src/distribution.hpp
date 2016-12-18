/****************************************************************
* Interfaces for taking a  list  of  zip entries and distributing
* the files among a number of threads.
****************************************************************/
#pragma once

#include "utils.hpp"
#include "zip.hpp"

#include <map>
#include <string>
#include <vector>

using index_lists = std::vector<std::vector<uint64_t>>;
using files_range = Range<std::vector<ZipStat>::iterator>;
typedef index_lists (*distributor_t)( size_t, files_range const& );

// This is the global dictionary  that  will  hold a mapping from
// strategy name to function pointer.  When called, that function
// will distribute zip entries among  a  given number of threads.
extern std::map<std::string, distributor_t> distribute;
