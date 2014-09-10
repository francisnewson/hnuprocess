#include "muon_selections.hh"
#include "RecoMuon.hh"


namespace fn
{

    MuonReqStatus::MuonReqStatus( const fne::Event * e ,
            std::set<int> allowed_status )
        :e_( e ), allowed_status_( allowed_status )
    {}

    bool MuonReqStatus::do_check() const
    {
        const TClonesArray& muons = e_->detector.muons;
        Int_t nmuons = e_->detector.nmuons;

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

            Int_t status = rm->status;

            if ( allowed_status_.find( status) 
                    != allowed_status_.end() )
            {
                return true;
            }
        }

        return false;
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