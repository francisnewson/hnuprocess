#ifndef ANNOUNCERY_HH
#define ANNOUNCERY_HH

#include "Subscriber.hh"

#if 0
/*                                                       
 *     /\                                                
 *    /  \   _ __  _ __   ___  _   _ _ __   ___ ___ _ __ 
 *   / /\ \ | '_ \| '_ \ / _ \| | | | '_ \ / __/ _ \ '__|
 *  / ____ \| | | | | | | (_) | |_| | | | | (_|  __/ |   
 * /_/    \_\_| |_|_| |_|\___/ \__,_|_| |_|\___\___|_|   
 */
#endif

namespace fn
{
    class Announcer : public Subscriber
    {
        public:
            Announcer( logger& log);
            void new_file();
            void new_burst();
            void new_run();
            void new_event();

            void end_processing();

        private:
            logger& log_;
            severity_level sl_;

            static SubscriberRegister<Announcer> announcer_reg_;
    };

    template<>
        Subscriber * create_subscriber<Announcer>
        (YAML::Node& instruct, RecoFactory& rf );

}

#endif
