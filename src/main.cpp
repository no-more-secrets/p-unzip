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
    cerr << "Usage: p-unzip [-j N] file.zip" << endl;
    cerr << endl;
    cerr << "    where `N` is number of threads." << endl;
}

int main( int argc, char* argv[] )
{
    try {

    options::OptResult opt_result;
    set<char> opts{ 'j' };
    bool parsed = options::parse( argc-1, argv+1, opts, opt_result );

    auto& positional = opt_result.second;
    auto& options    = opt_result.first;

    if( !parsed || positional.size() != 1 ) {
        usage();
        return 1;
    }

    int jobs = 1;
    if( options.count( 'j' ) ) {
        jobs = atoi( options['j'].get().c_str() );
        if( jobs <= 0 )
            throw runtime_error( "invalid number of jobs" );
    }

    string filename = positional[0];
    log( "File: " + filename );

    ostringstream ss;
    ss << jobs;
    log( "Jobs: " + ss.str() );

    File f( filename, "rb" );

    Buffer::SP buf = make_shared<Buffer>( f.read() );

    Zip z( buf );
    //Zip z2( buf );
    //Zip z3( buf );

    auto count = zip_get_num_entries( z.get(), ZIP_FL_UNCHANGED );
    cout << "Found " << count << " entries:" << endl;

    for( auto i = 0; i < count; ++i ) {

        zip_stat_t s;
        if( zip_stat_index( z.get(), i, ZIP_FL_UNCHANGED, &s ) )
            throw runtime_error( "failed to stat file" );

        if( s.valid & ZIP_STAT_INDEX     ) cout << "  index: "       << s.index     << endl;
        if( s.valid & ZIP_STAT_NAME      ) cout << "    name:      " << s.name      << endl;
        if( s.valid & ZIP_STAT_SIZE      ) cout << "    size:      " << s.size      << endl;
        if( s.valid & ZIP_STAT_COMP_SIZE ) cout << "    comp_size: " << s.comp_size << endl;
        if( s.valid & ZIP_STAT_MTIME     ) cout << "    mtime:     " << s.mtime     << endl;
        if( s.valid & ZIP_STAT_FLAGS     ) cout << "    flags:     " << s.flags     << endl;
    }

    } catch( exception const& e ) {
        cerr << "exception: " << e.what() << endl;
        return 1;
    }
}
