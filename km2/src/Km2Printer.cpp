#include "Km2Printer.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{

    REG_DEF_SUB( Km2Printer );

    Km2Printer::Km2Printer( const Selection& sel,
            const fne::Event * e,
            const Km2Event& km2_event,
            const Km2Clusters& km2_clusters,
            std::ostream& os
            )
        :Analysis( sel ), e_( e ), km2_event_( km2_event),
        km2_clusters_ ( km2_clusters ), os_( os )
    {}

    void Km2Printer::process_event()
    {
        const Km2RecoClusters & km2rc = km2_clusters_.get_reco_clusters();
        os_ << "# Bad clusters: " << km2rc.bad_size() << std::endl;

        if ( km2rc.bad_size() > 0 )
        {
            const fne::RecoCluster * cl =* km2rc.bad_begin();

            auto save_level = km2_clusters_.log_level();

            km2_clusters_.set_log_level( always_print );
            km2_clusters_.id_cluster( cl );
            km2_clusters_.set_log_level( save_level );

            os_ << "energy: " << cl->energy << std::endl;
            os_ << "time: " << cl->time << std::endl;

        }
    }

    template<>
        Subscriber * create_subscriber<Km2Printer>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event  = rf.get_event_ptr();
            const Km2Event * km2_event = get_km2_event( instruct, rf );
            const Km2Clusters * km2_clusters = get_km2_clusters( instruct, rf );

            const Selection * sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            std::string ostream_name =
                get_yaml<std::string>( instruct, "ostream" ) ;
            std::ostream& os = rf.get_ostream( ostream_name);

            return new Km2Printer( *sel , event, * km2_event, *km2_clusters, os );
        }

}
