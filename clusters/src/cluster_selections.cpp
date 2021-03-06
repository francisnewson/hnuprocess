#include "cluster_selections.hh"
#include "tracking_selections.hh"
#include "CorrCluster.hh"
#include "SingleTrack.hh"
#include "Xcept.hh"
#include "yaml_help.hh"

namespace fn
{
    K2piClusters * get_k2pi_clusters
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            YAML::Node yk2pic = instruct["inputs"]["k2pic"];

            if ( !yk2pic )
            {throw Xcept<MissingNode>( "k2pic" );}

            K2piClusters * k2pic = dynamic_cast<K2piClusters*>
                ( rf.get_subscriber( yk2pic.as<std::string>() ));

            if ( !k2pic )
            { throw Xcept<BadCast>( "K2PIC" ); }

            return k2pic;

        }

    //--------------------------------------------------

    REG_DEF_SUB( FoundK2piClusters);

    FoundK2piClusters::FoundK2piClusters( const K2piClusters& k2pic )
        :k2pic_( k2pic ) {}

    bool FoundK2piClusters::do_check() const
    {
        return k2pic_.found_clusters();
    }

    template<>
        Subscriber * create_subscriber<FoundK2piClusters>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K2piClusters * k2pic = get_k2pi_clusters( instruct, rf );
            return new FoundK2piClusters{ *k2pic };
        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackClusterEP);

    TrackClusterEP::TrackClusterEP( const K2piClusters& k2pic ,
            const SingleTrack& st , double min_eop, double max_eop)
        :k2pic_( k2pic), st_( st ), min_eop_( min_eop),max_eop_( max_eop)
    {}

    bool TrackClusterEP::do_check() const
    {
        const K2piRecoClusters &  k2pirc = k2pic_.get_reco_clusters();

        //If there is no track cluster we don't need to check
        if ( ! k2pirc.found_track_cluster() )
        {
            return true;
        }
        else
        {
            const fne::RecoCluster tc = k2pirc.track_cluster();
            CorrCluster cc( tc, k2pirc.get_cluster_corrector(), k2pirc.is_mc() );
            TrackProjCorrCluster tcep( cc );
            double track_cluster_energy = tcep.get_energy();

            const SingleRecoTrack& srt = st_.get_single_track();
            double track_momentum = srt.get_mom();

            double eop = track_cluster_energy / track_momentum;

            BOOST_LOG_SEV( get_log(), log_level() )
                << "EOP:  e "<< track_cluster_energy
                << " p " << track_momentum;

            return ( ( eop < max_eop_ ) && ( eop > min_eop_ ) );
        }
    }

    template<>
        Subscriber * create_subscriber<TrackClusterEP>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K2piClusters * k2pic = get_k2pi_clusters( instruct, rf );
            SingleTrack * st = get_single_track( instruct, rf );

            double min_eop = get_yaml<double>( instruct, "min_eop");
            double max_eop = get_yaml<double>( instruct, "max_eop");


            return new TrackClusterEP( *k2pic, *st, min_eop, max_eop );

        }

    //--------------------------------------------------

    REG_DEF_SUB( PhotonSeparation );

    PhotonSeparation::PhotonSeparation
        (const K2piClusters& k2pic, double min_sep )
        :k2pic_( k2pic), min_sep_( min_sep)
        {}

    bool PhotonSeparation::do_check() const
    {
        const K2piRecoClusters &  k2pirc = k2pic_.get_reco_clusters();

        CorrCluster c1 { k2pirc.cluster1(), k2pirc.get_cluster_corrector(), k2pirc.is_mc() };
        CorrCluster c2 { k2pirc.cluster2(), k2pirc.get_cluster_corrector(), k2pirc.is_mc()};

        double sep = fabs( ( c1.get_pos() - c2.get_pos() ).Mag() );
        return  (sep > min_sep_ );
    }

    template<>
        Subscriber * create_subscriber<PhotonSeparation>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            K2piClusters * k2pic = get_k2pi_clusters( instruct, rf );
            double min_sep = get_yaml<double>( instruct, "min_sep");

            return new PhotonSeparation( *k2pic, min_sep);
        }

    //--------------------------------------------------

    REG_DEF_SUB( PhotonTrackSeparation );

    PhotonTrackSeparation::PhotonTrackSeparation
        ( const K2piClusters& k2pic, const SingleTrack& st, 
          double min_sep)
        :k2pic_( k2pic), st_( st ), min_sep_( min_sep )
        {}

    bool PhotonTrackSeparation::do_check() const
    {
        //Extract clusters
        const K2piRecoClusters &  k2pirc = k2pic_.get_reco_clusters();
        CorrCluster c1 { k2pirc.cluster1(), k2pirc.get_cluster_corrector(), k2pirc.is_mc()  };
        CorrCluster c2 { k2pirc.cluster2(), k2pirc.get_cluster_corrector(), k2pirc.is_mc()  };
        TVector3 pos1 = c1.get_pos();
        TVector3 pos2 = c2.get_pos();

        //Extract track at Lkr cluster Z
        const SingleRecoTrack& srt = st_.get_single_track();
        TVector3 track1 = srt.extrapolate_ds( pos1.Z() );
        TVector3 track2 = srt.extrapolate_ds( pos2.Z() );

        double sep1 = (track1 - pos1 ).Mag();
        double sep2 = (track2 - pos2 ).Mag();

        return ( ( sep1 > min_sep_ ) && ( sep2 > min_sep_ ) );
    }

    template<>
        Subscriber * create_subscriber<PhotonTrackSeparation>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const K2piClusters * k2pic = get_k2pi_clusters( instruct, rf );
            const SingleTrack * st = get_single_track( instruct, rf );
            double min_sep = get_yaml<double>( instruct, "min_sep");

            return new PhotonTrackSeparation( *k2pic, *st, min_sep );
        }
}
