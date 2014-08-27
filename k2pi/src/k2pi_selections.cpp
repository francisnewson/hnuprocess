#include "k2pi_selections.hh"
#include "yaml_help.hh"

namespace fn
{

    REG_DEF_SUB( PipM2miss );

    PipM2miss::PipM2miss( const K2piReco& k2pic,
            double min_m2, double max_m2 )
        :k2pic_( k2pic ), min_m2_( min_m2), max_m2_( max_m2)
    {}

    bool PipM2miss::do_check() const
    {
        const K2piRecoEvent &  re = k2pic_.get_reco_event();

        double m2pip = re.get_m2pip();

        return ( ( m2pip > min_m2_ ) && ( m2pip < max_m2_ ) );
    }

    template<>
        Subscriber * create_subscriber<PipM2miss>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const K2piReco * k2pi_reco = get_k2pi_reco( instruct, rf );

            double min_m2 = get_yaml<double>( instruct,  "min_m2");
            double max_m2 = get_yaml<double>( instruct, "max_m2");

            return new PipM2miss( *k2pi_reco, min_m2, max_m2 );
        }

    //--------------------------------------------------

            REG_DEF_SUB( PhotonRadialCut );

    PhotonRadialCut::PhotonRadialCut( const K2piReco& k2pic,
            double z, double minR, double maxR )
        :k2pic_( k2pic), z_( z), min_r_( minR), max_r_( maxR )
    {}

    bool PhotonRadialCut::do_check() const
    {
        const K2piRecoEvent& k2pirc = k2pic_.get_reco_event();
        const ClusterData c1 = k2pirc.get_cluster1();
        const ClusterData c2 = k2pirc.get_cluster2();
        TVector3 vertex = k2pirc.get_vertex();

        return check_cluster( c1, vertex) 
            && check_cluster( c2, vertex);
    }

    bool PhotonRadialCut::check_cluster
        ( const ClusterData& cluster,
          const TVector3& vertex ) const
        {
            TVector3 dir = cluster.position - vertex;
            fn::Track track{ vertex, dir};
            TVector3 radial_pos = track.extrapolate( z_ );

            return (radial_pos.Perp() < max_r_ )
                && ( radial_pos.Perp() > min_r_ );
        }

    template<>
        Subscriber * create_subscriber<PhotonRadialCut>
        (YAML::Node& instruct, RecoFactory& rf )
    {
            const K2piReco * k2pi_reco = get_k2pi_reco( instruct, rf );
            double z = get_yaml<double>( instruct, "z" );
            double min_r = get_yaml<double>( instruct, "min_r" );
            double max_r = get_yaml<double>( instruct, "max_r" );

            return new PhotonRadialCut( *k2pi_reco, z, min_r, max_r );
    }

    //--------------------------------------------------
}
