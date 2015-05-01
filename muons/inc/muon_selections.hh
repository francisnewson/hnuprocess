#ifndef MUON_SELECTIONS_HH
#define MUON_SELECTIONS_HH
#include "Selection.hh"
#include "Event.hh"
#if 0
/*
 *                                           _           _   _
 *  _ __ ___  _   _  ___  _ __      ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * | '_ ` _ \| | | |/ _ \| '_ \    / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * | | | | | | |_| | (_) | | | |   \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |_| |_| |_|\__,_|\___/|_| |_|___|___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                            |_____|
 *
 */
#endif
namespace fn
{

    class SingleMuon;
    class SingleTrack;

    //Require a muon associated with the track
    //Always needs MUV1 and MUV2
    //Efficiency is handled by SingleMuon class
    //Additional cut on track-muon separation is handled here.
    class MuonReq : public CachedSelection
    {
        public:
            MuonReq( const SingleMuon& sm, 
                    const SingleTrack& st, double multiplier );

        private:
            bool do_check() const;
            double do_weight() const;

            const SingleMuon& sm_;
            const SingleTrack& st_;
            double multiplier_;

            REG_DEC_SUB( MuonReq );
    };

    template<>
        Subscriber * create_subscriber<MuonReq>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
