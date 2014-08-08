#ifndef RECOFACTORY_HH
#define RECOFACTORY_HH
#include "logging.hh"
#include "Subscriber.hh"
#include "Selection.hh"
#include "yaml-cpp/yaml.h"
#include "Rtypes.h"
#include <boost/filesystem/path.hpp>
#include "OSLoader.hh"

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

            //stream management
            void set_output_prefix( boost::filesystem::path prefix );
            void define_ostream( std::string name, std::string p );
            std::ostream& get_ostream( std::string name);

            //tfile management
			void  define_tfile
				( std::string name, boost::filesystem::path p );

			TFile& get_tfile( std::string name );

            //channel management
            void set_channel( const std::string& channel);
            std::string get_channel();

        private:
            Reconstruction& reco_;
            logger * log_;

            std::map< std::string, Subscriber* > subscribers_;
            std::map< std::string, Selection* > selections_;
            unsigned int next_sub_id_;

            //stream management
            boost::filesystem::path output_prefix_;
            std::map<std::string, std::ostream*> ostreams_;
            OSLoader osl_;

            //tfile management
			TFileLoader tfl_;
            std::map<std::string, TFile*> tfiles_;

            //channel management
            std::string channel_;
    };
}
#endif
