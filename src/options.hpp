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

using OptResult = std::pair<
                      std::map<char, OptionVal>,
                      std::vector<std::string>
                  >;

/****************************************************************
 * This does the full parsing of the arguments
 ***************************************************************/
bool parse( int argc,
            char** argv,
            std::set<char>,
            OptResult& result );

} // namespace options
