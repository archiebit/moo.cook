#ifndef MOO_CORE_HH
#define MOO_CORE_HH

#include <iostream>

#include <moo/object.hh>
#include <moo/recipe.hh>
#include <moo/config.hh>


namespace moo
{
    std::list<config> parse( std::wistream & stream );
}


#endif