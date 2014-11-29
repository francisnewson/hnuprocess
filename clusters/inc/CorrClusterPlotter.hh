#ifndef CORRCLUSTERPLOTTER_HH
#define CORRCLUSTERPLOTTER_HH
#include "ClusterEnergyCorr.hh"
#include "Analysis.hh"
#include "HistStore.hh"

#if 0
/*
 *   ____                 ____ _           _
 *  / ___|___  _ __ _ __ / ___| |_   _ ___| |_ ___ _ __
 * | |   / _ \| '__| '__| |   | | | | / __| __/ _ \ '__|
 * | |__| (_) | |  | |  | |___| | |_| \__ \ ||  __/ |
 *  \____\___/|_|  |_|   \____|_|\__,_|___/\__\___|_|
 *  ____  _       _   _
 * |  _ \| | ___ | |_| |_ ___ _ __
 * | |_) | |/ _ \| __| __/ _ \ '__|
 * |  __/| | (_) | |_| ||  __/ |
 * |_|   |_|\___/ \__|\__\___|_|
 *
 */
#endif

class TFile;

namespace fn
{
    class ClusterCorrector;
    class CorrCluster;

    class CorrClusterPlots
    {
        public:
            CorrClusterPlots();
            void Fill( const CorrCluster & c , double weight);
            void Write();

        private:
            HistStore hs_;
            TH1D * h_dE;
            TH1D * h_E;
            TH2D * h_dEvsE;
            TH2D * h_dXY;
    };

    //--------------------------------------------------

    class CorrClusterPlotter : public Analysis
    {
        public:

            CorrClusterPlotter( const Selection& sel, const fne::Event * e,
                    const ClusterCorrector& cluster_corrector, 
                    TFile& tf, std::string folder );

            void end_processing();

        private:
            void process_event();
            const fne::Event * e_;

            const ClusterCorrector& cluster_corrector_;
            TFile& tf_;
            std::string folder_;
    
            CorrClusterPlots cluster_plots_;

            REG_DEC_SUB( CorrClusterPlotter );
    };


    template<>
        Subscriber * create_subscriber<CorrClusterPlotter>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
