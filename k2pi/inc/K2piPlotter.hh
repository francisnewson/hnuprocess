#ifndef K2PIPLOTTER_HH
#define K2PIPLOTTER_HH
#include "Analysis.hh"
#include "K2piReco.hh"
#include "HistStore.hh"
#include "Event.hh"
#include "KaonTrack.hh"
#include "K2piVars.hh"
#include "K2piPlots.hh"

#if 0
/*
 *  _  ______        _ ____  _       _   _
 * | |/ /___ \ _ __ (_)  _ \| | ___ | |_| |_ ___ _ __
 * | ' /  __) | '_ \| | |_) | |/ _ \| __| __/ _ \ '__|
 * | . \ / __/| |_) | |  __/| | (_) | |_| ||  __/ |
 * |_|\_\_____| .__/|_|_|   |_|\___/ \__|\__\___|_|
 *            |_|
 *
 */
#endif
//--------------------------------------------------
//
//  K2piPlotter is a wrapper for K2piPlots. It fills a
//  K2piVars object but the actual plotting is done by 
//  K2piPlots.
//
//--------------------------------------------------
namespace fn
{
    class K2piPlotter : public Analysis
    {
        public: 
            K2piPlotter( const Selection& sel,
                    const fne::Event * e,
                    const KaonTrack& kt,
                    TFile& tfile, std::string folder,
                    const K2piReco& k2pi_reco, bool  mc);

            void end_processing();

        private:
            void process_event();
            const fne::Event * e_;
            const KaonTrack& kt_;

            TFile& tfile_;
            std::string folder_;

            const K2piReco& k2pi_reco_;
            K2piVars vars_;
            bool mc_;

            K2piPlots k2pi_plots_;


            REG_DEC_SUB( K2piPlotter);

    };

    template<>
        Subscriber * create_subscriber<K2piPlotter>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
