#ifndef K2PIPLOTTER_HH
#define K2PIPLOTTER_HH
#include "Analysis.hh"
#include "K2piReco.hh"
#include "HistStore.hh"

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
namespace fn
{
    class K2piPlotter : public Analysis
    {
        public: 
            K2piPlotter( const Selection& sel,
                TFile& tfile, std::string folder,
                const K2piReco& k2pi_reco);

        void end_processing();

        private:
        void process_event();

        TH1D * h_m2_pip_lkr_;
        TH1D * h_m2_pi0;
        TH1D * h_neutral_z_;

        TFile& tfile_;
        std::string folder_;
        HistStore hs_;

        const K2piReco& k2pi_reco_;

        REG_DEC_SUB( K2piPlotter);

    };

    template<>
        Subscriber * create_subscriber<K2piPlotter>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
