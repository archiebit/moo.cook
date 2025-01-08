#ifndef MOO_CONFIG_HH
#define MOO_CONFIG_HH

#include <moo/object.hh>
#include <moo/recipe.hh>


namespace moo
{
    // Config contains its recipes and will try build its targets.
    class config : public object
    {
    public:
       ~config( ) = default;
        config( ) = default;


    public:
        std::list<recipe> recipes;
    };
}


#endif