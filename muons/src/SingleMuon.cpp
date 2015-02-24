#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "Xcept.hh"
#include "tracking_selections.hh"
#include "yaml-cpp/yaml.h"

namespace fn
{
    REG_DEF_SUB( SingleMuon );

    template<>
        Subscriber * create_subscriber<SingleMuon>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = get_yaml<std::string>( instruct, "method" );
            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack * st = get_single_track( instruct, rf );

            if ( method == "Track" || method == "track" )
            {
                return new TrackSingleMuon( event, *st );
            }
            else
            {
                throw Xcept<UnknownSingleMuonMethod>( method );
            }
        }

    SingleMuon * get_single_muon( YAML::Node& instruct, RecoFactory& rf )
    {
        YAML::Node muon_node = instruct["inputs"]["sm"];

        if ( ! muon_node )
        { throw Xcept<MissingNode>( "sm" ); }

        SingleMuon * sm  = dynamic_cast<SingleMuon*>
            ( rf.get_subscriber( muon_node.as<std::string>() ) );

        if ( !sm )
        { throw Xcept<BadCast>( "SM" ); }

        return sm;
    }

    //--------------------------------------------------

    TrackSingleMuon::TrackSingleMuon( const fne::Event * e, const SingleTrack& st )
        :e_( e ), st_( st )
    {}

    void TrackSingleMuon::new_event()
    {
        dirty_ =  true;
    }

    void TrackSingleMuon::update() const
    {
        if ( ! dirty_ ){ return; }

        const auto & srt = st_.get_single_track();
        int muon_id_ = srt.get_muon_id();

        if ( muon_id_ < 0 )
        {
            found_muon_ = false;
            return;
        }

        found_muon_ = true;
        rm_ = static_cast<fne::RecoMuon*>( e_->detector.muons.At( muon_id_ ) );

        dirty_ = false;
        check_consistency();
    }

    bool TrackSingleMuon::found_muon() const 
    {
        update();
        return found_muon_; 
    }

    int TrackSingleMuon::get_muon_id() const
    {
        update();
        return muon_id_;
    }

    const fne::RecoMuon* TrackSingleMuon::get_muon() const 
    {
        update(); 
        return rm_ ;
    }

    void TrackSingleMuon::check_consistency() const
    {
        update();

        const auto & srt = st_.get_single_track();
        int track_id = srt.get_compact_id();

        std::vector<const fne::RecoMuon*> matching_muons;

        unsigned int nmuons = e_->detector.nmuons;

        for ( int imuon = 0 ; imuon != nmuons ; ++imuon )
        {
            const auto * muon = static_cast<const fne::RecoMuon*>(
                    e_->detector.muons.At( imuon ) );

            if ( muon->iTrk ==  track_id )
            {
                matching_muons.push_back( muon );
            }
        }

        if ( matching_muons.size() > 1 )
        {
            BOOST_LOG_SEV( get_log(), always_print )
                << "More than one matching muon!" ;
        }

        if ( matching_muons.size() == 1 )
        {
            if ( matching_muons.front() != rm_ )
            {
            BOOST_LOG_SEV( get_log(), always_print )
                << "Matching muon does not match track match" ;
            }
        }
    }
}

