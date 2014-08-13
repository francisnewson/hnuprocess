#include "tracking_functions.hh"
#include "NA62Constants.hh"

namespace fn
{

    //General tracking functions
    double p_corr_ab( double p, int q, double alpha, double beta )
    {
        return p * ( 1 + beta ) * ( 1 + alpha * p * q );
    }

    //--------------------------------------------------

    //Extrapolate  a compact track
    TVector3 extrapolate_bz( fne::RecoTrack& rt, double z )
    {
        double dz = z - na62const::bz_tracking;
        return TVector3( 
                rt.bx + dz * rt.bdxdz,
                rt.by + dz * rt.bdydz,
                z );
    }

    TVector3 extrapolate_z( fne::RecoTrack& rt, double z )
    {
        double dz = z - na62const::z_tracking;
        return TVector3( 
                rt.x + dz * rt.dxdz,
                rt.y + dz * rt.dydz,
                z );
    }

    //--------------------------------------------------

    //Extract tracks from compact events
    Track get_kaon_track( const fne::Conditions c )
    {
        return Track { c.pkxoffp, c.pkyoffp, 0, 
            c.pkdxdzp, c.pkdydzp, 1 };
    }

    Track get_bz_track( const fne::RecoTrack& rt )
    { 
        return Track { rt.bx, rt.by, na62const::bz_tracking,
            rt.bdxdz, rt.bdydz, 1  };
    }

    Track get_z_track( const fne::RecoTrack& rt )
    {
        return Track { rt.x, rt.y, na62const::z_tracking,
            rt.dxdz, rt.dydz, 1  };
    }
}
