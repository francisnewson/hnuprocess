#ifndef K3PIPLOTS_HH
#define K3PIPLOTS_HH
#include "K3piReco.hh"
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  _  _______       _ ____  _       _
 * | |/ /___ / _ __ (_)  _ \| | ___ | |_ ___
 * | ' /  |_ \| '_ \| | |_) | |/ _ \| __/ __|
 * | . \ ___) | |_) | |  __/| | (_) | |_\__ \
 * |_|\_\____/| .__/|_|_|   |_|\___/ \__|___/
 *            |_|
 *
 */
#endif
namespace fn
{
    class K3piReco;

    class K3piPlots : public Analysis
    {
        public:
            K3piPlots( const Selection& sel, const K3piReco & k3pi_reco,
                    TFile& tf, std::string folder 
                    );

            void end_processing();

        private:
            void process_event();
            const K3piReco& k3pi_reco_;

            TFile& tfout_;
            std::string folder_;

            //Plots
            HistStore hs_;

            TH1D * h_m2_;
            TH1D * h_dm2_;
            TH1D * h_pt_;
            TH1D * h_chi2_;
            TH1D * h_p_;
            TH1D * h_dxdz_;
            TH1D * h_dydz_;
            TH2D * h_m2m_p_;

            REG_DEC_SUB( K3piPlots );
    };

    template<>
        Subscriber * create_subscriber<K3piPlots>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
