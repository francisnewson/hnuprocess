#ifndef SUBSCRIBERFACTORY_HH
#define SUBSCRIBERFACTORY_HH
#include <stdexcept>

#include "Subscriber.hh"

#if 0
/*   _____       _                   _ _               
 *  / ____|     | |                 (_) |              
 * | (___  _   _| |__  ___  ___ _ __ _| |__   ___ _ __ 
 *  \___ \| | | | '_ \/ __|/ __| '__| | '_ \ / _ \ '__|
 *  ____) | |_| | |_) \__ \ (__| |  | | |_) |  __/ |   
 * |_____/ \__,_|_.__/|___/\___|_|  |_|_.__/ \___|_|   
 * |  ____|       | |                                  
 * | |__ __ _  ___| |_ ___  _ __ _   _                 
 * |  __/ _` |/ __| __/ _ \| '__| | | |                
 * | | | (_| | (__| || (_) | |  | |_| |                
 * |_|  \__,_|\___|\__\___/|_|   \__, |                
 *                                __/ |                
 *                               |___/                 
 */
#endif


namespace YAML { class Node; }

namespace fn
{
    //SubscriberFactor handles the mapping between strings
    //and actual classes via create_subscriber functions

    class RecoFactory;
    class Subscriber;

    template<typename T> Subscriber * 
        create_subscriber(  YAML::Node& instruct, RecoFactory& rf )
        { return new T ;}

    class UnknownSubscriber : public std::out_of_range
    {
        public:
            UnknownSubscriber( std::string );
    };

    struct SubscriberFactory {
        typedef std::map<std::string, Subscriber *(*)
            (  YAML::Node& instruct, RecoFactory& rf ) > map_type;

        static Subscriber * get_subscriber( std::string const& s,
                YAML::Node& instruct, RecoFactory& rf );

        protected:
        static map_type& global_subscriber_map();
    } ;

    template<typename ST>
        struct SubscriberRegister : private SubscriberFactory 
    { 
        SubscriberRegister(std::string const& s)
        { 
            global_subscriber_map().insert(
                    std::make_pair(s, &create_subscriber<ST> ) );
        }
    };
}
#endif