/****************************************************************
 * Program entry point.  Processes command line arguments.
 ***************************************************************/
#include "options.hpp"
#include "usage.hpp"

#include <iostream>
#include <exception>

using namespace std;

// This is expected to reside somewhere in another module.
int main_( options::opt_result::first_type,
           options::opt_result::second_type );

// If we are exiting in error then print usage info to stderr,
// otherwise print it to stdout.
void usage_exit( int code = 0 ) {
    auto& out = code ? cerr : cout;
    out << usage::info;
    exit( code );
}

// Wrapper around the application's main() function that
// will handle option parsing and catching/printing uncaught
// exceptions.
int main( int argc, char* argv[] )
{
    try {
        // Options that must have a value
        set<char> opt_val    = usage::options_val;
        // Options that have no value
        set<char> opt_no_val = usage::options_no_val;
        // Union of both sets of above options
        set<char> opt_all    = opt_no_val;
        opt_all.insert( opt_val.begin(), opt_val.end() );

        // Do the parsing/processing of arguments, taking
        // into account the above sets representing what
        // the application is expecting.
        options::opt_result opt_result;
        if( !options::parse( argc,
                             argv,
                             opt_all,
                             opt_val,
                             opt_result ) )
            exit( 1 );

        auto& positional = opt_result.first;
        auto& options    = opt_result.second;

        // We always support the -h (help) option.
        if( options.count( 'h' ) )
            usage_exit();

        if( positional.size() < usage::min_positional )
            usage_exit( 1 );

        // Call the application's main program.
        return main_( positional, options );

    } catch( exception const& e ) {
        cerr << e.what() << endl;
        return 1;
    }
}
