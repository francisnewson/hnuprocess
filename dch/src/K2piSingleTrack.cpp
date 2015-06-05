#include "K2piSingleTrack.hh"
#include "K2piInterface.hh"
#include "Track.hh"
#include "NA62Constants.hh"
namespace fn
{

    void K2piSingleRecoTrack::update( const K2piDchData& dch_data, const K2piLkrData& lkr_data )
    {
        K2piDchInterface dch_interface{ dch_data };
        K2piLkrInterface lkr_interface{ lkr_data };

        us_track_ = Track{
            TVector3{ dch_interface.x0(), dch_interface.y0(), na62const::bz_tracking },
                TVector3{ dch_interface.dxdz(), dch_interface.dydz(), 1.0 } };

        ds_track_ = Track{
            TVector3{ dch_interface.ds_x0(), dch_interface.ds_y0(), na62const::z_tracking },
                TVector3{ dch_interface.ds_dxdz(), dch_interface.ds_dydz(), 1.0 } };

        kaon_track_ = Track{
            TVector3 { lkr_interface.pos0K_X(), lkr_interface.pos0K_Y(), 0 },
                     TVector3{ lkr_interface.pK_X(), lkr_interface.pK_Y(), lkr_interface.pK_Z() } };

        mom_ = dch_data.p; 
        vertex_ = compute_cda( kaon_track_, us_track_ );
    }

    int K2piSingleRecoTrack::get_charge() const { return 1;}

    TVector3 K2piSingleRecoTrack::get_3mom() const { return us_track_.get_direction().Unit() * mom_ ; }
    double K2piSingleRecoTrack::get_mom() const { return mom_; }

    TVector3 K2piSingleRecoTrack::get_vertex() const { return vertex_.point; }
    double K2piSingleRecoTrack::get_cda() const { return vertex_.cda; }

    double K2piSingleRecoTrack::get_time() const { return 0 ; }
    double K2piSingleRecoTrack::get_adjusted_time() const { return 0 ; }

    double K2piSingleRecoTrack::get_quality() const { return 0 ; }

    double K2piSingleRecoTrack::get_ddead_cell() const { return 0 ; }

    TVector3 K2piSingleRecoTrack::get_unscattered_3mom() const { return TVector3{};}

    //downstream
    TVector3 K2piSingleRecoTrack::extrapolate_ds( double z) const{ return ds_track_.extrapolate( z ); }
    TVector3 K2piSingleRecoTrack::get_ds_mom() const { return mom_ * ds_track_.get_direction().Unit() ; }

    //raw upstream
    TVector3 K2piSingleRecoTrack::extrapolate_us( double z) const{ return us_track_.extrapolate( z ); }
    TVector3 K2piSingleRecoTrack::get_us_mom() const { return mom_ * us_track_.get_direction().Unit() ; }

    //BF track
    TVector3 K2piSingleRecoTrack::extrapolate_bf( double z) const { return us_track_.extrapolate( z ); }

    int K2piSingleRecoTrack::get_compact_id() const { return 0 ; }
    int K2piSingleRecoTrack::get_muon_id() const { return 0 ; }

    //--------------------------------------------------

    K2piSingleTrack:: K2piSingleTrack(const K2piDchData & dch_data, 
                    const K2piLkrData& lkr_data)
        :dch_data_( dch_data ), lkr_data_( lkr_data )
    {
        set_reco_track( &single_reco_track_);
    }

    bool K2piSingleTrack::process_event() const
    {
        single_reco_track_.update( dch_data_, lkr_data_ );
        return true;
    }
}
