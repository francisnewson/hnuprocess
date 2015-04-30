#ifndef MUVPMTHITS_HH
#define MUVPMTHITS_HH
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  __  __             ____            _   _   _ _ _
 * |  \/  |_   ___   _|  _ \ _ __ ___ | |_| | | (_) |_ ___
 * | |\/| | | | \ \ / / |_) | '_ ` _ \| __| |_| | | __/ __|
 * | |  | | |_| |\ V /|  __/| | | | | | |_|  _  | | |_\__ \
 * |_|  |_|\__,_| \_/ |_|   |_| |_| |_|\__|_| |_|_|\__|___/
 *
 *
 */
#endif
namespace fne
{
    class Event;
}

namespace fn
{
    class Km2RecoEvent;
    class Km2Event;
    class SingleRecoTrack;

    class MuvPmtPlotter
    {
        public:
            MuvPmtPlotter( std::string name, int n_strips );
            void Fill( int strip_id, double x, double y, double wgt );
            void Write();

        private:
            HistStore hs_;
            std::vector<TH2D*> vhs_;
    };

    class MuvPmtHits : public Analysis
    {
        public:
        MuvPmtHits( const Selection& sel, 
                const Km2Event& km2_event, const fne::Event * e,
                TFile& tfile, std::string folder);

            void end_processing();

        private:
            void process_event();

            const Km2Event& km2e_;
            const fne::Event * e_;

            TFile& tfile_;
            std::string folder_;

            MuvPmtPlotter muv1_;
            MuvPmtPlotter muv2_;
            MuvPmtPlotter muv3_;
            MuvPmtPlotter muv_all_;


            HistStore hs_;
            TH1D * hpmt_ids_;

            TH1D * htime_;
            TH1D * hacc_time_;

            TH2D * hchan_time_;

            REG_DEC_SUB( MuvPmtHits );

    };

    template<>
        Subscriber * create_subscriber<MuvPmtHits>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
