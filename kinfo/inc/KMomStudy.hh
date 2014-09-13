#ifndef KMOMSTUDY_HH
#define KMOMSTUDY_HH

#include "Analysis.hh"
#include "HistStore.hh"
#include "Event.hh"
#include "KaonTrack.hh"

#if 0
/*
 *  _  ____  __                 ____  _             _
 * | |/ /  \/  | ___  _ __ ___ / ___|| |_ _   _  __| |_   _
 * | ' /| |\/| |/ _ \| '_ ` _ \\___ \| __| | | |/ _` | | | |
 * | . \| |  | | (_) | | | | | |___) | |_| |_| | (_| | |_| |
 * |_|\_\_|  |_|\___/|_| |_| |_|____/ \__|\__,_|\__,_|\__, |
 *                                                    |___/
 *
 */

#endif
namespace fn
{
    class KMomStudy : public Analysis
    {
        public:
            KMomStudy( const Selection& sel,
                    TFile& tf, std::string folder,
                    const fne::Event * e, bool mc);

            void end_processing();

        private:
            void process_event();

            TH1D * h_px;
            TH1D * h_py;
            TH1D * h_pz;

            TH1D * h_dxdz;
            TH1D * h_dydz;

            TH1D * h_p;

            TH1D * h_x;
            TH1D * h_y;

            TH1D * h_wgt;

            TH1D * hmc_px;
            TH1D * hmc_py;
            TH1D * hmc_pz;
            TH1D * hmc_p;
            TH1D * hmc_dxdz;
            TH1D * hmc_dydz;
            TH1D * hmc_x;
            TH1D * hmc_y;

            HistStore hs_;

            TFile& tfile_;
            std::string folder_;
            const fne::Event * e_;
            KaonTrack kt_;

            REG_DEC_SUB( KMomStudy );
    };


    template<>
        Subscriber * create_subscriber<KMomStudy>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
