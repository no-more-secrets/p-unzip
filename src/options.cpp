/****************************************************************
 * General utilities
 ***************************************************************/
#include "options.hpp"

#include <stdexcept>

using namespace std;

namespace options {

/****************************************************************
 * OptionVal: Struct for holding an optional value
 ***************************************************************/
string const& OptionVal::get() {
    if( !has_value )
        throw runtime_error(
             "attempt to get value from OptionVal that has no value." );
    return value;
}

/****************************************************************
 * Options parser.  Implemented using recursion.
 ***************************************************************/
bool parse( int argc,
            char** argv,
            set<char> with_value,
            OptResult& res ) {

    // If arg list is empty then that's always valid
    if( !argc )
        return true;

    string arg( argv[0] );

    // Is this the start of a non-option (i.e., positional)
    // argument.  If so then add it to the list and recurse on
    // the remainder.
    if( arg[0] != '-' ) {
        res.second.push_back( arg );
        return parse( argc-1, argv+1, with_value, res );
    }

    // At this point we have an option argument, so it must
    // have at least two chars since the first is the '-' sign.
    if( arg.size() == 1 )
        return false;

    // Is this option one that needs a value?
    if( with_value.find( arg[1] ) != with_value.end() ) {
        // This option needs a value; is it attached to this
        // parameter, or a separate argument?
        if( arg.size() > 2 ) {
            // It is attached to this parameter, so extract it
            // and continue.
            res.first[arg[1]] = OptionVal( string( arg.c_str()+2 ) );
            return parse( argc-1, argv+1, with_value, res );
        }

        // Value must be next parameter, so therefore we must
        // have at least one more parameter.
        if( argc < 2 )
            return false;

        // Get next parameter and make sure it's not another option,
        // since it's supposed to be the value of this option.
        string value( argv[1] );
        if( value[0] == '-' )
            return false;

        // Option value is found and good, so record it and move on.
        res.first[arg[1]] = OptionVal( value );
        return parse( argc-2, argv+2, with_value, res );
    }

    // It is an option but should not take a parameter.  Make sure
    // that it only has one more character beyond the initial '-'.
    if( arg.size() > 2 )
        return false;

    // Looks good, so store it with empty value.
    res.first[arg[1]] = OptionVal();
    return parse( argc-1, argv+1, with_value, res );
}

}
