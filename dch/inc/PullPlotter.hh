#ifndef PULLPLOTTER_HH
#define PULLPLOTTER_HH

#include "Analysis.hh"
#include "HistStore.hh"
#include "K2piEventData.hh"

#if 0
/*
 *  ____        _ _ ____  _       _   _
 * |  _ \ _   _| | |  _ \| | ___ | |_| |_ ___ _ __
 * | |_) | | | | | | |_) | |/ _ \| __| __/ _ \ '__|
 * |  __/| |_| | | |  __/| | (_) | |_| ||  __/ |
 * |_|    \__,_|_|_|_|   |_|\___/ \__|\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class PullPlotter
    {
        public:
            enum class source
            {
                data, mc, mcafter
            };

            PullPlotter( TFile& tf, std::string folder, source data_source );

            void plot_pulls( 
                    const K2piLkrData& before, const K2piLkrData& after,
                    const K2piLkrData& errs, const K2piEventData& event, 
                    double wgt, double chi2 );

            void write();

        private:
            TFile& tf_;
            std::string folder_;

            HistStore hs_;

            TH1D * hE1_;
            TH1D * hE2_;

            TH1D * hX1_;
            TH1D * hY1_;

            TH1D * hX2_;
            TH1D * hY2_;

            TH1D * hpkx_;
            TH1D * hpky_;
            TH1D * hpkz_;

            TH1D * hkx0_;
            TH1D * hky0_;

            TH1D * hx0_;
            TH1D * hy0_;

            TH1D * h_prob_;
            TH1D * h_chi2_;

            source data_source_;
    };

    class PullPlotterAnalysis : public Analysis
    {
        public:
            PullPlotterAnalysis( 
                    Selection& sel,
                    TFile& tf, std::string folder,
                    const K2piLkrData& before,
                    const K2piLkrData& after,
                    const K2piLkrData& errs,
                    const K2piEventData& event,
                    PullPlotter::source data_source
                    );

            void end_processing();

        private:
            void process_event();
            PullPlotter pull_plotter_;

            const K2piLkrData& before_;
            const K2piLkrData& after_;
            const K2piLkrData& errs_;
            const K2piEventData& event_;
    };
}
#endif
