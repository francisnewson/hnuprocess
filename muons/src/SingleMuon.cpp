#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "Xcept.hh"
#include "tracking_selections.hh"

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

            if ( method == "raw" )
            {
                return new RawSingleMuon( event, *st );
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

    RawSingleMuon::RawSingleMuon( const fne::Event * e, const SingleTrack& st)
        :e_( e ), st_( st )
    {}

    void RawSingleMuon::new_event()
    {
        dirty_ = true;
    }

    void RawSingleMuon::update() const
    {
        if (!dirty_ ){ return ; }

        //lookt at track info
        const auto & srt = st_.get_single_track();
        int muon_id = srt.get_muon_id();
        int track_id = srt.get_compact_id();

        //negative id means no muon found
        if ( muon_id < 0 )
        {
            found_muon_ = false;
            return;
        }

        //save muon
        found_muon_ = true;
        rm_ = static_cast<fne::RecoMuon*>( e_->detector.muons.At( muon_id ) );

        dirty_ = false;

        //check consistency
        int n_matching_muons = 0;
        unsigned int nmuons = e_->detector.nmuons;
        for ( int imuon = 0 ; imuon != nmuons ; ++imuon )
        {
            const auto * muon = static_cast<const fne::RecoMuon*>(
                    e_->detector.muons.At( imuon ) );

            if ( muon->iTrk ==  track_id )
            {
                ++n_matching_muons;
            }
        }

        if ( n_matching_muons> 1 )
        {
            BOOST_LOG_SEV( get_log(), always_print )
                << "More than one matching muon!" ;
        }
    }

    bool RawSingleMuon::found() const { update(); return found_muon_; }
    double RawSingleMuon::weight() const { update(); return 1; }
    double RawSingleMuon::x() const { update(); return rm_->x; }
    double RawSingleMuon::y() const { update(); return rm_->y; }
}
