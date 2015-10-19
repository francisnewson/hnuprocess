#include "BFCorrection.hh"
#include "NA62Constants.hh"

namespace fn
{
    BFCorrection::BFCorrection()
    {
        user_blueinit_();
    }

    Track BFCorrection::compute_bf_track (
            double corr_mom,
            const fne::RecoTrack * rt,
            const Vertex& vertex )
    {

        //Track properties
        int charge = rt->q;
        float tmom =  corr_mom;

        //Rawy track geometry
        //blue_tack requires point at DCH1 but here DCH1 beams bz_tracking:
        //ie. 9700.6 not 9708, so it's fine to take values straight from Compact.
        float Vpnt[2];
        Vpnt[0] = rt->bx;
        Vpnt[1] = rt->by;

        float Vdir[2];
        Vdir[0] = rt->bdxdz; 
        Vdir[1] = rt->bdydz;

        //Vertex estimate
        float Vxyz[3]; 
        vertex.point.GetXYZ( Vxyz);

        //BF Correction
        blue_tack_ ( &charge, &tmom, Vxyz, Vpnt, Vdir );

        //New dir is the direction at the midpoint  of ( Z_raw, dch_bz )
        //where the uncorrected and corrected tracks are defined to meet
        //here we return a track with the correct direction but
        //based at z = bz_tracking.
        //The calling code in SingleTrack.cpp just uses the direction.
        
        return Track( TVector3( Vpnt[0], Vpnt[1], na62const::bz_tracking),
                TVector3( Vdir[0], Vdir[1], 1 ) );
    }


    BFCorrection& global_BFCorrection()
    {
        static BFCorrection bfc;
        return bfc;
    }
}
