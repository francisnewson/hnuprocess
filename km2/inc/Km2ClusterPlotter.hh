#ifndef KM2_HH
#define KM2_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "Km2Clusters.hh"
#if 0
/*
 *  _  __          ____
 * | |/ /_ __ ___ |___ \
 * | ' /| '_ ` _ \  __) |
 * | . \| | | | | |/ __/
 * |_|\_\_| |_| |_|_____|
 *   ____ _           _            ____  _       _   _
 *  / ___| |_   _ ___| |_ ___ _ __|  _ \| | ___ | |_| |_ ___ _ __
 * | |   | | | | / __| __/ _ \ '__| |_) | |/ _ \| __| __/ _ \ '__|
 * | |___| | |_| \__ \ ||  __/ |  |  __/| | (_) | |_| ||  __/ |
 *  \____|_|\__,_|___/\__\___|_|  |_|   |_|\___/ \__|\__\___|_|
 *
 */
#endif

class TFile;

namespace fn
{
    class Km2Event;
    class Km2RecoEvent;
    class Km2Clusters;
    class SingleRecoTrack;
    class Km2RecoClusters;
    class ClusterCorrector;

    class ClusterHists
    {
        public:
        void init( HistStore& hs, std::string prefix, std::string name);

        void fill( Km2RecoClusters::const_iterator begin, 
                Km2RecoClusters::const_iterator end,
                double track_momentum,  double m2m, double track_time,
                const SingleRecoTrack& srt, const TVector3& vertex,
                const ClusterCorrector& cluster_corrector,
                bool mc, double weight );

        private:
        TH1D * h_n;
        TH1D * h_E;
        TH1D * h_t;
        TH1D * h_gt;
        TH1D * h_ds;
        TH2D * h_xy;
        TH2D * h_Ep;
        TH2D * h_Em2m;
        TH2D * h_pr;
    };

    class Km2ClusterPlots
    {
        public:
            Km2ClusterPlots(bool mc, const ClusterCorrector& cluster_corrector);
            void Fill( const SingleRecoTrack& srt, const Km2RecoEvent& km2re,
                    const Km2RecoClusters& km2rc,  const Km2Clusters& km2_clusters,
                    double weight );
            void Write();

        private:
            bool mc_;
            const ClusterCorrector& cluster_corrector_;

            ClusterHists all_clusters_;
            ClusterHists bad_clusters_;
            ClusterHists ign_clusters_;
            ClusterHists ass_clusters_;
            ClusterHists not_ass_clusters_;

            TH1D * h_eop_;
            HistStore hs_;

    };

    //--------------------------------------------------

    class Km2ClusterPlotter : public Analysis
    {
        public:
            Km2ClusterPlotter( const Selection& sel, 
                    TFile& tfile, std::string folder,
                    const Km2Event& km2_event,
                    const  Km2Clusters& km2_clusters,
                    const ClusterCorrector& cluster_corrector,
                    bool mc);

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const Km2Event& km2_event_;
            const Km2Clusters& km2_clusters_;

            Km2ClusterPlots km2_cluster_plots_;

            REG_DEC_SUB( Km2ClusterPlotter);
    };


    template<>
        Subscriber * create_subscriber<Km2ClusterPlotter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
