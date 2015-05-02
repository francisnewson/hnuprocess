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

            const auto& srt = st_.get_single_track();
            return check_muon_track_distance( sm_, srt, multiplier_ );
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

    //--------------------------------------------------

    bool check_muon_track_distance
        ( const SingleMuon& sm, const SingleRecoTrack& srt, double multiplier )
        {
            std::pair<double,double> muon_track_separation
                = get_muon_track_separation( sm, srt );

            double dx = muon_track_separation.first;
            double dy = muon_track_separation.second;

            double mom = srt.get_mom();

            double err_x = multiplier * mu_error_0902_sc( mom, 2 );
            double err_y = multiplier * mu_error_0902_sc( mom, 1 );

            //Check separation is within error
            if ( fabs(dx) > err_x + na62const::muv_half_width )
            { return false; }

            if ( fabs(dy) > err_y + na62const::muv_half_width )
            { return false; }

            return true;
        }

    std::pair<double,double> get_muon_track_separation
        ( const SingleMuon& sm, const SingleRecoTrack& srt )
        {
            double muon_x = sm.x();
            double muon_y = sm.y();

            //extrapolate track
            double track_x = srt.extrapolate_ds( na62const::zMuv2).X();
            double track_y = srt.extrapolate_ds( na62const::zMuv1).Y();

            //find track - muon sepration
            double dx = track_x - muon_x;
            double dy = track_y - muon_y;

            return std::make_pair( dx, dy );
        }
}
