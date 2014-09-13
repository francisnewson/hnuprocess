#ifndef K2PI_FUNCTIONS_HH
#define K2PI_FUNCTIONS_HH
#include "TVector3.h"
#include "logging.hh"
#include "Mc.hh"
#include "Event.hh"
#include "Track.hh"
#if 0
/*
 *  _    ____        _ 
 * | | _|___ \ _ __ (_)
 * | |/ / __) | '_ \| |
 * |   < / __/| |_) | |
 * |_|\_\_____| .__/|_|
 *   __       |_|        _
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *                                             
 */
#endif
namespace fne
{
    class Event;
}
namespace fn
{
    class KaonTrack;
    class K2piRecoClusters;

    TVector3 compute_neutral_vertex( const fne::Event * e,
            const KaonTrack& kt, const K2piRecoClusters & k2pirc ,
            logger * slg  = 0 , severity_level sl = debug );

    double bracket_solve_neutral_vertex
        (
         //const KaonTrack& kt,
         const Track& kt,
         double E1, const TVector3& pos1,
         double E2, const TVector3& pos2,
            logger * slg  = 0 , severity_level sl = debug );

    double pi0_cos_photon_opening( double E1, double E2 );

    double raw_solve_neutral_vertex
        ( TVector3 pos1, double E1, TVector3 pos2, double E2 );

   double  lkr_energy_res( double energy);
   double  lkr_pos_res( double energy);
    
    //--------------------------------------------------
    
}
#endif
