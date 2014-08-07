#include "K2piClusters.hh"
#include "ClusterEnergyCorr.hh"
#include "tracking_selections.hh"
#include "Xcept.hh"

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
        ( K2piRecoClusters * krc )
        {
            reco_clusters_ = krc; 
        }

    template<>
        Subscriber * create_subscriber<K2piClusters>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = instruct["method"].as<std::string>();
            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack * single_track = get_single_track( instruct, rf );

            if ( method == "Default" )
            {
                return new DefaultK2piClusters{ event, single_track, instruct };
            }
            else
            {
                throw  Xcept<UnknownK2piClustersMethod>( method );
            }

            return 0;
        }

    //--------------------------------------------------

    DefaultK2piClusters::DefaultK2piClusters( 
            const fne::Event * event,
            const SingleTrack * single_track,
            YAML::Node & instruct )
        :event_( event ), st_( single_track ),
        cec_("/afs/cern.ch/user/f/fnewson/work/hnu"
                "/gopher/data/detector/eopCorrfile.dat" )
    { }

    bool DefaultK2piClusters::process_clusters() const
    {

        //Filter raw clusters
        filtered_clusters_.clear();
        filter_clusters();

        //assign clusters (photon / pi+ )

        return true;

    }
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
        for ( unsigned int iclus = 0 ; iclus != nclusters ; ++iclus )
        {
            pc.rc =  static_cast<fne::RecoCluster*>( eclusters[iclus] );

            //Timing cut
            double track_cluster_time = pc.rc->time - srt.get_time();
            if ( (!mc) && ( track_cluster_time > min_track_cluster_time_ ) )
            {
                continue;
            }

            //Energy cut
            pc.corr_energy = cec_( *pc.rc  );
            if ( pc.corr_energy > min_cluster_corr_energy_ )
            {
                filtered_clusters_.push_back( pc );
            }
        }

    }

}

