#ifndef GETSUBSCRIBER_HH
#define GETSUBSCRIBER_HH
#include "yaml_help.hh"
#include "RecoFactory.hh"
#include <boost/algorithm/string.hpp>

#if 0
/*
 *   ____      _   ____        _                   _ _
 *  / ___| ___| |_/ ___| _   _| |__  ___  ___ _ __(_) |__   ___ _ __
 * | |  _ / _ \ __\___ \| | | | '_ \/ __|/ __| '__| | '_ \ / _ \ '__|
 * | |_| |  __/ |_ ___) | |_| | |_) \__ \ (__| |  | | |_) |  __/ |
 *  \____|\___|\__|____/ \__,_|_.__/|___/\___|_|  |_|_.__/ \___|_|
 *
 *
 */
#endif
namespace fn
{
    template  <class T>
        T * get_sub( YAML::Node& instruct, RecoFactory& rf, std::string name )
        {
            T * result = 0;
            std::string upper_name = boost::to_upper_copy( name );

            try
            {
                YAML::Node yogt = instruct["inputs"][name];

                if ( !yogt ){ throw Xcept<MissingNode>( name ); }

                result = dynamic_cast<T*>( rf.get_subscriber( yogt.as<std::string>() ) );

                if ( !result ){ throw Xcept<BadCast>( upper_name ); }
            }
            catch ( ... )
            {
                std::cerr << "Trying to get " << name << " (" __FILE__ ")\n";
                throw;
            }
            return result;
        }

    template  <class T>
        T * get_sub( YAML::Node& instruct, RecoFactory& rf);
}
#endif
