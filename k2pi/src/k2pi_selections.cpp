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
}
