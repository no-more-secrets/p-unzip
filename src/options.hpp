/****************************************************************
 * General utilities
 ***************************************************************/
#pragma once

#include <utility>
#include <map>
#include <vector>
#include <string>
#include <set>

namespace options {

/****************************************************************
 * OptionVal: Struct for holding an optional value
 ***************************************************************/
struct OptionVal {

    bool has_value;
    std::string value;

public:
    OptionVal() : has_value( false ) {}

    OptionVal( std::string const& s )
        : has_value( true )
        , value( s )
    {}

    std::string const& get();
};

using Positional = std::vector<std::string>;
using Options    = std::map<char, OptionVal>;
using OptResult  = std::pair<Options, Positional>;

/****************************************************************
 * This does the full parsing of the arguments
 ***************************************************************/
bool parse( int                   argc,
            char**                argv,
            std::set<char> const& options,
            std::set<char> const& with_value,
            OptResult&            result );

} // namespace options
