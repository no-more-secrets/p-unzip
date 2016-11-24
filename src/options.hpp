/****************************************************************
* Command line options parsing engine
****************************************************************/
#pragma once

#include "utils.hpp"

#include <utility>
#include <map>
#include <vector>
#include <string>
#include <set>

namespace options {

using positional = std::vector<std::string>;
using options    = std::map<char, Optional<std::string>>;
using opt_result = std::pair<positional, options>;

/****************************************************************
* This does the full parsing of the arguments
****************************************************************/
bool parse( int                   argc,
            char**                argv,
            std::set<char> const& options_all,
            std::set<char> const& options_with_val,
            opt_result&           result );

} // namespace options
