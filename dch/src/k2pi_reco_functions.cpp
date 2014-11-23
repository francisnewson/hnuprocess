#include "k2pi_reco_functions.hh"
#include "RecoCluster.hh"
#include "K2piEventData.hh"
#include "k2pi_fitting.hh"
#include "K2piInterface.hh"
#include "ClusterEnergyCorr.hh"
#include "Track.hh"
#include "NA62Constants.hh"

namespace fn
{
    double extract_eop( K2piEventData& event, K2piDchData& dch )
    {
        fne::RecoCluster rc;
        rc.energy = event.TCE;
        rc.x = event.TCX;
        rc.y = event.TCY;
        double energy = correct_eop_energy( rc );
        double momentum = dch.p;
        double eop = energy / momentum;

        return eop;
    }


    double extract_photon_sep ( K2piLkrData &raw_lkr )
    {
        K2piLkrInterface lkr( raw_lkr);
        TVector3 vec_sep = TVector3( lkr.posC1_X(), lkr.posC1_Y(), 0 ) 
            - TVector3( lkr.posC2_X(), lkr.posC2_Y() );
        double sep = vec_sep.Mag();
        return sep ;
    };

    double extract_min_track_cluster_sep ( K2piLkrData &raw_lkr, K2piDchData& raw_dch )
    {
        K2piLkrInterface lkr( raw_lkr);
        K2piDchInterface dch( raw_dch);
        Track dch_track( dch.x0(), dch.y0(), na62const::bz_tracking,
                dch.dxdz(), dch.dydz(), 1 );
        TVector3 track_pos = dch_track.extrapolate( na62const::zLkr );

        TVector3 sep1 = track_pos - 
            TVector3( lkr.posC1_X(), lkr.posC1_Y(), na62const::zLkr );

        TVector3 sep2 = track_pos - 
            TVector3( lkr.posC2_X(), lkr.posC2_Y(), na62const::zLkr );

        return std::min( sep1.Mag(), sep2.Mag() );
    }

    double extract_min_photon_radius( K2piLkrData& raw_lkr )
    {
        //Copy data into parameter array
        k2pi_params lkr_param_data;
        K2piLkrInterface lkr_interface( raw_lkr);
        copy( lkr_interface, lkr_param_data );

        //Extract 'fit' result
        k2pi_fit data_fit;
        compute_pion_mass( lkr_param_data, data_fit );

        Track track1( data_fit.neutral_vertex, data_fit.p1.Vect() );
        TVector3 pos1 = track1.extrapolate( na62const::zDch1 );

        Track track2 ( data_fit.neutral_vertex, data_fit.p2.Vect() );
        TVector3 pos2 = track2.extrapolate( na62const::zDch1 );

        return std::min( pos1.Perp(), pos2.Perp() );
    }
}
