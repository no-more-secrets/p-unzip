/****************************************************************
* Command line options processing. This module is generic and
* does not depend on the specific options of any one program.
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

// For convenience: this function  is  used  ONLY on options that
// can take values, and furthermore, it is assumed that all
// options in the map that must take values will have values.
// This will be the case if this  options map was prepared by the
// framework. In any case, it will get  the value of an option if
// that option is present in the map. If the option is not
// present in the map it will return the default value. Again, if
// the option is present in the map but has an empty Optional
// value then an exception will be throw.
std::string option_get( options& op, char  k,
                        std::string const& def );

/****************************************************************
* This does the full parsing of the arguments
****************************************************************/
bool parse( int                   argc,
            char**                argv,
            std::set<char> const& options_all,
            std::set<char> const& options_with_val,
            opt_result&           result );

} // namespace options
