#include "stl_help.hh"
#include <fstream>
#include <iomanip>
namespace fn
{
    std::ostream& nullos()
    {
        static std::fstream ifs( "/dev/null" );
        return ifs;
    }

    void echo_launch( int argc, char * argv[], std::ostream& os, char sep )
    {
        os << "Launched with the command: " << std::endl;
        for (int arg = 0 ; arg != argc ; ++arg)
        {
            if (argv[arg][0] =='-')
            { 
                os << sep << std::setw( 15 ) << argv[arg];
            }
            else
            {
                os << " " << argv[arg] ;
            }
        }
        os << "\n" << std::endl;

    }

    void write_launch( int argc, char * argv[], 
            std::ostream& os )
    {
        for (int arg = 0 ; arg != argc ; ++arg)
        {
            os << " " << argv[arg];
        }
        os << "\n" ;
    }

    bool contains( const std::string& s1, const std::string& s2 )
    {
        return s1.find( s2 ) != std::string::npos ;
    }
}
