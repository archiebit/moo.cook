#include <moo/core.hh>

#include <regex>


namespace moo
{
    static config parse_config( std::wistream & stream )
    {
        std::wstring range;
        std::wsmatch match;
        std::wregex  cases[]
        {
            // Variable declaration.
            std::wregex( L"(\\S+)\\s*=\\s*\"(.+)\"$" )
        };


        config out;

        while( not stream.eof( ) and stream.peek( ) != L'[' )
        {
            std::getline( stream, range );


            if( range.empty( ) )
            {
                continue;
            }


            if( std::regex_match( range, match, cases[ 0 ] ) )
            {
                std::wstring name = match[ 1 ].str( );
                std::wstring data = match[ 2 ].str( );


                if( out.vars.contains( name ) )
                {
                    throw L"Configuration contains double variable declaration!";
                }


                out.vars.emplace( std::move( name ), std::move( data ) );

                continue;
            }


            throw L"Unexpected syntax of the configuration element!";
        }


        // Each configuration must have its own name
        // to distinguish it from others.
        if( not out.vars.contains( L"name" ) )
        {
            throw L"Configuration must have its own name!";
        }


        // Each configuration must have its target list.
        if( not out.vars.contains( L"list" ) )
        {
            throw L"Configuration must have its target list.";
        }


        return out;
    }


    static recipe parse_recipe( std::wistream & stream )
    {
        std::wstring range;
        std::wsmatch match;
        std::wregex  cases[]
        {
            // Variable declaration.
            std::wregex( L"(\\S+)\\s*=\\s*\"(.+)\"$" ),

            // Preparation declaration.
            std::wregex( L"(\\S+)\\s*=\\s*prepare\\s*\"(.+)\"" )
        };


        recipe out;

        while( not stream.eof( ) and stream.peek( ) != L'[' )
        {
            std::getline( stream, range );


            if( range.empty( ) )
            {
                continue;
            }


            // Variable declaration.
            if( std::regex_match( range, match, cases[ 0 ] ) )
            {
                std::wstring name = match[ 1 ].str( );
                std::wstring data = match[ 2 ].str( );


                if( out.vars.contains( name ) )
                {
                    throw L"Recipe contains double variable declaration!";
                }


                out.vars.emplace( std::move( name ), std::move( data ) );

                continue;
            }


            // Preparation declaration.
            if( std::regex_match( range, match, cases[ 1 ] ) )
            {
                std::wstring name = match[ 1 ].str( );
                std::wstring data = match[ 2 ].str( );


                if( out.cmds.contains( name ) )
                {
                    throw L"Recipe contains double preparation declaration!";
                }


                out.cmds.emplace( std::move( name ), std::move( data ) );

                continue;
            }


            throw L"Unexpected syntax in the recipe element!";
        }


        return out;
    }
}


namespace moo
{
    std::list<config> parse( std::wistream & stream )
    {
        std::wstring range;
        std::wsmatch match;
        std::wregex  cases[]
        {
            // Configuration declaration.
            std::wregex( L"\\[\\s*configuration\\s*\\]$" ),

            // Recipe declaration.
            std::wregex( L"\\[\\s*recipe\\s*\\]$"        )
        };


        std::list<config> out;

        while( not stream.eof( ) )
        {
            std::getline( stream, range );


            if( range.empty( ) )
            {
                continue;
            }


            // Configuration declaration.
            if( std::regex_match( range, match, cases[ 0 ] ) )
            {
                out.emplace_back( parse_config( stream ) );

                continue;
            }


            // Recipe declaration.
            if( std::regex_match( range, match, cases[ 1 ] ) )
            {
                recipe element = parse_recipe( stream );


                if( out.empty( ) )
                {
                    throw L"Configuration must be presented before its recipe!";
                }


                out.back( ).recipes.push_back( std::move( element ) );

                continue;
            }


            throw L"Unexpected syntax in the file!";
        }


        return out;
    }
}