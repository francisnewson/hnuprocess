#ifndef SUBSCRIBER_HH
#define SUBSCRIBER_HH

#include <iosfwd>
#include <string>
#include "logging.hh"
#include "SubscriberFactory.hh"

#if 0
/*
 * 
 *    _____       _                   _ _               
 *   / ____|     | |                 (_) |              
 *  | (___  _   _| |__  ___  ___ _ __ _| |__   ___ _ __ 
 *   \___ \| | | | '_ \/ __|/ __| '__| | '_ \ / _ \ '__|
 *   ____) | |_| | |_) \__ \ (__| |  | | |_) |  __/ |   
 *  |_____/ \__,_|_.__/|___/\___|_|  |_|_.__/ \___|_|   
 *                                                      
 */                                                   
#endif


namespace fn
{
    class Subscriber
    {
        public:
            virtual void new_file(){};
            virtual void new_run(){};
            virtual void new_burst(){};

            virtual void new_event(){};

            virtual void end_burst(){};
            virtual void end_run(){};
            virtual void end_file(){};

            virtual void end_processing(){};

            void set_name( std::string s );
            std::string get_name();

            void set_id( int id );
            int get_id();

        private:
            std::string name_;
            int id_;
    };



}


#endif
