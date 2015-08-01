#ifndef YAML_HELP_HH
#define YAML_HELP_HH
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
                std::cerr << c.what() << std::endl;
                std::cerr << node << std::endl;
                throw Xcept<MissingInfo>( s );
            }

            return result;
        }

    template < typename T>
        T get_yaml_default( const YAML::Node& node,  std::string s, T default_value )
        {
            T result = default_value;

            if ( const auto& res = node[s] )
            {
                try 
                {
                    result = res.as<T>();
                }
                catch( YAML::Exception& c )
                {
                    std::cerr << "Extracting " <<  s << "\n";
                    std::cerr << c.what() << std::endl;
                    throw Xcept<MissingInfo>( s );
                }
            }

            return result;
        }
}
#endif
