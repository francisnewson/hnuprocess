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

    //The cached object that gets returned
    class K2piRecoClusters
    {
        private:
            bool mc_;
            const ClusterCorrector& cluster_corrector_;

        public:
            K2piRecoClusters( bool mc, const ClusterCorrector& cluster_corrector)
                :mc_( mc ), cluster_corrector_( cluster_corrector) {}
            virtual const fne::RecoCluster& cluster1() const = 0;
            virtual const fne::RecoCluster& cluster2() const = 0;

            virtual bool found_track_cluster() const = 0;
            virtual const fne::RecoCluster& track_cluster() const = 0;


            bool is_mc() const { return mc_; }
            virtual const ClusterCorrector& get_cluster_corrector() const 
            { return cluster_corrector_; }

    };

    //The subscriber object that does the selection
    class K2piClusters : public Subscriber
    {
        public: 
            void new_event();
            bool found_clusters() const;
            const K2piRecoClusters& get_reco_clusters() const;
            virtual const ClusterCorrector& get_cluster_corrector() const = 0;

        protected:
            void set_reco_clusters( K2piRecoClusters * krc ) const;

        private:
            virtual bool process_clusters() const =0;
            mutable const K2piRecoClusters * reco_clusters_;

            //Results cacheing
            mutable bool dirty_;
            mutable bool found_;

            REG_DEC_SUB( K2piClusters );
    };

    template<>
        Subscriber * create_subscriber<K2piClusters>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------


    //Default cache class
    class DefaultK2piRecoClusters : public K2piRecoClusters
    {
        public:
            DefaultK2piRecoClusters( bool mc, const ClusterCorrector& cluster_corrector )
                : K2piRecoClusters( mc, cluster_corrector ) {}
            void update( const fne::RecoCluster * c1, 
                    const fne::RecoCluster * c2,
                    bool found_track, const fne::RecoCluster * tc );

            const fne::RecoCluster& cluster1() const;
            const fne::RecoCluster& cluster2() const;

            bool found_track_cluster() const;
            const fne::RecoCluster& track_cluster() const;

        private:
            const fne::RecoCluster * c1_;
            const fne::RecoCluster * c2_;
            const fne::RecoCluster * tc_;
            bool found_track_;
    };

    //--------------------------------------------------

    struct processing_cluster
    {
        double corr_energy;
        fne::RecoCluster *rc;
        ~processing_cluster(){}
    };

    class ClusterEnergyCorr;

    //Default implementation of selection
    class DefaultK2piClusters :  public K2piClusters
    {
        public:
            DefaultK2piClusters( 
                    const fne::Event * event,
                    const SingleTrack * single_track,
                    YAML::Node & instruct, bool mc ,
                    const ClusterCorrector& cluster_corrector
                    );

            enum failure
            {
                not_enough_clusters,
                not_enough_good_clusters,
                too_many_good_clusters,
            };

            virtual const ClusterCorrector& get_cluster_corrector() const ;

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
                        std::vector<processing_cluster>& clusters) const;

            //Corrections
            const ClusterCorrector& cluster_corrector_;

            //cluster parameters
            double min_track_cluster_time_;
            double min_cluster_energy_;

            double max_track_cluster_distance_;

    };

}
#endif
