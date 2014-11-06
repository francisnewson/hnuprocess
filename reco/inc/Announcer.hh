#ifndef ANNOUNCERY_HH
#define ANNOUNCERY_HH

#include "Subscriber.hh"
#include "Event.hh"

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
            Announcer( const fne::Event * e);
            void new_file();
            void new_burst();
            void new_run();
            void new_event();

            void end_processing();

        private:
            const fne::Event * e_;

            bool file_;
            bool run_;
            bool burst_;
            bool event_;

            REG_DEC_SUB( Announcer );
    };

    template<>
        Subscriber * create_subscriber<Announcer>
        (YAML::Node& instruct, RecoFactory& rf );

}

#endif
