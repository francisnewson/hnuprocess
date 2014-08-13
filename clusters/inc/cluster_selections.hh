#ifndef CLUSTER_SELECTIONS_HH
#define CLUSTER_SELECTIONS_HH
#include "Selection.hh"
#include "K2piClusters.hh"
#if 0
/*
 *       _           _            
 *   ___| |_   _ ___| |_ ___ _ __ 
 *  / __| | | | / __| __/ _ \ '__|
 * | (__| | |_| \__ \ ||  __/ |   
 *  \___|_|\__,_|___/\__\___|_|   
 *           _           _   _                 
 *  ___  ___| | ___  ___| |_(_) ___  _ __  ___ 
 * / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                                             
 */
#endif
namespace fn
{

    K2piClusters * get_k2pi_clusters
        ( YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------


    class FoundK2piClusters : public CachedSelection
    {
        public: 
            FoundK2piClusters( const K2piClusters& k2pic );

        private:
            bool do_check() const;
            const K2piClusters& k2pic_;

            REG_DEC_SUB( FoundK2piClusters);
    };

    template<>
        Subscriber * create_subscriber<FoundK2piClusters>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackClusterEP : public CachedSelection
    {
        public:
            TrackClusterEP( const K2piClusters& k2pic ,
                    const SingleTrack& st,
                    double min_eop, double max_eop);

        private:
            bool do_check() const;
            const K2piClusters& k2pic_;
            const SingleTrack& st_;

            double min_eop_;
            double max_eop_;

            REG_DEC_SUB( TrackClusterEP);
    };

    template<>
        Subscriber * create_subscriber<TrackClusterEP>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

}
#endif
