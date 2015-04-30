#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "Xcept.hh"

namespace fn
{
    RawSingleMuon::RawSingleMuon( const fne::Event * e, const SingleTrack& st)
        :e_( e ), st_( st )
    {}

    void RawSingleMuon::new_event()
    {
        dirty_ = true;
    }

    void RawSingleMuon::update() const
    {
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

    bool RawSingleMuon::found(){ return found_muon_; }
    double RawSingleMuon::weight(){ return 1; }
    double RawSingleMuon::x(){ return rm_->x; }
    double RawSingleMuon::y(){ return rm_->x; }
}
