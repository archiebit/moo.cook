#ifndef MOO_CORE_HH
#define MOO_CORE_HH

#include <iostream>

#include <moo/object.hh>
#include <moo/recipe.hh>
#include <moo/config.hh>


namespace moo
{
    std::list<config> parse( std::wistream & stream );

    
    void strip( std::wstring & value );       // Remove unnecessary spaces.
    void  path( std::wstring & value );       // Convert string to path format.
    void  fold( std::wstring const & value ); // Create parent folder of the file.


    std::wstring convert_string( std::string  const & value );
    std::string  convert_string( std::wstring const & value );
}


#endif