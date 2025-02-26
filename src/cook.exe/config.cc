#include <moo/config.hh>
#include <moo/recipe.hh>
#include <moo/util.hh>

#include <regex>
#include <sstream>
#include <exception>


namespace moo
{
    // Constructors and destructor.
    // -------------------------------------------------------------------------
    config::~config( )
    { }


    config:: config( std::istream & source, std::size_t & line )
    {
        parse( source, line );
    }




    // Small utility methods.
    // -------------------------------------------------------------------------
    void config::append( std::istream & source, std::size_t & line )
    {
        recipes.emplace_back( new recipe( source, line ) );
    }


    bool config::naming( std::string const & name )
    {
        if( variables.contains( "name" ) )
        {
            return variables.at( "name" ) == name;
        }

        return false;
    }




    // Parser.
    // -------------------------------------------------------------------------
    void config::parse( std::istream & source, std::size_t & line )
    {
        static std::regex const  comment( R"...([#;].*)..."                 );
        static std::regex const variable( R"...(\s*(.*\S)\s*=\s*"(.*)")..." );


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


                if( variables.contains( name ) )
                {
                    error << "Variable duplication!\n";
                    error << line << ": \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                variables.emplace( std::move( name ), std::move( data ) );

                continue;
            }


            // Invalid recipe contents.
            {
                error << "Invalid configuration content!\n";
                error << line << ": \'" << piece << "\'";

                throw std::runtime_error( error.str( ) );
            }
        }
    }




    // Substitution.
    // -------------------------------------------------------------------------
    std::string config::subst( std::string const & string ) const
    {
        static std::regex const root( R"(\<([^\>]+)\>)" );
        static std::regex const prev( R"(\(([^\)]+)\))" );
        static std::regex const curr( R"(\{([^\}]+)\})" );


        std::stringstream error;
        std::string       value = string;
        std::string       piece;
        std::smatch       match;
        std::size_t       count;

        // Substitute all variables.
        do
        {
            count = 0;

            if( std::regex_search( value, match, curr ) )
            {
                piece = match[ 1 ].str( );

                if( not variables.contains( piece ) )
                {
                    error << "Unknown variable!\n";
                    error << "Variable: \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                value = std::regex_replace( value, curr, variables.at( piece ), std::regex_constants::format_first_only );
                count = 1;

                continue;
            }

            if( std::regex_search( value, match, root ) )
            {
                piece = match[ 1 ].str( );

                if( not variables.contains( piece ) )
                {
                    error << "Unknown variable!\n";
                    error << "Variable: \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                value = std::regex_replace( value, root, variables.at( piece ), std::regex_constants::format_first_only );
                count = 1;

                continue;
            }
        }
        while( count != 0 );


        // Configuration has no parent.
        if( std::regex_search( value, match, prev ) )
        {
            error << "Configuration has no parent!\n";
            error << "Variable: \'" << match[ 1 ].str( ) << "\'";

            throw std::runtime_error( error.str( ) );
        }


        return value;
    }


    void        config::subst( std::string & string )       const
    {
        static std::regex const root( R"(\<([^\>]+)\>)" );
        static std::regex const prev( R"(\(([^\)]+)\))" );
        static std::regex const curr( R"(\{([^\}]+)\})" );


        std::stringstream error;
        std::string       piece;
        std::smatch       match;
        std::size_t       count;

        // Substitute all variables.
        do
        {
            count = 0;

            if( std::regex_search( string, match, curr ) )
            {
                piece = match[ 1 ].str( );

                if( not variables.contains( piece ) )
                {
                    error << "Unknown variable!\n";
                    error << "Variable: \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                string = std::regex_replace( string, curr, variables.at( piece ), std::regex_constants::format_first_only );
                count  = 1;

                continue;
            }

            if( std::regex_search( string, match, root ) )
            {
                piece = match[ 1 ].str( );

                if( not variables.contains( piece ) )
                {
                    error << "Unknown variable!\n";
                    error << "Variable: \'" << piece << "\'";

                    throw std::runtime_error( error.str( ) );
                }

                string = std::regex_replace( string, root, variables.at( piece ), std::regex_constants::format_first_only );
                count  = 1;

                continue;
            }
        }
        while( count != 0 );


        // Configuration has no parent.
        if( std::regex_search( string, match, prev ) )
        {
            error << "Configuration has no parent!\n";
            error << "Variable: \'" << match[ 1 ].str( ) << "\'";

            throw std::runtime_error( error.str( ) );
        }
    }




    // Building.
    // -------------------------------------------------------------------------
    recipe const & config::match( std::string const & target, recipe const * parent ) const
    {
        for( auto & element : recipes )
        {
            if( not element->match( target, parent, this ).empty( ) )
            {
                return * element;
            }
        }


        throw std::runtime_error( "No recipe for a \'" + target + "\'" );
    }


    std::string config::build( )                                                   const
    {
        static std::regex const element( R"([^'\s]+|'[^']+')" );
        static std::regex const  pathed( R"('\s*(.*\S)\s*')"  );


        if( not variables.contains( "list" ) )
        {
            return { };
        }


        std::string output;
        std::string  total = variables.at( "list" );
        std::string  piece;
        std::smatch  match;

        while( std::regex_search( total, match, element ) )
        {
            piece = match[ 0 ].str( );

            if( std::regex_match( piece, match, pathed ) )
            {
                piece = match[ 1 ].str( );
            }

            output += build( piece, nullptr );
            output += " ";

            total = std::regex_replace( total, element, "", std::regex_constants::format_first_only );
        }


        output.pop_back( );

        return output;
    }


    std::string config::build( std::string const & target, recipe const * parent ) const
    {
        recipe chosen = match( target, parent );

        chosen.bind_parent( parent );
        chosen.bind_origin( this   );

        return chosen.build( target );
    }
}