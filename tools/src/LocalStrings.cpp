#include "LocalStrings.hh"
#include "yaml-cpp/yaml.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace boost
{
    namespace filesystem
    {

        template <> path& path::append<path::iterator>(path::iterator begin, path::iterator end, 
                const codecvt_type& cvt)
        {
            for( ; begin != end ; ++begin )
                *this /= *begin;
            return *this;
        }

        // Return path when appended to a_From will resolve to same as a_To
        boost::filesystem::path make_relative( boost::filesystem::path a_From, boost::filesystem::path a_To )
        {
            a_From = boost::filesystem::absolute( a_From ); a_To = boost::filesystem::absolute( a_To );
            boost::filesystem::path ret;
            boost::filesystem::path::const_iterator itrFrom( a_From.begin() ), itrTo( a_To.begin() );
            // Find common base
            for( boost::filesystem::path::const_iterator toEnd( a_To.end() ), fromEnd( a_From.end() ) ;
                    itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo );

            // Navigate backwards in directory to reach previously found base
            for( boost::filesystem::path::const_iterator fromEnd( a_From.end() ); itrFrom != fromEnd; ++itrFrom )
            {
                if( (*itrFrom) != "." )
                    ret /= "..";
            }
            // Now navigate down the directory branch
            ret.append( itrTo, a_To.end() );
            return ret;
        }

    }
} // namespace boost::filesystem


namespace fn
{

    LocalStrings::LocalStrings( std::string file )
    {
        strings_ = YAML::LoadFile( file ).as<std::map<std::string, std::string>>();
    }


    boost::filesystem::path LocalStrings::convert( boost::filesystem::path p )
    {
        if ( strings_["machine"] == "bham" && boost::algorithm::contains( p.string(), "afs/cern.ch" ) )
        {
            auto relative_path = make_relative( "/afs/cern.ch/user/f/fnewson/work/hnu/gopher/data", p );
            return boost::filesystem::path( strings_["data"] ) / relative_path;
        }
        else
        {
            return p;
        }
    }

    LocalStrings& g_local_strings()
    {
        static LocalStrings ls( "local/prefixes.yaml" );
        return ls;
    }
}
