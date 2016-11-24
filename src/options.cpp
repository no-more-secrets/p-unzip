/****************************************************************
* Command line options processing
****************************************************************/
#include "options.hpp"

#include <stdexcept>
#include <iostream>

using namespace std;

namespace options {

/****************************************************************
* Arg: container for holder a parameter a labeling it as either
* an option or non-option (option means starts with a dash).
* These objects are immutable.
****************************************************************/
class Arg {

public:
    enum class Type { NORMAL, OPTION };

    Arg( char const* arg ) {
        string s( arg );
        if( s.size() > 1 && starts_with( s, '-' ) ) {
            m_type      = Type::OPTION;
            m_option    = s[1];
            m_value     = string( s.begin()+2, s.end() );
            m_option_has_value = (s.size() > 2 );
        } else {
            m_type      = Type::NORMAL;
            m_value     = s;
        }
    }

    /* Now we have one getter for each member, but with
     * some safety checks because not all members should
     * be gettable depending on the value of m_type. */

    Type type() const { return m_type; }

    bool option_has_value() const {
        // We should not be able to ask this question
        // if this is not an option parameter.
        FAIL_( m_type != Type::OPTION );
        return m_option_has_value;
    }

    char option() const {
        // We can not get the name of the option if this
        // is not an option.
        FAIL_( m_type != Type::OPTION );
        return m_option;
    }

    string const& value() const {
        // We cannot get a value if this is an option
        // parameter but does not have a value attached.
        FAIL_( m_type == Type::OPTION && !m_option_has_value );
        return m_value;
    }

private:
    // Not all of these parameters are relevant in all cases.
    // It depends on the value of m_type.
    Type     m_type;
    bool     m_option_has_value;
    char     m_option;
    string   m_value;

};

/****************************************************************
* Options parser.  Implemented using recursion.
****************************************************************/
bool parse_impl( set<char> const&            options,
                 set<char> const&            with_value,
                 vector<Arg>::const_iterator start,
                 vector<Arg>::const_iterator end,
                 opt_result&                 res ) {

    // If arg list is empty then that's always valid
    if( start == end )
        return true;

    Arg const& arg = *start;

    // Is this the start of a non-option (i.e., positional)
    // argument.  If so then add it to the list and recurse on
    // the remainder.
    if( arg.type() == Arg::Type::NORMAL )
        res.first.push_back( arg.value() );
    else if( has_key( options, arg.option() ) ) {
        // This is a valid option.
        Optional<string> op_val;
        // Is this option one that needs a value?
        if( has_key( with_value, arg.option() ) ) {
            string value;
            // This option needs a value; is it attached to
            // this parameter, or a separate argument?
            if( arg.option_has_value() )
                // It is attached to this parameter, so extract
                // it and continue.
                value = arg.value();
            else {
                // Value must be next parameter, so therefore
                // we must have a next non-option parameter.
                start++;
                if( start == end ||
                    start->type() == Arg::Type::OPTION ) {
                    cerr << "error: option '" << arg.option()
                         << "' must take a value." << endl;
                    return false;
                }
                value = start->value();
            }
            op_val = Optional<string>( value );
        }
        else if( arg.option_has_value() ) {
            cerr << "error: option '" << arg.option()
                 << "' does not take values." << endl;
            return false;
        }
        // Looks good, so store the option.  Depending on
        // what happened above, it may or may not have a
        // value inside op_val.
        res.second[arg.option()] = op_val;
    }
    else {
        // This is an option but not a valid one.
        cerr << "error: option '" << arg.option()
             << "' is not recognized." << endl;
        return false;
    }
    return parse_impl( options, with_value, start+1, end, res );
}

/****************************************************************
* Driver for options parsing.  This is what you should call from
* main.
****************************************************************/
bool parse( int              argc,
            char**           argv,
            set<char> const& options_all,
            set<char> const& options_with_val,
            opt_result&      res ) {

    vector<Arg> args;
    for( int i = 1; i < argc; ++i )
        args.push_back( Arg( argv[i] ) );
    FAIL_( args.size() != size_t(argc-1) );
    return parse_impl( options_all,
                       options_with_val,
                       args.begin(),
                       args.end(),
                       res );
}

} // namespace options
