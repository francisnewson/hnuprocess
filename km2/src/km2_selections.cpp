#include "km2_selections.hh"
#include "yaml_help.hh"

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

//--------------------------------------------------

    REG_DEF_SUB( Km2M2Miss);

    Km2M2Miss::Km2M2Miss( const Km2Event& km2_event,
            double min_m2, double max_m2)
        :km2_event_( km2_event ), 
        min_m2_( min_m2), max_m2_( max_m2)
    { }

    bool Km2M2Miss::do_check() const
    {
        const Km2RecoEvent & km2re = km2_event_.get_reco_event();
        double m2miss = km2re.get_m2miss();

        return (m2miss > min_m2_) && (m2miss < max_m2_) ;

    }

    template<>
        Subscriber * create_subscriber<Km2M2Miss>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            double min_m2 = 
                get_yaml<double>( instruct, "min_m2" );

            double max_m2 = 
                get_yaml<double>( instruct, "max_m2" );

            const auto * km2_event = get_km2_event( instruct, rf );

            return new Km2M2Miss( *km2_event, min_m2, max_m2 );
        }
}
