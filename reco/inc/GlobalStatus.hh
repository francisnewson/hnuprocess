#ifndef GLOBALSTATUS_HH
#define GLOBALSTATUS_HH
#include <boost/optional.hpp>
#include "Subscriber.hh"
#include "Rtypes.h"

#if 0
/*
 *   ____ _       _           _ ____  _        _
 *  / ___| | ___ | |__   __ _| / ___|| |_ __ _| |_ _   _ ___
 * | |  _| |/ _ \| '_ \ / _` | \___ \| __/ _` | __| | | / __|
 * | |_| | | (_) | |_) | (_| | |___) | || (_| | |_| |_| \__ \
 *  \____|_|\___/|_.__/ \__,_|_|____/ \__\__,_|\__|\__,_|___/
 *
 *
 */
#endif
namespace fne
{
    class Event;
}

namespace fn
{
    class K2piEventData;
    
    //--------------------------------------------------
    //Hacky class for a few dependencies
    //--------------------------------------------------

    class GlobalStatus : public Subscriber
    {
        public:
            virtual Long64_t get_run() const = 0;
            virtual bool is_mc() const = 0;
            virtual ~GlobalStatus(){}
    };

    class FNEGStatus : public GlobalStatus
    {
        public:
            FNEGStatus( const fne::Event * e, bool mc );
            virtual Long64_t get_run() const;
            virtual bool is_mc() const;
            virtual ~FNEGStatus(){}

        private:
            const fne::Event* e_;
            bool mc_;
    };


    GlobalStatus*& raw_global_status();
    GlobalStatus& global_status();
}
#endif
