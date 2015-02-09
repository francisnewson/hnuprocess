#include "Filter.hh"
#include "TTree.h"
#include "TFile.h"
#include "Event.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( Filter );

    Filter::Filter( const Selection& sel, TFile& tf,  const fne::Event * e)
        :Analysis( sel ), tf_(tf), e_( e )
    {

        ttree_ = std::unique_ptr<TTree>{
            new TTree( "T", "T") };

        TTree::SetMaxTreeSize( 10000000000LL );
        ttree_->Branch( "event", "fne::Event",
                &e_ , 64000, 2 );
    }

    void Filter::process_event()
    {
        ttree_->Fill();
    }


    void Filter::end_processing()
    {
        tf_.cd();
        ttree_->Write();
    }

    template<>
        Subscriber * create_subscriber<Filter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const fne::Event * e = rf.get_event_ptr();

            return new Filter( *sel, tfile, e );
        }
}
