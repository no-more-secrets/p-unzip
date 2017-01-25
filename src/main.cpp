/****************************************************************
* p-unzip: the multithreaded unzipper
*
* author: David P. Sicilia
*
* This program will take a  zip  file  as  input  and a number of
* threads and it will distribute  the  files in the archive among
* the threads in the specified way  in order to take advantage of
* the opportunity for  parallelism  while  unzipping  an archive.
****************************************************************/
#include "options.hpp"
#include "unzip.hpp"

#include <cstdlib>
#include <thread>

using namespace std;
using options::option_get;

/****************************************************************
* Entrypoint into program from the arg parsing framework. This is
* called by a wrapper that handles parsing commandline parameters
* which are  then  delivered,  already  syntax  checked,  as data
* structures.
****************************************************************/
int main_( options::positional positional,
           options::options    options )
{
    /************************************************************
    * Get miscellaneous options
    ************************************************************/
    bool q    = has_key( options, 'q' ); // quiet
    bool exts = has_key( options, 'a' ); // no long extensions
    bool g    = has_key( options, 'g' ); // diagnostic info

    /************************************************************
    * Determine timestamp (TS) policy
    *************************************************************
    * The `t` option allows  the  user  to control how timestamps
    * are set on the extracted files. If the timestamps are of no
    * concern then it might be  advantageous to specify "current"
    * which will not set them  at  all  --  it will leave them to
    * take on  the  time  that  they  are  created/written, which
    * avoids an extra hit to  the  file  system  to change it. If
    * timestamps are important then you might just leave out this
    * option and it will default  to  using the timestamps stored
    * in the zip file. However, note  that zip files do not store
    * timezone, so there will be certain discrepancies associated
    * with that (local timezone is assumed when extracting). Fur-
    * thermore, even correctly  reproducing  the  timestamp  as a
    * local time (by  this  program)  requires  the  C runtime to
    * handle daylight savings time properly (due to the implemen-
    * tation of libzip),  which  may  or  may  not work properly.
    * Lastly, if all the  files  are  to  have the same timestamp
    * then we can simply supply an  integer value for the t argu-
    * ment. This is interpreted as  a  Linux  epoch time, and all
    * files will be set to that timestamp. */

    // By default we just use the "id" function that will use the
    // exact timestamp stored in the zip.
    auto id_transform = []( time_t t ){ return t; };
    TSXFormer ts_xform( id_transform );
    if( has_key( options, 't' ) ) {
        string t = options['t'].get();
        if( t == "current" )
            // Just let the timestamps fall where they may.
            ts_xform = []( time_t ){ return 0; };
        else {
            // All extracted files  should  have  this timestamp.
            time_t fixedStamp = to_uint<time_t>( t );
            FAIL( fixedStamp == 0, "invalid integer for t arg" );
            ts_xform = [=]( time_t ){ return fixedStamp; };
        }
    }

    /************************************************************
    * Get optional output folder prefix
    ************************************************************/
    // The user has the opportunity to specify a folder path  for
    // extraction. Whatever they specify will simply be prepended
    // to each path in the zip archive before extraction.
    string o( option_get( options, 'o', "" ) );

    /************************************************************
    * Determine the number of jobs to use
    ************************************************************/
    // First initialize the number of  jobs to its default value.
    string jobs( option_get( options, 'j', "1" ) );
    size_t j;
    // Next, let's get the  number  of  threads  that the machine
    // naturally supports (this will  include hyperthreads). Even
    // if we don't need this, we might want to log it at the end.
    auto num_threads = thread::hardware_concurrency();
    // The user can override  number  of  jobs  by specifying -j.
    if( jobs == "max" )
        // Assume that num_threads includes  the hyperthreads, so
        // in that case we probably don't  want to go above that.
        j = num_threads;
    else if( jobs == "auto" )
        // Assume we have hyper-threading; use  all of the "true"
        // threads and then half of the hyperthreads.
        j = size_t( num_threads*0.75 + 0.5 );
    else
        // Otherwise, must be a positive integer.
        j = to_uint<size_t>( jobs );

    // No matter how we got number  of  jobs, check it once more.
    FAIL( j < 1, "invalid number of jobs: " << j );

    /************************************************************
    * Determine the chunk size
    *************************************************************
    * When extracting a file from the zip archive, the chunk size
    * is the number of bytes that are decompressed and written to
    * the output file at  a  time.  With  zip  files that contain
    * large files together with multithreaded execution it is de-
    * sireable to limit the chunk size to save memory. */
    auto chunk( to_uint<size_t>(
        option_get( options, 'c', DEFAULT_CHUNK_S ) ) );

    /************************************************************
    * Distribution of files to the threads
    *************************************************************
    * See if the user has  specified  a distribution strategy. */
    string strat( option_get( options, 'd', DEFAULT_DIST ) );

    /************************************************************
    * Get zip file name
    *************************************************************
    * The zip file name will be  one of the positional arguments.
    * Note that the option parsing  mechanism should already have
    * validated that there is exactly one positional argument. */
    string f = positional[0];

    /************************************************************
    * Unzip
    *************************************************************
    * Do the unzip, and, if the  user has requested so, print di-
    * agnostic info to stderr. */
    auto info = p_unzip( f, j, q, o, strat, chunk, ts_xform, exts );
    if( g ) cerr << info;

    return 0;
}
