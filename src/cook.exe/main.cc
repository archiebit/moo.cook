#include <moo/core.hh>

#include <fstream>


int main( int argc, char * argv[] )
{
    try
    {
        std::wifstream stream( L"recipe.ini" );

        if( not stream.is_open( ) )
        {
            throw L"Cannot find or open \'recipe.ini\' file!";
        }

        if( argc <= 1 )
        {
            throw L"Configuration was not provided!";
        }


        for( auto & config : moo::parse( stream ) )
        {
            for( int i = 1; i < argc; ++i ) if( config.vars[ L"name" ] == moo::convert_string( argv[ i ] ) )
            {
                config.build( );

                break;
            }
        }

        return 0;
    }
    catch( std::wstring::const_pointer message )
    {
        std::wcerr << message << std::endl;

        return 1;
    }
}