#ifndef BFCORRECTION_HH
#define BFCORRECTION_HH
#include "Track.hh"
#include "RecoTrack.hh"
#include "Event.hh"
#if 0
/*
 *  ____  _____ ____                         _   _
 * | __ )|  ___/ ___|___  _ __ _ __ ___  ___| |_(_) ___  _ __
 * |  _ \| |_ | |   / _ \| '__| '__/ _ \/ __| __| |/ _ \| '_ \
 * | |_) |  _|| |__| (_) | |  | | |  __/ (__| |_| | (_) | | | |
 * |____/|_|   \____\___/|_|  |_|  \___|\___|\__|_|\___/|_| |_|
 *
 *
 */
#endif

extern"C" {
    void blue_tack_( int* nchar, float * tmom, float * Vxyz, float *vpnt, float *vdir );
    void user_blueinit_();
    void print_field_( float * pos, float * bfld );
}

namespace fn
{
    class BFCorrection
    {
        public:
            BFCorrection();

            Track compute_bf_track (
                     double corr_mom,
                    const fne::RecoTrack * rt,
                    const Vertex& vertex );

    };

    BFCorrection& global_BFCorrection();

}
#endif
