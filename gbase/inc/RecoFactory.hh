#ifndef RECOFACTORY_HH
#define RECOFACTORY_HH
#include "logging.hh"
#include "Subscriber.hh"
#include "yaml-cpp/yaml.h"

#if 0
/*  ____
 * |  __ \                             
 * | |__) |___  ___ ___                
 * |  _  // _ \/ __/ _ \               
 * | | \ \  __/ (_| (_) |              
 * |_|__\_\___|\___\___/               
 * |  ____|       | |                  
 * | |__ __ _  ___| |_ ___  _ __ _   _ 
 * |  __/ _` |/ __| __/ _ \| '__| | | |
 * | | | (_| | (__| || (_) | |  | |_| |
 * |_|  \__,_|\___|\__\___/|_|   \__, |
 *                                __/ |
 *                               |___/ 
 */
#endif

namespace fn
{
    class Reconstruction;

    //exceptions
    struct DuplicateSubscriberName{};
    struct UnknownSubscriberName{};

    class RecoFactory
    {
        public:
            RecoFactory( Reconstruction& reco );

            //logger
            void set_log( logger& log );
            logger& get_log();

            //subscribers
            void create_subscriber( std::string name, 
                    std::string subscriber_type, YAML::Node instruct );

            Subscriber * get_subscriber ( std::string name );

        private:
            Reconstruction& reco_;
            logger * log_;

            std::map< std::string, Subscriber* > subscribers_;
            unsigned int next_sub_id_;
    };
}
#endif
