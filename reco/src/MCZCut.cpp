#include "MCZCut.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( MCZCut );

    MCZCut::MCZCut ( const fne::Event * e, bool mc,  double min_z, double max_z )
        :e_( e ), mc_( mc ), min_z_( min_z ), max_z_( max_z ) {}

    bool MCZCut::do_check() const
    {
        if ( !mc_ ){ return false ; }

        double z = e_->mc.decay.decay_vertex.Z();
        return ( z > min_z_ && z < max_z_ );
    }

    template<>
        Subscriber * create_subscriber<MCZCut>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();
            bool is_mc = rf.is_mc();

            double min_z = get_yaml<double>( instruct, "min_z" );
            double max_z = get_yaml<double>( instruct, "max_z" );

            return new MCZCut( event, is_mc, min_z, max_z );
        }
}
