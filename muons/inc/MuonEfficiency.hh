#ifndef MUONEFFICIENCY_HH
#define MUONEFFICIENCY_HH
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  __  __                   _____  __  __ _      _
 * |  \/  |_   _  ___  _ __ | ____|/ _|/ _(_) ___(_) ___ _ __   ___ _   _
 * | |\/| | | | |/ _ \| '_ \|  _| | |_| |_| |/ __| |/ _ \ '_ \ / __| | | |
 * | |  | | |_| | (_) | | | | |___|  _|  _| | (__| |  __/ | | | (__| |_| |
 * |_|  |_|\__,_|\___/|_| |_|_____|_| |_| |_|\___|_|\___|_| |_|\___|\__, |
 *                                                                  |___/
 *
 */
#endif
namespace fn
{

    class SingleTrack;
    class SingleMuon;
    class MuonReq;

    //produce efficiency plots using MuonReq object
    //as functions of track properties
    class MuonEfficiency : public Analysis
    {
        public:
            MuonEfficiency(const Selection& sel,
                    const Selection& mr, const SingleTrack& st,
                    TFile& tf, std::string folder );

            void end_processing();

        private:
            void process_event();

            const Selection& mr_;
            const SingleTrack& st_;

            TFile& tf_;
            std::string folder_;

            HistStore hs_;

            TH2D * h_xy_passed_ ;
            TH2D * h_xy_total_;
            TH1D * h_p_passed_;
            TH1D * h_p_total_ ;

            TH2D * h_track_pr_;

            REG_DEC_SUB( MuonEfficiency );
    };


    template<>
        Subscriber * create_subscriber<MuonEfficiency>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    //Plot efficiency as a function of multiplier
    class MuonMultiplierEfficiency : public Analysis
    {
        public:
            MuonMultiplierEfficiency(const Selection& sel,
                    std::vector<double> cut_values,
                    const SingleMuon& sm, const SingleTrack& st,
                    TFile& tf, std::string folder );

            void end_processing();

        private:
            void process_event();

            std::vector<double> cut_values_;
            std::vector<double> passed_;
            double total_;

            const SingleMuon& sm_;
            const SingleTrack& st_;

            TFile& tf_;
            std::string folder_;


            HistStore hs_;

            REG_DEC_SUB( MuonMultiplierEfficiency );
    };

    template<>
        Subscriber * create_subscriber<MuonMultiplierEfficiency>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
