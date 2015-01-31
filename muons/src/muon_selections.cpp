#include "muon_selections.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "NA62Constants.hh"


namespace fn
{
    REG_DEF_SUB( MuonReqStatus );

    MuonReqStatus::MuonReqStatus( const MuonVeto& muv,
            std::set<int> allowed_status )
        :muv_( muv ), allowed_status_( allowed_status )
    {}

    bool MuonReqStatus::do_check() const
    {
        int status = muv_.get_muv_status();
        return allowed_status_.find( status )!= allowed_status_.end() ;
    }

    template<>
        Subscriber * create_subscriber<MuonReqStatus>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const MuonVeto*  muv = get_muon_veto( instruct, rf );
            std::vector<int> allowed_status_list = 
                get_yaml<std::vector<int>>( instruct, "allowed_status" );

            std::set<int> allowed_status
                ( begin( allowed_status_list ), end( allowed_status_list ) );

            return new MuonReqStatus( *muv, allowed_status );
        }

    //--------------------------------------------------

    REG_DEF_SUB( NoMuv );

    NoMuv::NoMuv( const fne::Event * e )
        :e_( e ){}

    bool NoMuv::do_check() const
    {
        return (e_->detector.nmuons == 0 );
    }

    template<>
        Subscriber * create_subscriber<NoMuv>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            (void ) instruct;

            const fne::Event *  event = rf.get_event_ptr();

            return new NoMuv( event );
        }

    //--------------------------------------------------
}
