#include "Km2Clusters.hh"
#include "NA62Constants.hh"
#include "yaml_help.hh"
#include "tracking_selections.hh"

namespace fn
{
    void Km2RecoClusters::reset()
    {
        bad_clusters_.clear();
        associate_clusters_.clear();
        ignored_clusters_.clear();
    }

    void Km2RecoClusters::add_cluster( cluster_type ct, const fne::RecoCluster * cd )
    {
        switch ( ct )
        {
            case cluster_type::IGN:
                ignored_clusters_.push_back( cd );
                break;

            case cluster_type::BAD:
                bad_clusters_.push_back( cd );
                break;

            case cluster_type::ASS:
                associate_clusters_.push_back( cd );
                break;
        }
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Clusters );

    Km2Clusters::Km2Clusters( const fne::Event* e, const SingleTrack& st ,
            double noise_energy, double noise_time, 
            double brehm_radius, double track_cluster_radius )
        :e_ ( e ), st_( st ),
        noise_energy_( noise_energy ), noise_time_( noise_time),
        brehm_radius_( brehm_radius), track_cluster_radius_( track_cluster_radius )
    {}

    void Km2Clusters::new_event()
    {
        dirty_ = true;
    }

    const Km2RecoClusters& Km2Clusters::get_reco_clusters() const
    { 
        if ( dirty_ )
        {
            process_clusters();
            dirty_ = false;
        }

        assert( !dirty_ );
        return km2rc_;
    }

    cluster_type Km2Clusters::id_cluster( const fne::RecoCluster * rc ) const
    {
        //Is it noise?
        const SingleRecoTrack& srt = st_.get_single_track();
        double energy = rc->energy;
        if ( energy < noise_energy_ ) 
            return cluster_type::IGN;

        //Is it an accidental
        double track_time = srt.get_time();
        if( !mc_ && ( (track_time - rc->time) > noise_time_ ) )
            return cluster_type::IGN;

        //Is it Brehmsstrahlung
        TVector3 brehm_trkLkr = srt.extrapolate_bf( na62const::zLkr );
        PhotonProjCorrCluster photon_cluster{ *rc };
        TVector3 photon_pos = photon_cluster.get_pos();
        if( (brehm_trkLkr - photon_pos).Mag() < brehm_radius_ )
            return  cluster_type::IGN;

        //Is it associated to the track
        TVector3 trkLkr = srt.extrapolate_ds( na62const::zLkr );
        TrackProjCorrCluster track_cluster{ *rc };
        TVector3 cluster_pos = track_cluster.get_pos();
        if ( (trkLkr - cluster_pos).Mag() < track_cluster_radius_ )
            return cluster_type::ASS;

        //If nothing else, it is a bad cluster
        return cluster_type::BAD;
    }

    void Km2Clusters::process_clusters() const
    {
        km2rc_.reset();

        int nclusters = e_->detector.nclusters;
        auto& eclusters = e_->detector.clusters;

        for (  int iclus = 0 ; iclus != nclusters ; ++iclus )
        {
            fne::RecoCluster*  rc =  
                static_cast<fne::RecoCluster*>( eclusters[iclus] );

            cluster_type ct = id_cluster(  rc );
            km2rc_.add_cluster( ct, rc );
        }
    }

    template<>
        Subscriber * create_subscriber<Km2Clusters>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack * st = get_single_track( instruct, rf );

            double noise_energy = get_yaml<double>( instruct, "noise_energy" );
            double noise_time = get_yaml<double>( instruct, "noise_time" );
            double brehm_radius = get_yaml<double>( instruct, "brehm_radius" );
            double track_cluster_radius = get_yaml<double>( instruct, "track_cluster_radius" );

            return new Km2Clusters( event, *st, noise_energy, noise_time,
                    brehm_radius, track_cluster_radius );
        }

    //--------------------------------------------------

    Km2Clusters * get_km2_clusters
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            YAML::Node yogt = instruct["inputs"]["km2clusters"];

            if ( !yogt )
            {throw Xcept<MissingNode>( "km2clusters" );}

            Km2Clusters * ogt = dynamic_cast<Km2Clusters*>
                ( rf.get_subscriber( yogt.as<std::string>() ));

            if ( !ogt )
            { throw Xcept<BadCast>( "KM2CLUSTERS" ); }

            return ogt;
        }

}
