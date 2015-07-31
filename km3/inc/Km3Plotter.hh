#ifndef KM3PLOTTER_HH
#define KM3PLOTTER_HH
#if 0
/*
 *  _  __          _____ ____  _       _   _
 * | |/ /_ __ ___ |___ /|  _ \| | ___ | |_| |_ ___ _ __
 * | ' /| '_ ` _ \  |_ \| |_) | |/ _ \| __| __/ _ \ '__|
 * | . \| | | | | |___) |  __/| | (_) | |_| ||  __/ |
 * |_|\_\_| |_| |_|____/|_|   |_|\___/ \__|\__\___|_|
 *
 *
 */
#endif
#include "Analysis.hh"
#include "Km3Reco.hh"
#include "HistStore.hh"

namespace fn
{

    class Km3Plots
    {
        public:
            Km3Plots();
            void Fill( const Km3RecoEvent& re, double wgt );
            void Write();

        private:
            TH1D * h_m2m_;
            TH1D * h_ptm_;
            TH1D * h_pm_;
            TH1D * h_dv_;
            TH1D * h_z_;
            TH1D * h_eop_;
            TH1D * h_cda_;
            TH2D * h_m2_vs_p_m_;
            TH2D * h_m2_vs_pt_m_;
            HistStore hs_;
    };

    class Km3Plotter : public Analysis
    {
        public:
            Km3Plotter( const Selection& sel,
                    TFile& tfile, std::string folder, 
                    const Km3Reco& km3_reco );

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const Km3Reco& km3_reco_;
            Km3Plots plots_;

            REG_DEC_SUB( Km3Plotter );
    };

    template<>
        Subscriber * create_subscriber<Km3Plotter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
