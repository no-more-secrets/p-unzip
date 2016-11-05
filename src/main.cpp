#include "ptr_resource.hpp"
#include "utils.hpp"
#include "zip.hpp"
#include "options.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

using namespace std;

void usage() {
    cerr << "p-unzip: multithreaded unzipper." << endl;
    cerr << "Usage: p-unzip [-j N] file.zip" << endl;
    cerr << endl;
    cerr << "    N - number of threads." << endl;
    exit( 1 );
}

void help() {
    cout << "Help:" << endl;
    exit( 0 );
}

int main_( options::Positional positional,
           options::Options    options )
{
    if( options.count( 'h' ) )
        help();

    int jobs = 1;
    if( options.count( 'j' ) ) {
        jobs = atoi( options['j'].get().c_str() );
        if( jobs <= 0 )
            throw runtime_error( "invalid number of jobs" );
    }

    if( positional.size() != 1 )
        usage();

    string filename = positional[0];
    log( "File: " + filename );

    ostringstream ss;
    ss << jobs;
    log( "Jobs: " + ss.str() );

    Buffer::SP buf =
        make_shared<Buffer>( File( filename, "rb" ).read() );

    Zip z( buf );

    cout << "Found " << z.size() << " entries:" << endl;

    for( size_t i = 0; i < z.size(); ++i ) {

        ZipStat s = z[i];

        cout << "  index: "       << s.index()     << endl;
        cout << "    name:      " << s.name()      << endl;
        cout << "    size:      " << s.size()      << endl;
        cout << "    comp_size: " << s.comp_size() << endl;
        cout << "    mtime:     " << s.mtime()     << endl;
    }

    return 0;
}

int main( int argc, char* argv[] )
{
    set<char> options{ 'j', 'h' };
    set<char> options_with_value{ 'j' };

    try {

        if( argc == 1 )
            usage();

        options::OptResult opt_result;
        bool parsed = options::parse( argc,
                                      argv,
                                      options,
                                      options_with_value,
                                      opt_result );
        auto& positional = opt_result.second;
        auto& options    = opt_result.first;

        if( !parsed )
            exit( 1 );

        return main_( positional, options );

    } catch( exception const& e ) {
        cerr << "exception: " << e.what() << endl;
        return 1;
    }
}
