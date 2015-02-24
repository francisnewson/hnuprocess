#ifndef STUDYSINGLEMUON_HH
#define STUDYSINGLEMUON_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "muon_selections.hh"
#include <string>
#if 0
/*
 *  ____  _             _       ____  _             _      __  __
 * / ___|| |_ _   _  __| |_   _/ ___|(_)_ __   __ _| | ___|  \/  |_   _  ___  _ __
 * \___ \| __| | | |/ _` | | | \___ \| | '_ \ / _` | |/ _ \ |\/| | | | |/ _ \| '_ \
 *  ___) | |_| |_| | (_| | |_| |___) | | | | | (_| | |  __/ |  | | |_| | (_) | | | |
 * |____/ \__|\__,_|\__,_|\__, |____/|_|_| |_|\__, |_|\___|_|  |_|\__,_|\___/|_| |_|
 *                        |___/               |___/
 *
 */
#endif

class TFile;

namespace fne {
    class RecoMuon;
}

namespace fn
{
    class SingleMuon;
    class SingleTrack;
    class SingleRecoTrack;
    class Km2Event;

    class StudySingleMuon : public Analysis
    {
        public:
            StudySingleMuon( const Selection & sel, 
                    const SingleMuon& sm, const SingleTrack& st ,  
                    const Selection& muv, const Km2Event& km2, 
                    TFile& tfile, std::string folder);


            void end_processing();

        private:

            //Nested classes for duplicated plots

            //Variables for efficiencies
            struct eff_plots
            {
                eff_plots();
                void fill( const SingleRecoTrack& srt, double wgt );
                void write();
                HistStore hs_;
                TH1D * hmom_;
                TH2D * hxy_;
            };

            struct muon_plots
            {
                muon_plots();
                void fill( const SingleRecoTrack& srt,
                        const fne::RecoMuon& mu, double wgt );

                void write();
                HistStore hs_;
                TH2D * h_track_muon_sep_;
                TH2D * h_track_muon_mom_dist_;
                TH1D * h_track_muon_dist_;
            };

            void process_event();
            const SingleMuon& sm_;
            const SingleTrack& st_;
            const Selection& muv_;
            const Km2Event& km2_;

            TH1D * h_m2m_;
            HistStore hs_;

            TFile& tfile_;
            std::string folder_;

            eff_plots all_;
            eff_plots pass_;
            eff_plots wgtpass_;

            muon_plots muon_all_;
            muon_plots muon_pass_;

            REG_DEC_SUB( StudySingleMuon );
    };

    template<>
        Subscriber * create_subscriber<StudySingleMuon>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
