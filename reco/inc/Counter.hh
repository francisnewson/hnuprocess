#ifndef COUNTER_HH
#define COUNTER_HH

#include "logging.hh"
#include "Subscriber.hh"

#if 0
/*
 *   _____                  _            
 *  / ____|                | |           
 * | |     ___  _   _ _ __ | |_ ___ _ __ 
 * | |    / _ \| | | | '_ \| __/ _ \ '__|
 * | |___| (_) | |_| | | | | ||  __/ |   
 *  \_____\___/ \__,_|_| |_|\__\___|_|   
 *                                       
 */
#endif

namespace fn
{

    int nice_mod_count ( int nEvents );

    class Counter : public Subscriber
    {
        public:
            Counter( logger& log, int max );
            void new_event();

        private:
            logger& log_;
            int max_;
            int current_;
            int mod_;

            REG_DEC_SUB( Counter);
    };

    template<>
        Subscriber * create_subscriber<Counter>
        (YAML::Node& instruct, RecoFactory& rf );

}

#endif
