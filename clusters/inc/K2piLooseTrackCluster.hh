#ifndef K2PILOOSETRACKCLUSTER_HH
#define K2PILOOSETRACKCLUSTER_HH
#include "K2piClusters.hh"
#include <vector>
#include "TClonesArray.h"

#if 0
/*
 *  _  ______        _ _
 * | |/ /___ \ _ __ (_) |    ___   ___  ___  ___
 * | ' /  __) | '_ \| | |   / _ \ / _ \/ __|/ _ \
 * | . \ / __/| |_) | | |__| (_) | (_) \__ \  __/
 * |_|\_\_____| .__/|_|_____\___/ \___/|___/\___|
 *            |_|
 *  _____               _     ____ _           _
 * |_   _| __ __ _  ___| | __/ ___| |_   _ ___| |_ ___ _ __
 *   | || '__/ _` |/ __| |/ / |   | | | | / __| __/ _ \ '__|
 *   | || | | (_| | (__|   <| |___| | |_| \__ \ ||  __/ |
 *   |_||_|  \__,_|\___|_|\_\\____|_|\__,_|___/\__\___|_|
 *
 *
 */
#endif

namespace fn
{
    //An alternative K2piClusters implementation with 
    //logic that allows track clusters of any energy
    class K2piLooseTrackCluster : public K2piClusters
    {
        public:
            K2piLooseTrackCluster( 
                    const fne::Event * event,
                    const SingleTrack * single_track,
                    YAML::Node & instruct, bool mc ,
                    const ClusterCorrector& cluster_corrector
                    );

            virtual const ClusterCorrector& get_cluster_corrector() const;


        private:
            bool process_clusters() const;
            mutable DefaultK2piRecoClusters reco_clusters_;

            //Reconstruction Objects
            const fne::Event * event_;
            const SingleTrack * st_;

            //Corrections
            const ClusterCorrector& cluster_corrector_;

            //cluster parameters
            double max_track_cluster_time_;
            double min_cluster_energy_;
            double max_track_cluster_distance_;
    };

    //Copy the fne::cluster's in to a vector of processing_clusters's
    std::vector<processing_cluster> extract_fne_clusters
        ( int nclusters, const TClonesArray& eclusters, bool mc,
          const ClusterCorrector& cluster_corrector );

    //Copy only the intime clusters into a new vector
    std::vector<processing_cluster> get_intime_clusters
        ( const std::vector<processing_cluster> & all_clusters, 
          const SingleRecoTrack& srt, 
          double max_dt, bool mc );

    //Sort by energy and return threshold point
    std::vector<processing_cluster>::iterator sort_energy
        ( std::vector<processing_cluster>& clusters, double min_cluster_energy);

    //Find closest track cluster
    std::vector<processing_cluster>::iterator find_track_cluster
        ( std::vector<processing_cluster>& clusters, const SingleRecoTrack& srt,
          double max_track_cluster_distance_, bool mc,
          const ClusterCorrector& cluster_corrector );


}
#endif
