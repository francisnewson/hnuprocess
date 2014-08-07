#ifndef K2PICLUSTERS_HH
#define K2PICLUSTERS_HH
#include "Event.hh"
#include "RecoCluster.hh"
#include "Subscriber.hh"
#include "SingleTrack.hh"
#include "ClusterEnergyCorr.hh"
#if 0
/*
 *  _  ______        _
 * | |/ /___ \ _ __ (_)
 * | ' /  __) | '_ \| |
 * | . \ / __/| |_) | |
 * |_|\_\_____| .__/|_|
 *   ____ _   |_|     _           
 *  / ___| |_   _ ___| |_ ___ _ __
 * | |   | | | | / __| __/ _ \ '__\
 * | |___| | |_| \__ \ ||  __/ |  
 *  \____|_|\__,_|___/\__\___|_|  
 *
 *
 */
#endif
namespace fn
{
    /*****************************************************
     * K2PI Clusters
     *
     * Select clusters compatible with K2pi
     *
     *****************************************************/
    struct UnknownK2piClustersMethod{};

    class K2piRecoClusters
    {
        fne::RecoCluster& cluster1() const;
        fne::RecoCluster& cluster2() const;

        bool found_charged_cluster() const;
        fne::RecoCluster& charged_cluster() const;
    };

    class K2piClusters : public Subscriber
    {
        public: 
            void new_event();
            bool found_clusters() const;
            const K2piRecoClusters& get_reco_clusters() const;

        protected:
            void set_reco_clusters( K2piRecoClusters * krc );

        private:
            virtual bool process_clusters() const =0;
            const K2piRecoClusters * reco_clusters_;

            //Results cacheing
            mutable bool dirty_;
            mutable bool found_;

            REG_DEC_SUB( K2piClusters );
    };

    template<>
        Subscriber * create_subscriber<K2piClusters>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------


    class DefaultK2piRecoClusters : public K2piRecoClusters
    {
        public:
            void update( fne::RecoCluster * c1, fne::RecoCluster * c2,
                    bool found_track, fne::RecoCluster * tc );

            const fne::RecoCluster& cluster1() const;
            const fne::RecoCluster& cluster2() const;

            bool found_track_cluster() const;
            const fne::RecoCluster& track_cluster() const;

        private:
            fne::RecoCluster * c1_;
            fne::RecoCluster * c2_;
            fne::RecoCluster * tc_;
            bool found_track_;
    };

    //--------------------------------------------------

    struct processing_cluster
    {
        double corr_energy;
        fne::RecoCluster *rc;
    };

    class ClusterEnergyCorr;

    class DefaultK2piClusters :  public K2piClusters
    {
        public:
            DefaultK2piClusters( 
                    const fne::Event * event,
                    const SingleTrack * single_track,
                    YAML::Node & instruct );

        private:
            bool process_clusters() const;

            //Reconstruction Objects
            const fne::Event * event_;
            const SingleTrack * st_;

            //Processing objects
            mutable DefaultK2piRecoClusters reco_clusters_;
            mutable std::vector<processing_cluster>
                filtered_clusters_;

            //Processing functions
            void filter_clusters() const;
            bool assign_clusters() const;

            std::vector<processing_cluster>::iterator
                find_track_cluster( 
                        std::vector<processing_cluster> clusters) const;

            bool check_track_cluster
                ( const processing_cluster * pc ) const;

            bool check_photon_clusters
                ( const std::vector<processing_cluster>&  pcs ) const;


            //Corrections
            ClusterEnergyCorr cec_;

            //cluster parameters
            double min_track_cluster_time_;
            double min_cluster_energy_;

            double max_track_cluster_distance_;

    };

}
#endif
