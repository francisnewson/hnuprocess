#include "MuonComp.hh"
#include "SingleTrack.hh"
#include "SingleMuon.hh"
#include <sstream>
#include <iomanip>
#include "tracking_selections.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( MuonComp );

    MuonComp::MuonComp( const Selection& sel, const fne::Event * e, const SingleTrack& st,
             SingleMuon& sm1,  SingleMuon& sm2)
        :Analysis( sel), e_( e ),st_( st), sm1_( sm1),  sm2_( sm2 )
    {
    }

    void MuonComp::process_event()
    {

        if ( sm1_.found() != sm2_.found() )
        {
            BOOST_LOG_SEV( get_log(), always_print )
                << "muon mismatach" ;

        }

            if ( false )
        {
            std::stringstream ss;
            if ( sm1_.found() )
            {
                ss << "[ " << std::setw(10) << sm1_.x() << ", "
                    << std::setw(10) << sm1_.y() << "]" ;
            }
            else
            {
                ss << "[ " << std::setw(10) << " " << ", "
                    << std::setw(10) << " " << "]" ;
            }

            if ( sm2_.found() )
            {
                ss << "[ " << std::setw(10) << sm2_.x() << ", "
                    << std::setw(10) << sm2_.y() << "]" ;
            }
            else
            {
                ss << "[ " << std::setw(10) << " " << ", "
                    << std::setw(10) << " " << "]" ;
            }

            BOOST_LOG_SEV( get_log(), always_print )
                << ss.str() ;

            sm2_.debug_event();
        }
    }


    template<>
        Subscriber * create_subscriber<MuonComp>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const SingleTrack * st = get_single_track( instruct, rf );

            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            SingleMuon * sm1  =
                dynamic_cast<SingleMuon*> ( 
                        rf.get_subscriber( 
                            get_yaml<std::string>( instruct, "muon1" ) ) );

            if ( ! sm1 ){ throw std::runtime_error( "MuonComp: Could not read muon1") ; }

            SingleMuon * sm2  =
                dynamic_cast<SingleMuon*> ( 
                        rf.get_subscriber( 
                            get_yaml<std::string>( instruct, "muon2" ) ) );

            if ( ! sm2 ){ throw std::runtime_error( "MuonComp: Could not read muon2") ; }

            const fne::Event * e = rf.get_event_ptr();

            return new MuonComp( *sel, e, *st, *sm1, *sm2 );
        }

}
