#include <moo/recipe.hh>
#include <moo/config.hh>
#include <moo/util.hh>

#include <regex>
#include <sstream>
#include <exception>
#include <filesystem>
#include <iostream>


namespace moo
{
    // Constructors and destructor.
    // -------------------------------------------------------------------------
    recipe::~recipe( )
    { }


    recipe:: recipe( std::istream & source, std::size_t & line )
    :   parent( nullptr )
    ,   origin( nullptr )
    {
        parse( source, line );
    }




    // Setters.
    // -------------------------------------------------------------------------
    void recipe::bind_parent( recipe const * parent ) noexcept
    {
        this->parent = parent;
    }


    void recipe::drop_parent( )                       noexcept
    {
        this->parent = nullptr;
    }


    void recipe::bind_origin( config const * origin ) noexcept
    {
        this->origin = origin;
    }


    void recipe::drop_origin( )                       noexcept
    {
        this->origin = nullptr;
    }




    // Parser.
    // -------------------------------------------------------------------------
    void recipe::parse( std::istream & source, std::size_t & line )
    {
        static std::regex const     comment( R"...([#;].*)..."                           );
        static std::regex const    variable( R"...(\s*(.*\S)\s*=\s*"(.+)")..."           );
        static std::regex const preparation( R"...(\s*(.*\S)\s*=\s*prepare\s*"(.+)")..." );


        std::stringstream error;
        std::string       piece;
        std::smatch       match;

        while( source.peek( ) != EOF and source.peek( ) != '[' )
        {
            std::getline( source, piece ), ++line;


            if( piece.empty( ) )
            {
                continue;
            }


            if( std::regex_match( piece, comment ) )
            {
                continue;
            }


            if( std::regex_match( piece, match, variable ) )
            {
                std::string name = match[ 1 ].str( );
                std::string data = match[ 2 ].str( );


                if( variables.contains( name ) or preparations.contains( name ) )
                {
                    error << "Variable duplication!\n";
                    error << line << ": \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                variables.emplace( std::move( name ), std::move( data ) );

                continue;
            }


            if( std::regex_match( piece, match, preparation ) )
            {
                std::string name = match[ 1 ].str( );
                std::string data = match[ 2 ].str( );


                if( variables.contains( name ) or preparations.contains( name ) )
                {
                    error << "Variable duplication!\n";
                    error << line << ": \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                preparations.emplace( std::move( name ), std::move( data ) );

                continue;
            }


            // Invalid recipe contents.
            {
                error << "Invalid recipe content!\n";
                error << line << ": \'" << piece << "\'";

                throw std::runtime_error( error.str( ) );
            }
        }
    }




    // Substitution.
    // -------------------------------------------------------------------------
    std::string recipe::subst( std::string const & string )                                               const
    {
        static std::regex const root( R"__(\<([^\>]+)\>)__" );
        static std::regex const prev( R"__(\(([^\)]+)\))__" );
        static std::regex const curr( R"__(\{([^\}]+)\})__" );

        std::stringstream error;
        std::string       value = string;
        std::string       piece;
        std::smatch       match;


        while( std::regex_search( value, match, curr ) )
        {
            piece = match[ 1 ].str( );

            if( variables.contains( piece ) )
            {
                value = std::regex_replace( value, curr, variables.at( piece ), std::regex_constants::format_first_only );
            }
            else
            {
                piece.insert( piece.cbegin( ), 1, '(' );
                piece.insert( piece.cend( ),   1, ')' );

                value = std::regex_replace( value, curr, piece, std::regex_constants::format_first_only );
            }
        }


        while( std::regex_search( value, match, prev ) )
        {
            piece = match[ 1 ].str( );

            piece.insert( piece.cbegin( ), 1, '{' );
            piece.insert( piece.cend( ),   1, '}' );

            value = std::regex_replace( value, prev, piece, std::regex_constants::format_first_only );
        }


        if( parent ) parent->subst( value );
        if( origin ) origin->subst( value );

        return value;
    }


    std::string recipe::subst( std::string const & string, recipe const * parent, config const * origin ) const
    {
        static std::regex const root( R"__(\<([^\>]+)\>)__" );
        static std::regex const prev( R"__(\(([^\)]+)\))__" );
        static std::regex const curr( R"__(\{([^\}]+)\})__" );

        std::stringstream error;
        std::string       value = string;
        std::string       piece;
        std::smatch       match;


        while( std::regex_search( value, match, curr ) )
        {
            piece = match[ 1 ].str( );

            if( variables.contains( piece ) )
            {
                value = std::regex_replace( value, curr, variables.at( piece ), std::regex_constants::format_first_only );
            }
            else
            {
                piece.insert( piece.cbegin( ), 1, '(' );
                piece.insert( piece.cend( ),   1, ')' );

                value = std::regex_replace( value, curr, piece, std::regex_constants::format_first_only );
            }
        }


        while( std::regex_search( value, match, prev ) )
        {
            piece = match[ 1 ].str( );

            piece.insert( piece.cbegin( ), 1, '{' );
            piece.insert( piece.cend( ),   1, '}' );

            value = std::regex_replace( value, prev, piece, std::regex_constants::format_first_only );
        }


        if( parent ) parent->subst( value );
        if( origin ) origin->subst( value );

        return value;
    }


    void        recipe::subst( std::string & string )                                               const
    {
        static std::regex const root( R"__(\<([^\>]+)\>)__" );
        static std::regex const prev( R"__(\(([^\)]+)\))__" );
        static std::regex const curr( R"__(\{([^\}]+)\})__" );

        std::stringstream error;
        std::string       piece;
        std::smatch       match;


        while( std::regex_search( string, match, curr ) )
        {
            piece = match[ 1 ].str( );

            if( variables.contains( piece ) )
            {
                string = std::regex_replace( string, curr, variables.at( piece ), std::regex_constants::format_first_only );
            }
            else
            {
                piece.insert( piece.cbegin( ), 1, '(' );
                piece.insert( piece.cend( ),   1, ')' );

                string = std::regex_replace( string, curr, piece, std::regex_constants::format_first_only );
            }
        }


        while( std::regex_search( string, match, prev ) )
        {
            piece = match[ 1 ].str( );

            piece.insert( piece.cbegin( ), 1, '{' );
            piece.insert( piece.cend( ),   1, '}' );

            string = std::regex_replace( string, prev, piece, std::regex_constants::format_first_only );
        }


        if( parent ) parent->subst( string );
    //  if( origin ) origin->subst( string );
    }


    void        recipe::subst( std::string & string, recipe const * parent, config const * origin ) const
    {
        static std::regex const root( R"__(\<([^\>]+)\>)__" );
        static std::regex const prev( R"__(\(([^\)]+)\))__" );
        static std::regex const curr( R"__(\{([^\}]+)\})__" );

        std::stringstream error;
        std::string       piece;
        std::smatch       match;


        while( std::regex_search( string, match, curr ) )
        {
            piece = match[ 1 ].str( );

            if( variables.contains( piece ) )
            {
                string = std::regex_replace( string, curr, variables.at( piece ), std::regex_constants::format_first_only );
            }
            else
            {
                piece.insert( piece.cbegin( ), 1, '(' );
                piece.insert( piece.cend( ),   1, ')' );

                string = std::regex_replace( string, curr, piece, std::regex_constants::format_first_only );
            }
        }


        while( std::regex_search( string, match, prev ) )
        {
            piece = match[ 1 ].str( );

            piece.insert( piece.cbegin( ), 1, '{' );
            piece.insert( piece.cend( ),   1, '}' );

            string = std::regex_replace( string, prev, piece, std::regex_constants::format_first_only );
        }


        if( parent ) parent->subst( string );
        if( origin ) origin->subst( string );
    }




    // Building.
    // -------------------------------------------------------------------------
    std::string            recipe::match( std::string const & target, recipe const * parent, config const * origin ) const
    {
        static std::regex const    pathed( R"('(.*\S)')" );
        static std::regex const separator( R"([\\\/])"   );
        static std::regex const       dot( R"(\.)"       );
        static std::regex const  anything( R"(\*)"       );


        if( not variables.contains( "pattern" ) )
        {
            return { };
        }


        std::string   value( subst( variables.at( "pattern" ), parent, origin ) );
        std::string pattern( value  );

        if( std::regex_match( target, pathed ) )
        {
            pattern.insert( pattern.cbegin( ), 1, '\'' );
            pattern.insert( pattern.cend( ),   1, '\'' );
        }


        pattern = std::regex_replace( pattern, separator, R"(\/)" );
        pattern = std::regex_replace( pattern, dot,       R"(\.)" );
        pattern = std::regex_replace( pattern, anything,  R"(.+)" );

        if( std::regex_match( target, std::regex( pattern ) ) )
        {
            return value;
        }
        else
        {
            return { };
        }
    }


    std::string            recipe::place( std::string const & target )                                               const
    {
        static std::regex const closed( R"('(.*[\\\/])?([^\.]*)?(\..*)?')" );
        static std::regex const opened( R"((.*[\\\/])?([^\.]*)?(\..*)?)"   );

        std::string piece;
        std::smatch match;


        if( std::regex_match( target, match, closed ) )
        {
            piece = match[ 1 ].str( );

            if( not piece.empty( ) )
            {
                // Remove trailing separator.
                piece.pop_back( );

                std::filesystem::create_directories( piece );
            }
        }


        if( std::regex_match( target, match, opened ) )
        {
            piece = match[ 1 ].str( );

            if( not piece.empty( ) )
            {
                std::filesystem::create_directories( piece );
            }
        }


        return piece;
    }


    std::string            recipe::build( std::string const & target )
    {
        static std::regex const closed( R"('(.*[\\\/])?([^\.]*)?(\..*)?')" );
        static std::regex const opened( R"((.*[\\\/])?([^\.]*)?(\..*)?)"   );
        static std::smatch       match;


        // Create target specific variables.
        if( std::regex_match( target, match, closed ) or std::regex_match( target, match, opened ) )
        {
            std::string name = match[ 2 ].str( );
            std::string type = match[ 3 ].str( );


            variables.emplace( "filename", name );
            variables.emplace( "filetype", type );
            variables.emplace( "file",     name + type );
        }
        else
        {
            variables.emplace( "filename", "" );
            variables.emplace( "filetype", "" );
            variables.emplace( "file",     "" );
        }


        // Make all preparations.
        for( auto & [ name, data ] : preparations )
        {
            std::string & output = variables[ name ];

            for( auto & file : ready( data ) )
            {
                output += origin->build( file, this );
                output += " ";
            }

            // Remove last space character.
            output.pop_back( );
        }


        // Execute command.
        std::string command;
        std::string  output;


        if( variables.contains( "output" )  )
        {
            output  = subst( const_cast<std::string const &>( variables.at( "output" )  ) );
        }


        if( variables.contains( "command" ) )
        {
            command = subst( const_cast<std::string const &>( variables.at( "command" ) ) );

            correct_gaps( command );
            correct_path( command );

            std::cout << command << std::endl;

            place( output ), std::system( command.c_str( ) );
        }


        return output;
    }


    std::list<std::string> recipe::ready( std::string const & target )
    {
        static std::regex  const separator( R"([\\\/])" );
        static std::regex  const       dot( R"(\.)"     );
        static std::regex  const  anything( R"(\*)"     );


        std::list<std::string> paths;
        std::string            piece = subst( target );


        piece = std::regex_replace( piece, separator, R"(\/)" );
        piece = std::regex_replace( piece, dot,       R"(\.)" );
        piece = std::regex_replace( piece, anything,  R"(.+)" );

        for( auto & entry : std::filesystem::recursive_directory_iterator( std::filesystem::current_path( ) ) )
        {
            if( not entry.is_regular_file( ) )
            {
                continue;
            }


            std::string root = std::filesystem::current_path( ).generic_string( );
            std::string file = entry.path( ).generic_string( );


            root = std::regex_replace( root, separator, R"(/)" );
            file = std::regex_replace( file, separator, R"(/)" );

            file.erase( file.find( root ), root.size( ) );

            if( file.size( ) != 0 )
            {
                std::string_view view( "\\/" );

                if( view.contains( file.front( ) ) )
                {
                    file.erase( 0, 1 );
                }
            }


            if( std::regex_match( file, std::regex( piece ) ) )
            {
                file.insert( file.cbegin( ), 1, '\'' );
                file.insert( file.cend( ),   1, '\'' );

                paths.emplace_back( std::move( file ) );
            }
        }


        return paths;
    }
}