#include "K2piLooseTrackCluster.hh"
#include "CorrCluster.hh"
#include "yaml_help.hh"
namespace fn
{
    K2piLooseTrackCluster::K2piLooseTrackCluster( 
            const fne::Event * event,

            const SingleTrack * single_track,
            YAML::Node & instruct, bool mc ,
            const ClusterCorrector& cluster_corrector)
        :reco_clusters_( mc , cluster_corrector),
        event_( event ), st_( single_track ),
        cluster_corrector_( cluster_corrector)
    {
        max_track_cluster_time_  = 
            get_yaml<double>( instruct, "max_track_cluster_time");

        min_cluster_energy_  = 
            get_yaml<double>( instruct, "min_cluster_energy");

        max_track_cluster_distance_  = 
            get_yaml<double>( instruct, "max_track_cluster_distance");

        BOOST_LOG_SEV( get_log(), startup )
            << "max_track_cluster_time: " << max_track_cluster_time_ ;
    }

    const ClusterCorrector& K2piLooseTrackCluster::get_cluster_corrector() const
    { return cluster_corrector_; }


    bool K2piLooseTrackCluster::process_clusters() const
    {
        //Track for timing and matching
        const SingleRecoTrack & srt = st_->get_single_track();

        //MC for corrections
        bool mc = event_->header.mc;

        //extract clusters
        int nclusters = event_->detector.nclusters;
        auto& eclusters = event_->detector.clusters;
        auto all_clusters = extract_fne_clusters
            ( nclusters, eclusters, mc, cluster_corrector_ );

        BOOST_LOG_SEV( get_log(), log_level() )
            << "all_clusters: " << all_clusters.size();

        //remove out of time clusters
        auto intime_clusters = get_intime_clusters
            ( all_clusters, srt, max_track_cluster_time_, mc );

        BOOST_LOG_SEV( get_log(), log_level() )
            << "intime_clusters: " << intime_clusters.size();

        //find high energy clusters
        auto first_high_E = sort_energy( intime_clusters, min_cluster_energy_ );
        std::vector<processing_cluster> high_E_clusters{ first_high_E, end( intime_clusters ) };
        std::vector<processing_cluster> low_E_clusters{ begin( intime_clusters ), first_high_E };
        assert( high_E_clusters.size() + low_E_clusters.size() == intime_clusters.size() );

        BOOST_LOG_SEV( get_log(), log_level() )
            << "high_E_clusters: " << high_E_clusters.size();

        int nclus = high_E_clusters.size();

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Nclus: " << nclus ;

        //Must be 2 or 3 high E clusters
        if ( ! ( (nclus ==3 ) || (nclus == 2 ) ) ) { return false; }
        assert( nclus > 1 );
        assert( nclus < 4 );

        const fne::RecoCluster * tc = 0;
        const fne::RecoCluster * c1 = 0;
        const fne::RecoCluster * c2 = 0;

        //3rd cluster must be from track
        if ( nclus == 3 )
        {
            auto it_track_cluster = find_track_cluster( high_E_clusters, srt, 
                    max_track_cluster_distance_, mc, cluster_corrector_ );
            if ( it_track_cluster == high_E_clusters.end() ){ return false; }
            high_E_clusters.erase( it_track_cluster );
            assert( high_E_clusters.size() == 2 );

            tc = it_track_cluster->rc;
        }

        //Order photon clusters by energy
        if ( high_E_clusters[0].corr_energy > high_E_clusters[1].corr_energy )
        {
            using std::swap;
            swap( high_E_clusters[0], high_E_clusters[1] );
        }

        c1 = high_E_clusters[0].rc;
        c2 = high_E_clusters[1].rc;

        //Can look for track cluster in left over clusters
        if ( tc == 0 )
        {
            auto it_track_cluster = find_track_cluster( low_E_clusters, srt,
                    max_track_cluster_distance_, mc, cluster_corrector_ );
            if ( it_track_cluster != low_E_clusters.end() )
            { 
                tc = it_track_cluster->rc;   
            }
        }
        bool found_track = ( tc != 0 );

        //Save reco clusters
        reco_clusters_.update( c1, c2, found_track, tc );
        set_reco_clusters( &reco_clusters_);
        return true;
    }

    //----------

    std::vector<processing_cluster> extract_fne_clusters
        ( int nclusters, const TClonesArray& eclusters, bool mc,
          const ClusterCorrector& cluster_corrector )
        {
            std::vector<processing_cluster> all_clusters;
            processing_cluster pc;

            for (  int iclus = 0 ; iclus != nclusters ; ++iclus )
            {
                pc.rc = 0;
                pc.rc =  static_cast<fne::RecoCluster*>( eclusters[iclus] );
                assert( pc.rc != 0 );
                CorrCluster cc{ *pc.rc, cluster_corrector, mc };
                pc.corr_energy = cc.get_energy();
                all_clusters.push_back( pc );
            }

            return all_clusters;
        }

    //----------

    std::vector<processing_cluster> get_intime_clusters
        ( const std::vector<processing_cluster>& all_clusters, const SingleRecoTrack& srt, double max_dt, bool mc )
        {
            if ( mc ){ return all_clusters; }

            double track_time = srt.get_time();
            std::vector<processing_cluster> result;

            std::copy_if( begin( all_clusters), end( all_clusters ), 
                    std::back_inserter( result ),
                    [track_time, max_dt]( const processing_cluster& pc )
                    { 
                    return (fabs(  pc.rc->time - track_time ) < max_dt ); 
                    } );

            return result;
        }

    //----------

    std::vector<processing_cluster>::iterator sort_energy
        ( std::vector<processing_cluster>& clusters, double min_cluster_energy)
        {
            std::sort( begin(clusters), end(clusters),
                    []( const processing_cluster &a, const processing_cluster &b )
                    { return a.corr_energy  < b.corr_energy; } ); 

            return find_if( begin(clusters), end(clusters),
                    [min_cluster_energy]( const processing_cluster& pc )
                    { return pc.corr_energy > min_cluster_energy; } );
        }

    //----------

    std::vector<processing_cluster>::iterator find_track_cluster
        ( std::vector<processing_cluster>& clusters, const SingleRecoTrack& srt,
          double max_track_cluster_distance_, bool mc,
          const ClusterCorrector& cluster_corrector )
        {
            if (clusters.size() == 0 ) { return end( clusters ); }

            //collect distances
            std::vector<double> track_cluster_dists;
            for ( auto& cluster : clusters )
            {
                CorrCluster cc( *cluster.rc, cluster_corrector, mc );
                TrackProjCorrCluster tpcc( cc );
                TVector3 pos = tpcc.get_pos();
                TVector3 track_pos = srt.extrapolate_ds( pos.Z() );
                double dist = fabs ( ( track_pos - pos).Mag() );
                track_cluster_dists.push_back(  dist );
            }

            //extract min distance
            auto min_distance = std::min_element(
                    std::begin( track_cluster_dists ), 
                    std::end( track_cluster_dists ) );

            //Check we are close enough
            if ( *min_distance < max_track_cluster_distance_ )
            {
                unsigned int index =  std::distance(
                        std::begin( track_cluster_dists), min_distance ) ;

                return std::begin( clusters) + index;
            }
            else
            {
                return std::end( clusters );
            }
        }
}
