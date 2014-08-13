#include "yaml-cpp/yaml.h"
#include "Xcept.hh"
#include <iostream>

namespace fn
{
    template < typename T>
        T get_yaml( const YAML::Node& node,  std::string s )
        {
            T result;

            try 
            {
                result = node[s].as<T>();
            }
            catch( YAML::Exception& c )
            {
                std::cerr << "Extracting " <<  s << "\n";
                throw Xcept<MissingInfo>( s );
            }

            return result;
        }



}