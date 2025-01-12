#include <moo/core.hh>

#include <regex>
#include <filesystem>


namespace moo
{
    void strip( std::wstring & value )
    {
        // Temp storage.
        std::list<std::wstring> elements;


        // Split line into filepaths and strings.
        {
            std::wsmatch match;
            std::wregex  regex( L"[^\\s\']+|\'.+\'" );

            while( std::regex_search( value, match, regex ) )
            {
                elements.emplace_back( match[ 0 ].str( ) );

                value = std::regex_replace( value, regex, L"", std::regex_constants::format_first_only );
            }
        }


        // Remove spaces from the start and at the ending of the filepaths.
        {
            std::wsmatch match;
            std::wregex  regex( L"\'\\s*(.+\\S)\\s*\'" );

            for( auto & element : elements ) if( std::regex_match( element, match, regex ) )
            {
                element = match[ 1 ].str( );

                element.insert( element.cbegin( ), L'\'' );
                element.insert( element.cend( ),   L'\'' );
            }
        }


        // Assemble all back.
        value.clear( );

        for( auto & element : elements )
        {
            value.append( element );
            value.append( L" "    );
        }

        value.pop_back( );
    }


    void  path( std::wstring & value )
    {
        std::size_t index = 0;
        std::size_t  npos = std::wstring::npos;


        for( index = value.find( L"\'" ); index != npos; index = value.find( L"\'" ) )
        {
            value.replace( index, 1, L"\"" );
        }


    #ifdef _WIN32
        for( index = value.find( L"/"  ); index != npos; index = value.find( L"/"  ) )
        {
            value.replace( index, 1, L"\\" );
        }
    #endif
    }


    void  fold( std::wstring const & value )
    {
        std::wstring temp = value;

        if( temp[ 0 ] == L'\'' )
        {
            temp.erase( 0, 1 ).pop_back( );
        }

        std::filesystem::path file = temp;
        std::filesystem::path spot = file.parent_path( );

        std::wstring dir = spot.generic_wstring( );

        std::filesystem::create_directories( spot );
    }


    std::wstring convert_string( std::string  const & value )
    {
        return std::filesystem::path( value ).wstring( );
    }


    std::string  convert_string( std::wstring const & value )
    {
        return std::filesystem::path( value ).string( );
    }
}


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


        // Each recipe must have pattern.
        if( not out.vars.contains( L"pattern" ) )
        {
            throw L"Recipe must have pattern variable.";
        }


        // Each recipe must have command.
        if( not out.vars.contains( L"command" ) )
        {
            throw L"Recipe must have command variable.";
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