#include "ScmpVsCmp.hh"
#include "Event.hh"
#include "RecoFactory.hh"
#include "RecoTrack.hh"
#include "RecoCluster.hh"
#include "RecoMuon.hh"
#include <iomanip>

namespace fn
{
    REG_DEF_SUB( ScmpVsCmp );

    ScmpVsCmp::ScmpVsCmp( const fn::Selection& sel, 
            const fne::Event* e, std::ostream& os )
        :Analysis( sel) , e_( e), os_( os )
    {}

    void ScmpVsCmp::process_event()
    {
        os_ << std::setw(12) << e_->header.burst_time 
            << std::setw( 6 ) << e_->header.compact_number
            << std::setw( 6 ) << e_->header.trig_word
            << std::setw( 6 ) << e_->conditions.pkp 
            << std::setw( 6 ) << e_->detector.ntracks;

        if ( e_->detector.ntracks > 0 )
        {
            auto track = static_cast<fne::RecoTrack*> 
                ( e_->detector.tracks[0] );
            os_ << std::setw(10 ) << track->p;
        }
        else
        {
            os_ << std::setw(10 ) << '-' ;
        }

          os_  << std::setw( 6 ) << e_->detector.nclusters;
        if ( e_->detector.nclusters > 0 )
        {
            auto cluster = static_cast<fne::RecoCluster *> 
                ( e_->detector.clusters[0] );
            os_ << std::setw(10 ) << cluster->energy;
        }
        else
        {
            os_ << std::setw(10 ) << '-' ;
        }

          os_  << std::setw( 6 ) << e_->detector.nmuons;
        if ( e_->detector.nmuons > 0 )
        {
            auto muon = static_cast<fne::RecoMuon *> 
                ( e_->detector.muons[0] );
            os_ << std::setw(10 ) << muon->x;
        }
        else
        {
            os_ << std::setw(10 ) << '-' ;
        }

        os_  << std::endl;
    }

    template<>
        Subscriber * create_subscriber<ScmpVsCmp>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            Selection * sel = rf.get_selection
                ( instruct["selection"].as<std::string>() );

            const fne::Event * event = rf.get_event_ptr();

            std::ostream& os = rf.get_ostream
                ( instruct["inputs"]["os"].as<std::string>() );

            return new  ScmpVsCmp( *sel, event , os  );
        }
}
