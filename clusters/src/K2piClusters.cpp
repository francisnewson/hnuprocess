#include "K2piClusters.hh"
#include "ClusterEnergyCorr.hh"
#include "tracking_selections.hh"
#include "CorrCluster.hh"
#include "Xcept.hh"
#include "yaml_help.hh"
#include "root_help.hh"
#include <iomanip>

namespace fn
{
    REG_DEF_SUB( K2piClusters );


    //K2piClusters CACHEING LOGIC
    void K2piClusters::new_event()
    {
        dirty_ = true;
    }

    bool K2piClusters::found_clusters() const
    {
        if ( dirty_ )
        {
            found_ = process_clusters();
            dirty_ = false;
        }
        assert( !dirty_ );
        return found_;
    }


    const K2piRecoClusters& 
        K2piClusters::get_reco_clusters() const
        {
            if ( dirty_ )
            {
                found_ =   process_clusters();
                dirty_ = false;
            }
            if ( !found_ )
            {
                throw Xcept<EventDoesNotContain>( "K2piClusters");
            }
            assert( !dirty_ );
            assert( found_);
            return *reco_clusters_;
        }

    void K2piClusters::set_reco_clusters 
        ( K2piRecoClusters * krc ) const
        {
            reco_clusters_ = krc; 
        }

    template<>
        Subscriber * create_subscriber<K2piClusters>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = get_yaml<std::string>(instruct, "method");

            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack * single_track = get_single_track( instruct, rf );
            const ClusterCorrector * cluster_corrector = get_cluster_corrector( instruct, rf );

            bool mc = rf.is_mc();

            if ( method == "Default" )
            {
                return new DefaultK2piClusters{ event, single_track,
                    instruct, mc, *cluster_corrector };
            }
            else
            {
                throw  Xcept<UnknownK2piClustersMethod>( method );
            }

            return 0;
        }

    //--------------------------------------------------

    void DefaultK2piRecoClusters::update
        ( const fne::RecoCluster * c1, const fne::RecoCluster * c2,
          bool found_track, const fne::RecoCluster * tc )
        {
            c1_ = c1;
            c2_ = c2;
            found_track_ = found_track;
            tc_ = tc;
        }

    const fne::RecoCluster& 
        DefaultK2piRecoClusters::cluster1() const
        { return *c1_; }

    const fne::RecoCluster& 
        DefaultK2piRecoClusters::cluster2() const
        { return *c2_; }

    bool 
        DefaultK2piRecoClusters::found_track_cluster() const
        { return found_track_; }

    const fne::RecoCluster& 
        DefaultK2piRecoClusters::track_cluster() const
        {
            if ( !found_track_ )
            {
                throw Xcept<EventDoesNotContain>(
                        "K2PiRecoClusters: track_cluster" );
            }
            return *tc_; 
        }

    //--------------------------------------------------

    DefaultK2piClusters::DefaultK2piClusters( 
            const fne::Event * event,
            const SingleTrack * single_track,
            YAML::Node & instruct, bool mc, const ClusterCorrector& cluster_corrector )
        :event_( event ), st_( single_track ),reco_clusters_( mc , cluster_corrector),
        cluster_corrector_( cluster_corrector)

    { 
        min_track_cluster_time_  = 
            get_yaml<double>( instruct, "min_track_cluster_time");

        min_cluster_energy_  = 
            get_yaml<double>( instruct, "min_cluster_energy");

        max_track_cluster_distance_  = 
            get_yaml<double>( instruct, "max_track_cluster_distance");
    }

    const ClusterCorrector& DefaultK2piClusters::get_cluster_corrector() const
    { return cluster_corrector_; }

    //Process clusters
    bool DefaultK2piClusters::process_clusters() const
    {

        //Filter raw clusters
        filtered_clusters_.clear();
        assert( filtered_clusters_.size() == 0 );

        filter_clusters();

        //assign clusters (photon / pi+ )
        bool success = assign_clusters();

        set_reco_clusters( &reco_clusters_ );

        return success;
    }

    //Filter clusters
    void DefaultK2piClusters::filter_clusters() const
    {
        //Need a track for timing
        const SingleRecoTrack & srt = st_->get_single_track();

        //Is this MC or data?
        bool mc = event_->header.mc;

        //Extract clusters from event
        int nclusters = event_->detector.nclusters;
        auto& eclusters = event_->detector.clusters;
        processing_cluster pc;

        //Loop over clusters - Ignore noise-like clusters
        for (  int iclus = 0 ; iclus != nclusters ; ++iclus )
        {
            pc.rc = 0;


            pc.rc =  static_cast<fne::RecoCluster*>( eclusters[iclus] );
            assert( pc.rc != 0 );

            BOOST_LOG_SEV( get_log(), log_level() )
                << pc.rc->energy;

            //Timing cut
            double track_cluster_time = pc.rc->time - srt.get_time();
            if ( (!mc) && ( track_cluster_time > min_track_cluster_time_ ) )
            {
                continue;
            }

            //Energy cut
            CorrCluster cc{ *pc.rc, cluster_corrector_,  reco_clusters_.is_mc() };
            pc.corr_energy = cc.get_energy();
            if ( pc.corr_energy > min_cluster_energy_ )
            {
                filtered_clusters_.push_back( pc );
            }
        }
    }

    //Assign clusters
    bool DefaultK2piClusters::assign_clusters() const
    {
        int nclus = filtered_clusters_.size();

        if ( ! ( (nclus ==3 ) || (nclus == 2 ) ) )
        {
            return false;
        }

        assert( nclus > 1 );
        assert( nclus < 4 );

        //--

        bool found_track = false;
        const fne::RecoCluster * track_cluster = 0;

        if ( nclus == 3)
        {
            BOOST_LOG_SEV( get_log(), log_level() )
                << "Found 3 clusters!" ;

            BOOST_LOG_SEV( get_log(), log_level() )
                << "Need d < " <<  max_track_cluster_distance_; 

            //one cluster should be from the track
            auto track_cluster_it = find_track_cluster( filtered_clusters_ );
            if ( track_cluster_it == std::end( filtered_clusters_ ) )
            {
                return false;
            }

            BOOST_LOG_SEV( get_log(), log_level() )
                << "Found track cluster!" ;

            found_track = true;
            assert( track_cluster_it->rc != 0 );
            track_cluster = track_cluster_it->rc;
            filtered_clusters_.erase(  track_cluster_it  );
        }

        assert( filtered_clusters_.size() == 2 );

        //--

        //sort by energy
        if ( filtered_clusters_[0].corr_energy > filtered_clusters_[1].corr_energy )
        {
            using std::swap;
            swap( filtered_clusters_[0], filtered_clusters_[1] );
        }

        BOOST_LOG_SEV( get_log(), log_level() )
            << filtered_clusters_.size() ;

        BOOST_LOG_SEV( get_log(), log_level() )
            << std::setw(15) << filtered_clusters_[0].rc ;

        BOOST_LOG_SEV( get_log(), log_level() )
            << std::setw(15) << filtered_clusters_[1].rc ;

        BOOST_LOG_SEV( get_log(), log_level() )
            << std::setw(15) << found_track;

        if ( found_track )
        {
            BOOST_LOG_SEV( get_log(), log_level() )
                << std::setw(15) << track_cluster ;
        }

        reco_clusters_.update( filtered_clusters_[0].rc, filtered_clusters_[1].rc,
                found_track, track_cluster  );

        return true;
    }

    //find track cluster
    std::vector<processing_cluster>::iterator
        DefaultK2piClusters::find_track_cluster( 
                std::vector<processing_cluster>& clusters) const
        {
            //Get single track
            const SingleRecoTrack & srt = st_->get_single_track();

            //collect distances
            std::vector<double> track_cluster_dists;
            for ( auto& cluster : clusters )
            {
                CorrCluster cc( *cluster.rc, cluster_corrector_, reco_clusters_.is_mc() );
                TrackProjCorrCluster tpcc( cc );
                TVector3 pos = tpcc.get_pos();
                TVector3 track_pos = srt.extrapolate_ds( pos.Z() );
                double dist = fabs ( ( track_pos - pos).Mag() );

                std::stringstream ss; 
                ss << pos << " " << track_pos << " " << dist;

                BOOST_LOG_SEV( get_log(), log_level() )
                    <<  ss.str() ;

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

