#ifndef MUONSHAPES_HH
#define MUONSHAPES_HH
#include "Analysis.hh"
#include "HistStore.hh"

#if 0
/*
 *  __  __                   ____  _
 * |  \/  |_   _  ___  _ __ / ___|| |__   __ _ _ __   ___  ___
 * | |\/| | | | |/ _ \| '_ \\___ \| '_ \ / _` | '_ \ / _ \/ __|
 * | |  | | |_| | (_) | | | |___) | | | | (_| | |_) |  __/\__ \
 * |_|  |_|\__,_|\___/|_| |_|____/|_| |_|\__,_| .__/ \___||___/
 *                                            |_|
 *
 */
#endif
namespace fn
{
    class SingleTrack;
    class SingleMuon;

    //Look at muon - track seperation etc
    class MuonShapes : public Analysis
    {
        public:
            MuonShapes( const Selection& sel, const SingleTrack& st, const SingleMuon & sm,
                    TFile& tf, std::string folder );

            void end_processing();

        private:
            void process_event();
            HistStore hs_;

            const SingleTrack & st_;
            const SingleMuon & sm_;

            TFile& tf_;
            std::string folder_;

            TH2D * h_xy_;
            TH2D * h_dxy_;
            TH2D * h_dsvsp_;
            TH1D * h_ds_;
            TH1D * h_dx_;
            TH1D * h_dy_;

            REG_DEC_SUB( MuonShapes );
    };

    template<>
        Subscriber * create_subscriber<MuonShapes>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
