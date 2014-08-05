#include "SubscriberFactory.hh"
#include "yaml-cpp/yaml.h"
#include "RecoFactory.hh"


namespace fn
{

    UnknownSubscriber::UnknownSubscriber( std::string s )
        :std::out_of_range( s ) {}

    Subscriber * SubscriberFactory::get_subscriber( std::string const& s,
            YAML::Node& instruct, RecoFactory& rf )
    {
        map_type::iterator it =
            global_subscriber_map().find( s );

        if ( it == global_subscriber_map().end() )
        {
            throw UnknownSubscriber( s + __FILE__ );
        }
        else
        {
            Subscriber * result =  it->second( instruct, rf );

            if (YAML::Node level_node = instruct["log_level"]) {
                result->set_log_level(
                global_severity_map().at(level_node.as<std::string>()) );
            }

            return result;
        }
    }

    SubscriberFactory::map_type& SubscriberFactory::global_subscriber_map()
    {
        static map_type map_;
        return map_;
    }
}
