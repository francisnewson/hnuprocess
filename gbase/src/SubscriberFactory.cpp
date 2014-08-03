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
            throw UnknownSubscriber( s );
        }
        else
        {
            return it->second( instruct, rf );
        }
    }

    SubscriberFactory::map_type& SubscriberFactory::global_subscriber_map()
    {
        static map_type map_;
        return map_;
    }
}
