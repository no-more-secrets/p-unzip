#include "options.hpp"
#include "utils.hpp"
#include "zip.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>

using namespace std;

// Positional args should always be referred to by these.
static size_t const ARG_FILE_NAME = 0;

int main_( options::positional positional,
           options::options    options )
{
    size_t jobs = 1;
    if( has_key( options, 'j' ) )
        jobs = atoi( options['j'].get().c_str() );

    ERR_IF( jobs < 1, "invalid number of jobs: " << jobs );

    string filename = positional[ARG_FILE_NAME];

    bool force = has_key( options, 'f' );
    Buffer::SP buf =
        make_shared<Buffer>( File( filename, "rb" ).read() );

    Zip z( buf );

    zip_uint64_t max_size = 0;
    for( size_t i = 0; i < z.size(); ++i )
        max_size = max( max_size, z[i].size() );

    LOG( "file:     " << filename );
    LOG( "jobs:     " << jobs     );
    LOG( "force:    " << force    );
    LOG( "entries:  " << z.size() );
    LOG( "max size: " << max_size );

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
        LOG( "Thread "      << i+1 << ":" );
        LOG( "    files: "  << counts[i]  );
        LOG( "    bytes: "  << bytes[i]   );
        //cout << "    mtime:     "  << s.mtime()     << endl;
    }

    return 0;
}
