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
#include <algorithm>

using namespace std;

char const* info =
R"R(p-unzip: multithreaded unzipper.
Usage: p-unzip [-j N] file.zip

    N - number of threads.
)R";

void usage() {
    cerr << info;
    exit( 1 );
}

int main_( options::Positional positional,
           options::Options    options )
{
    if( options.count( 'h' ) ) {
        usage();
        exit( 0 );
    }

    size_t jobs = 1;
    if( options.count( 'j' ) ) {
        jobs = atoi( options['j'].get().c_str() );
        if( jobs <= 0 )
            throw runtime_error( "invalid number of jobs" );
    }

    if( positional.size() != 1 ) {
        usage();
        exit( 1 );
    }

    string filename = positional[0];
    log( "File: " + filename );

    ostringstream ss;
    ss << jobs;
    log( "Jobs: " + ss.str() );

    Buffer::SP buf =
        make_shared<Buffer>( File( filename, "rb" ).read() );

    //////////////////////////////////////////////////////////

    Zip z( buf );
    cout << "Found " << z.size() << " entries:" << endl;

    zip_uint64_t max_size = 0;
    for( size_t i = 0; i < z.size(); ++i )
        max_size = max( max_size, z[i].size() );
    cout << "Max uncompressed size: " << max_size << endl;

    auto unzip = [jobs, max_size]( Zip&    zip,
                                   size_t  start,
                                   size_t& num_written,
                                   size_t& bytes_written ) {
        Buffer uncompressed( max_size );
        for( size_t i = start; i < zip.size(); i += jobs ) {
            zip.extract_in( start, uncompressed );
            File( zip[i].name(), "wb" ).write( uncompressed );
            num_written++;
            bytes_written += zip[i].size();
        }

    };

    vector<size_t> counts( jobs, 0 );
    vector<size_t> bytes( jobs, 0 );

    vector<Zip> zips;
    for( size_t i = 0; i < jobs; ++i )
        zips.emplace_back( buf );

    /////////////////////////////////////////////////////////////

    for( size_t i = 0; i < jobs; ++i ) {
        unzip( zips[i], i, counts[i], bytes[i] );
        //cout << "    mtime:     "  << s.mtime()     << endl;
    }

    /////////////////////////////////////////////////////////////

    for( size_t i = 0; i < jobs; ++i ) {
        cout << "Thread "      << i+1 << ":" << endl;
        cout << "    files: "  << counts[i] << endl;
        cout << "    bytes: "  << bytes[i]  << endl;
        //cout << "    mtime:     "  << s.mtime()     << endl;
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
