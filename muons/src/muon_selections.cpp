#include "muon_selections.hh"
#include "muon_functions.hh"
#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "NA62Constants.hh"
#include "yaml_help.hh"

namespace fn
{

    REG_DEF_SUB( MuonReq );

    MuonReq::MuonReq( const SingleMuon& sm, 
            const SingleTrack& st, double multiplier )
        :sm_(sm), st_(st), multiplier_( multiplier)
    {}

    bool MuonReq::do_check() const
    {

        //check we have a muon
        if ( ! sm_.found() )
        {
            return false;
        }

        double muon_x = sm_.x();
        double muon_y = sm_.y();

        //extrapolate track
        const auto& srt = st_.get_single_track();
        double mom = srt.get_mom();
        double track_x = srt.extrapolate_ds( na62const::zMuv2).X();
        double track_y = srt.extrapolate_ds( na62const::zMuv1).Y();

        //find track - muon sepration
        double dx = track_x - muon_x;
        double dy = track_y - muon_y;

        double err_x = multiplier_ * mu_error_0902_sc( mom, 2 );
        double err_y = multiplier_ * mu_error_0902_sc( mom, 1 );

        if ( fabs(dx) > err_x + na62const::muv_half_width )
        { return false; }
        if ( fabs(dy) > err_y + na62const::muv_half_width )
        { return false; }

        return true;
    }
    double MuonReq::do_weight() const
    {
        return sm_.found() * sm_.weight();
    }

    template<>
        Subscriber * create_subscriber<MuonReq>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            const SingleTrack * st = get_single_track( instruct, rf );
            const SingleMuon * sm = get_single_muon( instruct, rf );
            double multiplier = get_yaml<double>( instruct, "multiplier" );

            return new MuonReq(*sm, *st, multiplier );
        }
}
