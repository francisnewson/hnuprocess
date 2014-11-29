#include "Km2Clusters.hh"
#include "NA62Constants.hh"
#include "yaml_help.hh"
#include "root_help.hh"
#include "tracking_selections.hh"
#include <boost/lexical_cast.hpp>

namespace fn
{
    Km2RecoClusters::Km2RecoClusters( const ClusterCorrector& cluster_corrector)
        :cluster_corrector_( cluster_corrector)
    {}

    void Km2RecoClusters::reset(bool mc)
    {
        mc_ = mc;
        bad_clusters_.clear();
        associate_clusters_.clear();
        ignored_clusters_.clear();
        all_clusters_.clear();
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

        all_clusters_.push_back( cd );
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Clusters );

    Km2Clusters::Km2Clusters( const fne::Event* e, const SingleTrack& st ,
            double noise_energy, double noise_time, 
            double brehm_radius, double track_cluster_radius,
            const ClusterCorrector& cluster_corrector, bool is_mc )
        :e_ ( e ), st_( st ), km2rc_( cluster_corrector),
        noise_energy_( noise_energy ), noise_time_( noise_time),
        brehm_radius_( brehm_radius), track_cluster_radius_( track_cluster_radius ),
         cluster_corrector_( cluster_corrector), mc_( is_mc )
    { }

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

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Energy: " << energy ;

        if ( energy <= noise_energy_ ) 
            return cluster_type::IGN;

        //Is it an accidental
        double track_time = srt.get_time();
        double delta_t = fabs(track_time - rc->time);

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Delta_t: " << delta_t ;

        BOOST_LOG_SEV( get_log(), log_level() )
            << "MC? : " << ( mc_ ? "YES" : "NO" );

        if( (!mc_) && (delta_t >= noise_time_)  )
            {return cluster_type::IGN;}

        CorrCluster  cc{ *rc, cluster_corrector_, mc_ };

        //Is it Brehmsstrahlung
        TVector3 brehm_trkLkr = srt.extrapolate_bf( na62const::zLkr );
        PhotonProjCorrCluster photon_cluster{ cc };
        TVector3 photon_pos = photon_cluster.get_pos();
        double brehm_sep =  (brehm_trkLkr - photon_pos).Mag();

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Brehm_sep: " << brehm_sep ;

        if( brehm_sep  <= brehm_radius_ )
            return  cluster_type::IGN;

        //Is it associated to the track
        TrackProjCorrCluster track_cluster{ cc };
        TVector3 cluster_pos = track_cluster.get_pos();
        TVector3 trkLkr = srt.extrapolate_ds( cluster_pos.Z() );
        double track_cluster_sep = (trkLkr - cluster_pos).Mag();

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Track: " <<  trkLkr.X() << ", " << trkLkr.Y() <<  ", " << trkLkr.Z()
            << " Cluster: " << cluster_pos.X() << ", " << cluster_pos.Y() << ", " << cluster_pos.Z()
            << " Track cluster sep: " << track_cluster_sep ;

        if ( track_cluster_sep  < track_cluster_radius_ )
            return cluster_type::ASS;

        //If nothing else, it is a bad cluster
        return cluster_type::BAD;
    }

    void Km2Clusters::process_clusters() const
    {
        static int counter = 0;
        ++counter;

        auto save_log = log_level();

        if ( counter < 50  ) 
        {
            set_log_level( always_print );
        BOOST_LOG_SEV( get_log(), log_level() )
            <<"--------------------NEW Km2Clusters EVENT--------------------";
        }

        km2rc_.reset( mc_);

        int nclusters = e_->detector.nclusters;
        auto& eclusters = e_->detector.clusters;

        for (  int iclus = 0 ; iclus != nclusters ; ++iclus )
        {
            fne::RecoCluster*  rc =  
                static_cast<fne::RecoCluster*>( eclusters[iclus] );

            cluster_type ct = id_cluster(  rc );

            if ( ct == cluster_type::ASS )
            { 
                BOOST_LOG_SEV( get_log(), log_level() )
                << "Found associated cluster!" ;
            }

            km2rc_.add_cluster( ct, rc );
        }

        BOOST_LOG_SEV( get_log(), log_level() )
            << km2rc_.bad_size() << " bad clusters!";

        set_log_level( save_log );
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

            const ClusterCorrector* cluster_corrector =
                get_cluster_corrector( instruct, rf );

            bool is_mc = rf.is_mc();

            return new Km2Clusters( event, *st, noise_energy, noise_time,
                    brehm_radius, track_cluster_radius, *cluster_corrector, is_mc );
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
