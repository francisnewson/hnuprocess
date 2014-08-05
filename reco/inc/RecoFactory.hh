#ifndef RECOFACTORY_HH
#define RECOFACTORY_HH
#include "logging.hh"
#include "Subscriber.hh"
#include "Selection.hh"
#include "yaml-cpp/yaml.h"
#include "Rtypes.h"

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

namespace fne
{
    class Event;
}

namespace fn
{
    class Reconstruction;

    //exceptions
    struct DuplicateSubscriberName{};
    struct DuplicateSelectionName{};
    struct UnknownSubscriberName{};
    struct UnknownSelectionName{};

    class RecoFactory
    {
        public:
            RecoFactory( Reconstruction& reco );

            //logger
            void set_log( logger& log );
            logger& get_log();

            //reco interaction
            Long64_t get_max_events();
            const fne::Event * get_event_ptr();

            //subscribers
            Subscriber * create_subscriber( std::string name, 
                    std::string subscriber_type, YAML::Node instruct );

            Subscriber * get_subscriber ( std::string name );

            //Selection management
            void add_selection ( std::string name, Selection * s  );
            Selection * get_selection ( std::string name);

        private:
            Reconstruction& reco_;
            logger * log_;

            std::map< std::string, Subscriber* > subscribers_;
            std::map< std::string, Selection* > selections_;
            unsigned int next_sub_id_;
    };
}
#endif
