#ifndef TRACKING_SELECTIONS_HH
#define TRACKING_SELECTIONS_HH
#include "SingleTrack.hh"
#include "Selection.hh"
#if 0
/*
 *  _                  _    _             
 * | |_ _ __ __ _  ___| | _(_)_ __   __ _ 
 * | __| '__/ _` |/ __| |/ / | '_ \ / _` |
 * | |_| | | (_| | (__|   <| | | | | (_| |
 *  \__|_|  \__,_|\___|_|\_\_|_| |_|\__, |
 *                                  |___/ 
 *           _           _   _                 
 *  ___  ___| | ___  ___| |_(_) ___  _ __  ___ 
 * / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                                             
 */
#endif

namespace fn
{
    SingleTrack * get_single_track
        ( YAML::Node& instruct, RecoFactory& rf );

    class FoundGoodTrack : public CachedSelection
    {
        public: 
            FoundGoodTrack( const SingleTrack& st );

        private:
            bool do_check() const;
            const SingleTrack& st_;

                REG_DEC_SUB( FoundGoodTrack);
    };

    template<>
        Subscriber * create_subscriber<FoundGoodTrack>
        (YAML::Node& instruct, RecoFactory& rf );

//--------------------------------------------------

    class TrackCharge : public CachedSelection
    {
        public: 
            TrackCharge( const SingleTrack& st, int charge );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            int charge_;

                REG_DEC_SUB( TrackCharge);
    };

    template<>
        Subscriber * create_subscriber<TrackCharge>
        (YAML::Node& instruct, RecoFactory& rf );

//--------------------------------------------------
}
#endif
