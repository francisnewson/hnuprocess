#ifndef PIONPLOTTER_HH
#define PIONPLOTTER_HH
#include "HistStore.hh"
#include "Analysis.hh"
#if 0
/*
 *  ____  _             ____  _       _   _
 * |  _ \(_) ___  _ __ |  _ \| | ___ | |_| |_ ___ _ __
 * | |_) | |/ _ \| '_ \| |_) | |/ _ \| __| __/ _ \ '__|
 * |  __/| | (_) | | | |  __/| | (_) | |_| ||  __/ |
 * |_|   |_|\___/|_| |_|_|   |_|\___/ \__|\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class K2piLkrData;
    class K2piDchData;
    class K2piMcData;
    class K2piEventData;

    class PionPlotter
    {
        public:
            PionPlotter(TFile & tf, std::string folder );
            void plot_pion( K2piEventData& event_data, 
                    K2piDchData& dch_data, double weight, bool mc );

            void write();

        private:
            TFile& tf_;
            std::string folder_;
            HistStore hs_;
            TH1D * heop_;
            TH1D * hp_;
            TH1D * hE_;
            TH2D * heop_p_;
            TH2D * hEp_;
    };

    //--------------------------------------------------

    class PionAnalysis : public Analysis
    {
        public:
        PionAnalysis( Selection& sel,
                TFile& tf_, std::string folder_, 
                K2piEventData& k2pi_data, bool is_mc );

        void end_processing();

        private:
        void process_event();

        K2piEventData& k2pi_data_;
        PionPlotter pion_plots_;

        bool is_mc_;
    };
}
#endif
