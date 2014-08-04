#ifndef TRACKING_FUNCTIONS_HH
#define TRACKING_FUNCTIONS_HH

#include "TVector3.h"
#include "RecoTrack.hh"
#include "Header.hh"
#include "Track.hh"

#if 0
/*
 *  _                  _    _
 * | |_ _ __ __ _  ___| | _(_)_ __   __ _
 * | __| '__/ _` |/ __| |/ / | '_ \ / _` |
 * | |_| | | (_| | (__|   <| | | | | (_| |
 *  \__|_|  \__,_|\___|_|\_\_|_| |_|\__, |
 *                                  |___/
 *   __                  _   _
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *
 */
#endif

namespace fn
{
    //General tracking functions
    double p_corr_ab( double p, int q, 
            double alpha, double beta );

    //Extrapolate  a compact track
    TVector3 extrapolate_bz( fne::RecoTrack& rt, double z );
    TVector3 extrapolate_z( fne::RecoTrack& rt, double z );


    //Extract tracks from compact events
    Track get_kaon_track( const fne::Conditions c );
    Track get_bz_track( const fne::RecoTrack& rt );
    Track get_z_track( const fne::RecoTrack& rt );


}
#endif
