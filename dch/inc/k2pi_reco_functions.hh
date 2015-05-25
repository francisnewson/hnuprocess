#ifndef K2PI_RECO_FUNCTIONS_HH
#define K2PI_RECO_FUNCTIONS_HH
#include "TVector3.h"
#include "logging.hh"
#include "Track.hh"
#include <boost/optional.hpp>
#if 0
/*
 *  _    ____        _                       
 * | | _|___ \ _ __ (_)  _ __ ___  ___ ___   
 * | |/ / __) | '_ \| | | '__/ _ \/ __/ _ \  
 * |   < / __/| |_) | | | | |  __/ (_| (_) | 
 * |_|\_\_____| .__/|_| |_|  \___|\___\___/  
 *            |_|
 *  __                  _   _
 * / _|_   _ _ __   ___| |_(_) ___  _ __  ___
 *| |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 *|  _| |_| | | | | (__| |_| | (_) | | | \__ \
 *|_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *
 *
 */
#endif

namespace fne{
    class Event;
    class McParticle;
}

namespace fn
{

    class K2piLkrData;
    class K2piDchData;
    class K2piEventData;
    class KaonTrack;
    class K2piRecoClusters;
    class Track;

    double extract_eop( K2piEventData& event, K2piDchData& dch, bool mc );
    double extract_eop_E( K2piEventData& event, K2piDchData& dch, bool mc );

    double extract_photon_sep ( K2piLkrData &raw_lkr );
    double extract_min_track_cluster_sep ( K2piLkrData &raw_lkr, K2piDchData& dch );

    double extract_min_photon_radius( K2piLkrData& raw_lkr );

    Vertex extract_vertex( const K2piDchData& dch, const K2piLkrData& lkr );



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
    

    struct k2pi_mc_parts
    {
        const fne::McParticle * k;
        const fne::McParticle * pip;
        const fne::McParticle * pi0;
        std::vector<const fne::McParticle*>  photons;
    };

    boost::optional<k2pi_mc_parts> extract_k2pi_particles( const fne::Event * e );
    
}
#endif
