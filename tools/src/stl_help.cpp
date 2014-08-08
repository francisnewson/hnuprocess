#include "stl_help.hh"
#include <fstream>
namespace fn
{
    std::ostream& nullos()
    {
        static std::fstream ifs( "/dev/null" );
        return ifs;
    }

}
