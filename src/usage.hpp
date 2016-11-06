/****************************************************************
 * Application-specific data describing the kinds of command line
 * arguments and flags that are expected/allowed/required.
 ***************************************************************/
namespace usage {

static char const* info =
    "p-unzip: multithreaded unzipper."   "\n"
    "Usage: p-unzip [-j N] file.zip"     "\n"
    ""                                   "\n"
    "    N - number of threads."         "\n";

// Options that do not take a value
static auto options_no_val   = { 'h', 'f' };
// Options that must take a value
static auto options_val      = { 'j' };

// Minimum number of positional arguments that any valid
// commandline must have.
static size_t min_positional = 1;

} // namespace usage
