#ifndef MOO_CONFIG_HH
#define MOO_CONFIG_HH

#include <cstdint>
#include <string>
#include <map>
#include <list>
#include <istream>
#include <memory>


namespace moo
{
    class recipe;

    class config
    {
    public:
       ~config( );
        config( std::istream & source, std::size_t & line );


        void append( std::istream & source, std::size_t & line );
        bool naming( std::string const & name );


        std::string subst( std::string const & string ) const;
        void        subst( std::string &       string ) const;


        recipe const & match( std::string const & target, recipe const * parent ) const;

        std::string    build( std::string const & target, recipe const * parent ) const;
        std::string    build( )                                                   const;

    private:
        void parse( std::istream & source, std::size_t & line );


    private:
        std::map<std::string, std::string> variables;
        std::list<std::unique_ptr<recipe>>   recipes;
    };
}


#endif