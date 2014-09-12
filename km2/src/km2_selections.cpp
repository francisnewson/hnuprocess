#include "km2_selections.hh"

namespace fn
{

    REG_DEF_SUB( Km2NoBadCluster );

    Km2NoBadCluster::Km2NoBadCluster( const Km2Clusters& km2c )
        :km2c_( km2c ){}

    bool Km2NoBadCluster::do_check() const
    {
        const Km2RecoClusters& km2rc  = km2c_.get_reco_clusters();
        return ( km2rc.bad_size() < 1 );
    }

    template<>
        Subscriber * create_subscriber<Km2NoBadCluster>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Km2Clusters* km2c  = get_km2_clusters( instruct, rf );
            return new Km2NoBadCluster( *km2c );
        }
}


