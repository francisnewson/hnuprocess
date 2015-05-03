#include "MuvBursts.hh"
#include "TFile.h"
#include "Event.hh"
#include "HistStore.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( MuvBursts );

    MuvBursts::MuvBursts( const Selection& sel, const Selection& muon_req,
            const fne::Event * e, TFile& tf, std::string folder )
        :Analysis( sel), tf_( tf), folder_( folder), e_(e), mr_( muon_req )
        {

            ttree_.reset( new TTree( "T", "MUV BURSTS" ) );
            ttree_->Branch( "muv_burst", &mb_, "run/L:burst_time/L:total/D:pass/D" );
        }

    void MuvBursts::process_event()
    {
        double wgt = get_weight();
        bool passed = mr_.check();

        if ( passed )
        {
            wgt *= mr_.get_weight();
        }

        mb_.total += wgt;

        if ( passed )
        {
            mb_.pass += wgt;
        }
    }

    void MuvBursts::new_burst()
    {
        mb_ = muv_burst{ e_->header.run, e_->header.burst_time, 0, 0 };
    }

    void MuvBursts::end_burst()
    {
        ttree_->Fill();
    }

    void MuvBursts::end_processing()
    {
        cd_p( &tf_, folder_ );
        ttree_->Write();
    }

    template<>
        Subscriber * create_subscriber<MuvBursts>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();

            const Selection * sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * mr = rf.get_selection( 
                    get_yaml<std::string>( instruct, "muv_selection" ) );

            TFile & tfile = rf.get_tfile(
                    get_yaml<std::string>( instruct, "tfile" ));

            std::string folder = get_folder ( instruct, rf );

            return new MuvBursts( *sel, *mr, event, tfile, folder );
        }
}
