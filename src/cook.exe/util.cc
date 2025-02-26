#include <moo/util.hh>
#include <moo/recipe.hh>
#include <moo/config.hh>

#include <regex>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <exception>
#include <list>


namespace moo
{
    // Parsing and building.
    // -------------------------------------------------------------------------
    static std::list<config> configs;


    void parse( std::string const & filename      )
    {
        std::ifstream file( filename );

        if( not file.is_open( ) )
        {
            throw std::runtime_error( "Can not open a file!" );
        }


        std::stringstream error;
        std::string       piece;
        std::smatch       match;
        std::size_t        line;

        for( line = 1; file.peek( ) != EOF; ++line )
        {
            static std::regex const comment( R"([#;].*)"                  );
            static std::regex const  config( R"(\[\s*configuration\s*\])" );
            static std::regex const  recipe( R"(\[\s*recipe\s*\])"        );


            std::getline( file, piece );


            if( piece.empty( ) )
            {
                continue;
            }


            if( std::regex_match( piece, comment ) )
            {
                continue;
            }


            if( std::regex_match( piece, config ) )
            {
                configs.emplace_back( file, line );

                continue;
            }


            if( std::regex_match( piece, recipe ) )
            {
                if( configs.empty( ) )
                {
                    throw std::runtime_error( "Recipe without a configuration!" );
                }

                configs.back( ).append( file, line );

                continue;
            }


            // Invalid file contents.
            {
                error << "Invalid file content!\n";
                error << line << ": \'" << piece << "\'";

                throw std::runtime_error( error.str( ) );
            }
        }
    }


    void build( std::string const & configuration )
    {
        for( auto & element : configs ) if( element.naming( configuration ) )
        {
            element.build( );
        }
    }




    // Correction functions.
    // -------------------------------------------------------------------------
    void correct_gaps( std::string & path ) noexcept
    {
        std::list<std::string> parts;


        // Split string into multiple parts.
        {
            std::regex expression( R"([^'\s]+|'[^']+')" );
            std::smatch     match;

            while( std::regex_search( path, match, expression ) )
            {
                parts.emplace_back( match[ 0 ].str( ) );

                path = std::regex_replace( path, expression, "", std::regex_constants::format_first_only );
            }
        }


        // Correct gaps ath the beginnig and at the end of the path.
        {
            std::regex expression( R"('\s*(.*\S)\s*')" );
            std::smatch     match;

            for( auto & piece : parts ) if( std::regex_match( piece, match, expression ) )
            {
                piece = match[ 1 ].str( );

                piece.insert( piece.cbegin( ), 1, '\'' );
                piece.insert( piece.cend( ),   1, '\'' );
            }
        }


        // Construct it all back.
        {
            path.clear( );

            for( auto & piece : parts )
            {
                path += piece;
                path += " ";
            }

            path.pop_back( );
        }
    }


    void correct_path( std::string & path ) noexcept
    {
        static std::regex  const separator( R"([\\\/])" );
        static std::regex  const quotation( R"(')"      ); 
        static std::string const   delimit( 1, std::filesystem::path::preferred_separator );


        correct_gaps( path );

        path = std::regex_replace( path, separator, delimit );
        path = std::regex_replace( path, quotation, R"(")"  );
    }
}