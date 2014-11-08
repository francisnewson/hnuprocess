#ifndef MUVEFF_HH
#define MUVEFF_HH
#include "TH1D.h"
#include "Analysis.hh"
#include "HistStore.hh"
#include "TEfficiency.h"
#include "RecoMuon.hh"
#include "Km2Plotter.hh"
#include "Event.hh"
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
    class SingleRecoTrack;

    //--------------------------------------------------

    class MuvGeom {
        public:
            MuvGeom( int status );
            void Fill( const fne::RecoMuon& rm, 
                    const SingleRecoTrack& srt,
                    double wgt);
            void Write();

        private:
            int status_;
            TH2D * h_muv_hits_;
            TH2D * h_muv_track_hits_;
            HistStore hs_;
    };

    //--------------------------------------------------

    //Plot efficiencies etc for MUV
    class MuvEff : public Analysis
    {
        public:
            MuvEff( const Selection& sel, const Selection& muv_selection,
                    const Km2Event& km2_event, const fne::Event * e,
                    TFile& tfile, std::string folder);

            void end_processing();

        private:
            void process_event();

            std::string channel_;

            REG_DEC_SUB( MuvEff );

            const Selection& muv_selection_;
            const Km2Event& km2e_;
            const fne::Event * e_;

            TFile& tfile_;
            std::string folder_;

            Km2Plots all_plots_;
            Km2Plots pass_plots_;
            Km2Plots fail_plots_;

            MuvGeom muvhits_1_;
            MuvGeom muvhits_2_;
            MuvGeom muvhits_3_;
            MuvGeom muvhits_4_;

            TEfficiency eff_mom_;
            TEfficiency eff_m2m_;
            TEfficiency eff_z_;
            TEfficiency eff_rmuv1_;
    };

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<MuvEff>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
