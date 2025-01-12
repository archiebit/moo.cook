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
        // Build all targets.
        void build( );


    private:
        // Build specific target.
        std::wstring build( std::wstring const & target, std::list<recipe> & list );


        // Splits 'list' variable into multiple strings.
        std::list<std::wstring> create_target_list( );

        // Create initial recipe chain with config in the root.
        std::list<recipe>       create_recipe_list( );


    private:
        // Substitutes variable with data from the chain.
        std::wstring substitute_full( std::wstring const & value, std::list<recipe> const & list );
        std::wstring substitute_test( std::wstring const & value, std::list<recipe> const & list );

        // Finds fitting recipe and filepath.
        std::wstring match( std::wstring const & value, std::wstring const & pattern, std::list<recipe> const & list );
        recipe &     match( std::wstring const & value, std::list<recipe> const & list );

        // Special treatment for a 'prepare' command of the recipe.
        std::list<std::wstring> prepare( std::wstring const & value, std::list<recipe> & list );


    public:
        std::list<recipe> recipes;
    };
}


#endif