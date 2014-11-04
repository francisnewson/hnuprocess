#ifndef MUVEFF_HH
#define MUVEFF_HH
#include "TH1D.h"
#include "Analysis.hh"
#include "HistStore.hh"
#include "TEfficiency.h"
#if 0
/*
 *  __  __             _____  __  __
 * |  \/  |_   ___   _| ____|/ _|/ _|
 * | |\/| | | | \ \ / /  _| | |_| |_
 * | |  | | |_| |\ V /| |___|  _|  _|
 * |_|  |_|\__,_| \_/ |_____|_| |_|
 *
 *
 */
#endif
namespace fn
{
    class Km2RecoEvent;
    class Km2Event;

    //MUVPLOTS - plot all relevant info for a km2 event
    class MuvPlots {
        public:
            MuvPlots();
            void Fill( Km2RecoEvent& km2re, double weight );
            void Write();

        private:
            TH1D * h_mom_;
            TH1D * h_m2miss_;
            TH2D * h_m2p_;
            TH2D * h_pz_;
            HistStore hs_;
    };

    //--------------------------------------------------

    //Plot efficiencies etc for MUV
    class MuvEff : public Analysis
    {
        public:
        MuvEff( const Selection& sel, const Selection& muv_selection,
                const Km2Event& km2_event,
                TFile& tfile, std::string folder);

            void end_processing();

        private:
        void process_event();

        std::string channel_;

        REG_DEC_SUB( MuvEff );

        const Selection& muv_selection_;
        const Km2Event& km2e_;

        TFile& tfile_;
        std::string folder_;

        MuvPlots all_plots_;
        MuvPlots pass_plots_;
        MuvPlots fail_plots_;

        TEfficiency eff_mom_;
    };

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<MuvEff>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
