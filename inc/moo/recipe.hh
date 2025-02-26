#ifndef MOO_RECIPE_HH
#define MOO_RECIPE_HH

#include <cstdint>
#include <string>
#include <map>
#include <list>
#include <istream>


namespace moo
{
    class config;


    class recipe
    {
    public:
       ~recipe( );
        recipe( std::istream & source, std::size_t & line );


        void bind_parent( recipe const * parent ) noexcept;
        void drop_parent( )                       noexcept;

        void bind_origin( config const * origin ) noexcept;
        void drop_origin( )                       noexcept;


        std::string subst( std::string const & string )                                               const;
        std::string subst( std::string const & string, recipe const * parent, config const * origin ) const;
        void        subst( std::string &       string )                                               const;
        void        subst( std::string &       string, recipe const * parent, config const * origin ) const;


        std::string            match( std::string const & target, recipe const * parent, config const * origin ) const;
        std::string            place( std::string const & target )                                               const;
        std::string            build( std::string const & target );
        std::list<std::string> ready( std::string const & target );


    private:
        void parse( std::istream & source, std::size_t & line );
        bool cache( std::string const & output );


    private:
        std::map<std::string, std::string>    variables;
        std::map<std::string, std::string> preparations;

        recipe const * parent;
        config const * origin;
    };
}


#endif