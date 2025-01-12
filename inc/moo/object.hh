#ifndef MOO_OBJECT_HH
#define MOO_OBJECT_HH

#include <string>
#include <map>
#include <list>


namespace moo
{
    // Base for the other objects.
    class object
    {
    protected:
       ~object( ) = default;
        object( ) = default;


    public:
        std::map<std::wstring, std::wstring> vars;
    };
}


#endif