#include <moo/util.hh>

#include <iostream>


int main( int argc, char * argv[] )
{
    try
    {
        std::string file = "recipe.ini";
        std::string name;

        if( argc == 3 )
        {
            file = argv[ 1 ];
            name = argv[ 2 ];
        }

        if( argc == 2 )
        {
            name = argv[ 1 ];
        }

        moo::parse( file );
        moo::build( name );

        return 0;
    }
    catch( std::runtime_error const & error )
    {
        std::cerr << error.what( ) << std::endl;

        return 1;
    }
}