#ifndef BASICMUONSTATS_HH
#define BASICMUONSTATS_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include <string>
#if 0
/*
 *  ____            _      __  __                   ____  _        _
 * | __ )  __ _ ___(_) ___|  \/  |_   _  ___  _ __ / ___|| |_ __ _| |_ ___
 * |  _ \ / _` / __| |/ __| |\/| | | | |/ _ \| '_ \\___ \| __/ _` | __/ __|
 * | |_) | (_| \__ \ | (__| |  | | |_| | (_) | | | |___) | || (_| | |_\__ \
 * |____/ \__,_|___/_|\___|_|  |_|\__,_|\___/|_| |_|____/ \__\__,_|\__|___/
 *
 *
 */
#endif

namespace fne { class Event; }
class TFile;

namespace fn
{

    //Collect some raw stats about muons hits
    //No reconstruction, and no special MC treatment
    class BasicMuonStats : public Analysis
    {
        public:
            BasicMuonStats( const Selection & sel, const fne::Event * event, bool filter,
                    TFile& tf, std::string folder );

            void end_processing();

        private:
            void process_event();

            const fne::Event * e_;
            TFile& tf_;
            std::string folder_;
            bool filter_;

            HistStore hs_;
            TH1D * h_muv_status_;
            TH2D * h_xy_;
            TH1D * h_nmuon_;
            TH1D * h_nhits_;
            TH2D * h_muons_vs_hits_;
            TH2D * h_separation_;
            TH1D * h_sep_x_;
            TH1D * h_sep_y_;

            REG_DEC_SUB( BasicMuonStats );
    };


    template<>
        Subscriber * create_subscriber<BasicMuonStats>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
