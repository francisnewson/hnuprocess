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
            fne::RecoCluster& cluster1() const;
            fne::RecoCluster& cluster2() const;

            bool found_charged_cluster() const;
            fne::RecoCluster& charged_cluster() const;

        private:
            fne::RecoCluster * c1_;
            fne::RecoCluster * c2_;
            fne::RecoCluster * cc_;
            bool found_charged_;
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

            //Corrections
            ClusterEnergyCorr cec_;

            //cluster parameters
            double min_track_cluster_time_;
            double min_cluster_corr_energy_;

    };

}
#endif
