#include "BFCorrection.hh"
#include "NA62Constants.hh"

namespace fn
{
    BFCorrection::BFCorrection()
    {
        user_blueinit_();
    }

    Track BFCorrection::compute_bf_track (
            const fne::Event * event,
            const fne::RecoTrack * rt,
            const Vertex& vertex )
    {

        //Track properties
        int charge = rt->q;
        float tmom = rt->p;

        //Rawy track geometry
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

        return Track( TVector3( Vpnt[0], Vpnt[1], na62const::bz_tracking),
                TVector3( Vdir[0], Vdir[1], 1 ) );
    }
}
