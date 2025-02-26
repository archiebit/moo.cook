#ifndef MOO_UTIL_HH
#define MOO_UTIL_HH

#include <string>


namespace moo
{
    void parse( std::string const & filename      );
    void build( std::string const & configuration );

    void correct_gaps( std::string & path ) noexcept;
    void correct_path( std::string & path ) noexcept;
}


#endif