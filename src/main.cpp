#include "options.hpp"
#include "utils.hpp"
#include "zip.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <array>
#include <thread>

using namespace std;

// Positional args should always be referred to by these.
size_t const ARG_FILE_NAME = 0;

size_t const MAX_JOBS = 8;

int main_( options::positional positional,
           options::options    options )
{
    size_t jobs = has_key( options, 'j' )
                ? atoi( options['j'].get().c_str() )
                : 1;

    FAIL( jobs < 1, "invalid number of jobs: " << jobs );
    FAIL( jobs > MAX_JOBS, "max allowed jobs is " << MAX_JOBS );

    string filename = positional[ARG_FILE_NAME];

    Buffer::SP zip_buffer =
        make_shared<Buffer>( File( filename, "rb" ).read() );

    Zip z( zip_buffer );

    zip_uint64_t max_size = 0;
    for( size_t i = 0; i < z.size(); ++i )
        max_size = max( max_size, z[i].size() );

    LOG( "file:       " << filename  );
    LOG( "jobs:       " << jobs      );
    LOG( "entries:    " << z.size()  );
    LOG( "max size:   " << max_size  );

    auto unzip = [&]( size_t                thread_idx,
                      vector<size_t> const& idxs,
                      size_t&               files,
                      size_t&               bytes,
                      bool&                 ret ) -> void {

        // We are guilty until proven innocent.
        ret = false; // return value: fail
        try {

            // Create the zip here because we don't know if
            // libzip or zlib are thread safe.  All that the
            // Zip creation will do here is to change the ref
            // count on the buffer which is thread safe since
            // it's a shared_ptr.
            Zip zip( zip_buffer );
            // Allocate a new buffer for use only within this
            // thread that is large enough to hold any single
            // uncompressed file in the archive.
            Buffer uncompressed( max_size );

            for( auto idx : idxs ) {
                LOG( "[" << thread_idx <<
                     "] uncompressing " << zip[idx].name() );
                zip.extract_in( idx, uncompressed );
                // Enclose in a scope to make sure the file
                // gets close immediately after.
                {
                    File( zip[idx].name(), "wb" )
                        .write( uncompressed, zip[idx].size() );
                }
                bytes += zip[idx].size();
                files++;
            }

            ret = true; // return value: success

        } catch( exception const& e ) {
            LOG( e.what() );
        } catch( ... ) {
            LOG( "unknown error" );
        }
    };

    // Here we need to distribute the non-folder elements
    // evenly among the jobs.
    size_t size_non_folders = 0;
    vector<vector<size_t>> thread_idxs( jobs );
    for( auto const& zs : z ) {
        if( !zs.is_folder() ) {
            auto slot = size_non_folders++ % jobs;
            thread_idxs[slot].push_back( zs.index() );
        }
    }

    vector<size_t> files( jobs, 0 );
    vector<size_t> bytes( jobs, 0 );

    array<bool, MAX_JOBS> results;
    static_assert( sizeof( results ) == sizeof( bool )*MAX_JOBS,
        "size of results array is not expected" );
    results.fill( false );

    vector<thread> threads( jobs );

    /************************************************************
     * Start multithreading
     ************************************************************/

    for( size_t i = 0; i < jobs; ++i )
        threads[i] = thread( unzip, i,
                                    std::ref( thread_idxs[i] ),
                                    std::ref( files[i] ),
                                    std::ref( bytes[i] ),
                                    std::ref( results[i] ) );

    for( auto& t : threads )
        t.join();

    /************************************************************
     * End multithreading
     ************************************************************/

    for( size_t i = 0; i < jobs; ++i ) {
        FAIL_( results[i] == false );
        LOG( "Thread "      << i+1 << ":" );
        LOG( "    files: "  << files[i]   );
        LOG( "    bytes: "  << bytes[i]   );
        //cout << "    mtime:     "  << s.mtime()     << endl;
    }

    // Make sure that the sum of files counts for each thread
    // equals the total number of files in the zip.
    auto files_written = std::accumulate(
        files.begin(), files.end() , size_t( 0 ) );
    FAIL_( files_written != size_non_folders );

    LOG( "==============================================" );
    LOG( "Total bytes written: " << std::accumulate(
        bytes.begin(), bytes.end() , size_t( 0 ) ) );

    return 0;
}
