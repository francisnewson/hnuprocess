#ifndef MUONCHECK_HH
#define MUONCHECK_HH
#include "Analysis.hh"
#include "HistStore.hh"

#if 0
/*
 *  __  __                    ____ _               _
 * |  \/  |_   _  ___  _ __  / ___| |__   ___  ___| | __
 * | |\/| | | | |/ _ \| '_ \| |   | '_ \ / _ \/ __| |/ /
 * | |  | | |_| | (_) | | | | |___| | | |  __/ (__|   <
 * |_|  |_|\__,_|\___/|_| |_|\____|_| |_|\___|\___|_|\_\
 *
 *
 */
#endif
namespace fn
{
    class SingleTrack;
    class SingleRecoTrack;
    class SingleMuon;
    class Km2Event;
    class Km2RecoEvent;
    class MuonReq;
    class KaonTrack;

    class MuonCheckPlots
    {
        public:
            MuonCheckPlots();
            void Fill( const SingleRecoTrack& srt, 
                    const KaonTrack& kt,  const Km2RecoEvent& kmrre,
                    double wgt );

            void Write();

        private:
            HistStore hs_;
            TH1D  * h_m2m_;
            TH1D  * h_p_;
            TH1D  * h_t_;
            TH1D  * h_z_;
            TH1D  * h_rmuv_;
            TH1D  * h_phi_;
    };

    //--------------------------------------------------


    class MuonCheck : public Analysis
    {
        public:
        MuonCheck( const Selection& sel,
                const Selection& mr,
                const SingleTrack& st,
                const KaonTrack& kt,
                const Km2Event & km2e,
                TFile& tf, std::string folder );

        void end_processing();


        private:
        void process_event();

        const Selection& mr_;
        const SingleTrack& st_;
        const KaonTrack& kt_;
        const Km2Event& km2e_;

        TFile& tf_;
        std::string folder_;

        MuonCheckPlots pass_;
        MuonCheckPlots all_;

        REG_DEC_SUB( MuonCheck );
    };

    template<>
        Subscriber * create_subscriber<MuonCheck>
        ( YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif
