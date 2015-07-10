#ifndef MUON3DEFFICIENCY_HH
#define MUON3DEFFICIENCY_HH
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  __  __                   _____ ____  _____  __  __
 * |  \/  |_   _  ___  _ __ |___ /|  _ \| ____|/ _|/ _|
 * | |\/| | | | |/ _ \| '_ \  |_ \| | | |  _| | |_| |_
 * | |  | | |_| | (_) | | | |___) | |_| | |___|  _|  _|
 * |_|  |_|\__,_|\___/|_| |_|____/|____/|_____|_| |_|
 *
 *
 */
#endif
namespace fn
{
    class SingleTrack;
    class SingleMuon;
    class MuonReq;

    class Muon3DEfficiency : public Analysis
    {
        public:
            Muon3DEfficiency( const Selection& sel,
                    const Selection& mr,
                    const SingleTrack& st,
                    TFile& tf, std::string folder );

            void end_processing();


        private:
            void process_event();

            const Selection& mr_;
            const SingleTrack& st_;

            TFile& tf_;
            std::string folder_;

            HistStore hs_;

            TH3D * h_xyp_passed_;
            TH3D * h_xyp_total_;

            REG_DEC_SUB( Muon3DEfficiency );
    };

    template<>
        Subscriber * create_subscriber<Muon3DEfficiency>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif
