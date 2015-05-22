#ifndef KM2K2PI_HH
#define KM2K2PI_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "Km2Clusters.hh"
#if 0
/*
 *  _  __          ____  _  ______        _
 * | |/ /_ __ ___ |___ \| |/ /___ \ _ __ (_)
 * | ' /| '_ ` _ \  __) | ' /  __) | '_ \| |
 * | . \| | | | | |/ __/| . \ / __/| |_) | |
 * |_|\_\_| |_| |_|_____|_|\_\_____| .__/|_|
 *                                 |_|
 *
 */
#endif

class TH2D;

namespace fn
{
    class Km2Event;
    class Km2RecoEvent;
    class Km2Clusters;
    class ClusterCorrector;
}


namespace fne
{
    class Mc;
    class Event;
}

namespace fn
{
    double get_z_pipe( Track& t, double rpipe );

    struct photon_properties
    {
        double E;
        double ds;
        double r;
        double z;
        double t;
        double phi;
    };

    class Km2K2piClusters
    {
        public:
            Km2K2piClusters( std::string prefix, std::string name, HistStore& hs_, 
                    const ClusterCorrector& cluster_corrector, bool mc );
            void fill ( const Km2RecoEvent& km2re, Km2RecoClusters::const_iterator begin,
                    Km2RecoClusters::const_iterator end, double wgt );

        private:
            const ClusterCorrector& cluster_corrector_;
            bool mc_;
            //Histograms
            TH2D * h_cluster_t_z_;
            TH1D * h_cluster_r_;
            TH1D * h_cluster_E_;
            TH2D * h_cluster_r_E_;
            TH2D * h_cluster_ds_E_;

            TH2D * h_cluster_zpipe_t_;

            TH2D * h_cluster_ds_ds_;
            TH2D * h_cluster_E_E_;
            TH2D * h_cluster_r_r_;
    };


    class Km2K2pi : public Analysis
    {
        public:
            Km2K2pi( const Selection& sel, bool mc,
                    const Km2Event& km2e, const fne::Event * e,
                    const  Km2Clusters& km2_clusters,
                    const ClusterCorrector& cluster_corrector,
                    TFile& tfile, std::string folder );

            void end_processing();

        private:
            void process_event();
            void plot_mc( const fne::Event& e, double wgt );


            //Histograms
            TH2D * h_mc_photon_t_z_;
            TH1D * h_mc_photon_r_;
            TH2D * h_mc_photon_r_E_;
            TH2D * h_mc_photon_ds_E_;
            TH2D * h_mc_photon_E_vs_E_;
            TH2D * h_mc_photon_r_vs_r_;
            TH2D * h_mc_photon_phi_vs_phi_;
            TH1D * h_mc_photon_dphi_;
            TH2D * h_mc_photon_dphi_p_;
            TH2D * h_mc_photon_ds_vs_ds_;
            TH1D * h_mc_photon_n_;

            bool mc_;

            const Km2Event & km2e_;
            const fne::Event * e_;

            const Km2Clusters& km2_clusters_;
            const ClusterCorrector& cluster_corrector_;


            TFile& tfile_;
            std::string folder_;

            HistStore hs_;

            Km2K2piClusters bad_clusters_;
            Km2K2piClusters all_clusters_;

            REG_DEC_SUB( Km2K2pi);
    };

    template<>
        Subscriber * create_subscriber<Km2K2pi>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
