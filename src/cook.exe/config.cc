#include <moo/config.hh>
#include <moo/core.hh>

#include <regex>
#include <filesystem>


namespace moo
{
    void config::build( )
    {
        std::list<std::wstring> target_list = create_target_list( );
        std::list<recipe>       recipe_list = create_recipe_list( );


        for( auto const & target : target_list )
        {
            build( target, recipe_list );
        }
    }


    std::wstring config::build( std::wstring const & target, std::list<recipe> & list )
    {
        // Find correct recipe and put its instance into the chain.
        std::wsmatch match;
        std::wregex  regex( L"([^\\*]*[\\/\\\\])*([^\\/\\\\\\.\\*]+|\\*)(\\.[^\\*]*)?$" );

        recipe & current = list.emplace_front( config::match( target, list ) );

        if( std::regex_match( target, match, regex ) )
        {
            std::wstring name = match[ 2 ].str( );
            std::wstring type = match[ 3 ].str( );

            // Fill recipe instance with target's data.
            current.vars[ L"filename" ] = name;
            current.vars[ L"filetype" ] = type;
            current.vars[ L"file"     ] = name + type;
        }


        // Resolve recipe's dependencies.
        for( auto const & [ name, data ] : current.cmds )
        {
            current.vars[ name ] = L"";

            for( auto & item : prepare( data, list ) )
            {
                current.vars[ name ] += build( item, list );
                current.vars[ name ] += L" ";
            }
        }


        // Execute command and process recipe's output.
        std::wstring execute;
        std::wstring  output;

        {
            output = substitute_full( current.vars[ L"output" ], list );

            moo::fold( output );
            moo::strip( output );
        }

        {
            execute += current.vars[ L"command" ];
            execute += L" ";
            execute += current.vars[ L"args"    ];

            execute  = substitute_full( execute, list );

            moo::strip( execute ), moo::path( execute );


            // Print command and execute it.
            std::wcout << execute << std::endl;

            std::system( convert_string( execute ).c_str( ) );
        }


        // Remove current recipe because its done.
        list.pop_front( );

        return output;
    }


    std::list<std::wstring> config::prepare( std::wstring const & value, std::list<recipe> & list )
    {
        std::list<std::wstring> output;
        std::wstring            target = substitute_full( value, list );


        // Format 'target' as regular expression.
        {
            auto replace = []( std::wstring & value, std::wstring const & part, std::wstring const & data )
            {
                std::size_t index = 0;
                std::size_t limit = std::wstring::npos;

                while( ( index = value.find( part, index ) ) != limit )
                {
                    value.replace( index, part.size( ), data );

                    index += data.size( );
                }
            };

            replace( target, L"/", L"[\\/\\\\]" );
            replace( target, L".", L"\\."       );
            replace( target, L"*", L"(.+)"      );
        }


        // Iterate through folders and find fitting files.
        {
            auto root = std::filesystem::current_path( ).generic_wstring( );
            auto dirs = std::filesystem::recursive_directory_iterator( root );

            for( auto const & entry : dirs ) if( entry.is_regular_file( ) )
            {
                std::wregex  regex( L"\\\\" );
                std::wstring range = entry.path( );



                range.replace( range.find( root ), root.size( ) + 1, L"" );

                range = std::regex_replace( range, regex, L"/" );

                if( std::regex_match( range, std::wregex( target ) ) )
                {
                    output.push_back( std::move( range ) );
                }
            }
        }


        return output;
    }
}


namespace moo
{
    std::wstring config::substitute_full( std::wstring const & value, std::list<recipe> const & list )
    {
        static std::wsmatch match;
        static std::array   regex
        {
            std::wregex( L"\\<([^\\s\\>]+)\\>" ), // Last element of the list.
            std::wregex( L"\\(([^\\s\\)]+)\\)" ), // Prev element.
            std::wregex( L"\\{([^\\s\\}]+)\\}" ), // Curr element.
            std::wregex( L".*"                 )  // Invalid option.
        };


        std::wstring total = value;
        std::size_t  index;
        std::array   start
        {
            std::next( list.cbegin( ), list.size( ) - 1 ),
            std::next( list.cbegin( ), 1 ),
            std::next( list.cbegin( ), 0 ),
            list.cend( )
        };

        for( ; ; )
        {
            for( index = 0; index < std::size( regex ); ++index ) if( std::regex_search( total, match, regex[ index ] ) )
            {
                break;
            }


            if( index == regex.size( ) - 1 )
            {
                break;
            }


            std::wstring name = match[ 1 ].str( );
            std::wstring data;

            for( auto curr = start[ index ]; curr != start.back( ); ++curr ) if( curr->vars.contains( name ) )
            {
                data = curr->vars.at( name );

                break;
            }


            total = std::regex_replace( total, regex[ index ], data, std::regex_constants::format_first_only );
        }


        return total;
    }

    std::wstring config::substitute_test( std::wstring const & value, std::list<recipe> const & list )
    {
        static std::wsmatch match;
        static std::array   regex
        {
            std::wregex( L"\\<([^\\s\\>]+)\\>" ), // Last element of the list.
            std::wregex( L"\\(([^\\s\\)]+)\\)" ), // Prev element.
            std::wregex( L"\\{([^\\s\\}]+)\\}" ), // Curr element (unavailable for testing).
            std::wregex( L".*"                 )  // Invalid option.
        };


        std::wstring total = value;
        std::size_t  index;
        std::array   start
        {
            std::next( list.cbegin( ), list.size( ) - 1 ),
            std::next( list.cbegin( ), 0 ),
            std::next( list.cbegin( ), 0 ),
            list.cend( )
        };

        for( ; ; )
        {
            for( index = 0; index < regex.size( ); ++index ) if( std::regex_search( total, match, regex[ index ] ) )
            {
                break;
            }


            if( index == regex.size( ) - 1 )
            {
                break;
            }

            if( index == 2 )
            {
                throw L"Prohibited use of local variables.";
            }


            std::wstring name = match[ 1 ].str( );
            std::wstring data;

            for( auto curr = start[ index ]; curr != start.back( ); ++curr ) if( curr->vars.contains( name ) )
            {
                data = curr->vars.at( name );

                break;
            }


            total = std::regex_replace( total, regex[ index ], data, std::regex_constants::format_first_only );
        }


        return total;
    }


    std::wstring config::match( std::wstring const & value, std::wstring const & pattern, std::list<recipe> const & list )
    {
        try
        {
            std::wstring target = substitute_full( value,   list );
            std::wstring source = substitute_test( pattern, list );

            std::wstring path[ 2 ];
            std::wstring name[ 2 ];
            std::wstring type[ 2 ];

            auto separate = []( std::wstring const & origin, std::wstring & path, std::wstring & name, std::wstring & type )
            {
                std::wsmatch match;
                std::wregex  regex( L"([^\\*]*[\\/\\\\])*([^\\/\\\\\\.\\*]+|\\*)(\\.[^\\*]*)?$" );

                if( std::regex_match( origin, match, regex ) )
                {
                    path = match[ 1 ].str( );
                    name = match[ 2 ].str( );
                    type = match[ 3 ].str( );
                }
                else throw L"Invalid path format!";
            };

            separate( target, path[ 0 ], name[ 0 ], type[ 0 ] ); moo::path( path[ 0 ] );
            separate( source, path[ 1 ], name[ 1 ], type[ 1 ] ); moo::path( path[ 1 ] );


            if( path[ 0 ] != path[ 1 ] )
            {
                return L"none";
            }

            if( name[ 0 ] == name[ 1 ] and type[ 0 ] == type[ 1 ] )
            {
                return L"precise";
            }

            if( type[ 0 ] == type[ 1 ] and ( name[ 0 ] == L"*" or name[ 1 ] == L"*" ) )
            {
                return L"typed";
            }

            if( ( type[ 0 ].empty( ) and name[ 0 ] == L"*" ) or ( type[ 1 ].empty( ) and name[ 1 ] == L"*" ) )
            {
                return L"anything";
            }

            return L"none";
        }
        catch( std::wstring::const_pointer )
        {
            return L"none";
        }
    }

    recipe &     config::match( std::wstring const & value, std::list<recipe> const & list )
    {
        recipe * precise  = nullptr;
        recipe * typed    = nullptr;
        recipe * anything = nullptr;

        for( auto & item : recipes )
        {
            std::wstring option = match( value, item.vars.at( L"pattern" ), list );

            if( not precise  and option == L"precise"  ) precise  = & item;
            if( not typed    and option == L"typed"    ) typed    = & item;
            if( not anything and option == L"anything" ) anything = & item;

            if( precise and typed and anything )
            {
                break;
            }
        }

        if( precise  ) return * precise;
        if( typed    ) return * typed;
        if( anything ) return * anything;

        throw L"Cannot find fitting recipe!";
    }
}


namespace moo
{
    std::list<std::wstring> config::create_target_list( )
    {
        std::list<std::wstring> range;
        std::wsmatch            match;
        std::wregex             regex( L"\'[^\']+\'|[^\'\\s]+" );


        std::wstring init = vars[ L"list" ];
        std::wstring item;

        while( std::regex_search( init, match, regex ) )
        {
            item = match[ 0 ].str( );

            init = std::regex_replace( init, regex, L"", std::regex_constants::format_first_only );


            if( item[ 0 ] == L'\'' )
            {
                item.erase( 0, 1 );
                item.erase( item.size( ) - 1, 1 );
            }

            strip( item );

            range.push_back( std::move( item ) );
        }


        return range;
    }


    std::list<recipe>       config::create_recipe_list( )
    {
        recipe proxy;

        proxy.vars = vars;


        return { std::move( proxy ) };
    }
}