#ifndef K2PI_EXTRACT
#define K2PI_EXTRACT

#include "Event.hh"

#if 0
/*
 *  _    ____        _
 * | | _|___ \ _ __ (_)
 * | |/ / __) | '_ \| |
 * |   < / __/| |_) | |
 * |_|\_\_____| .__/|_|
 *            |_|
 *            _                  _
 *   _____  _| |_ _ __ __ _  ___| |_
 *  / _ \ \/ / __| '__/ _` |/ __| __|
 * |  __/>  <| |_| | | (_| | (__| |_
 *  \___/_/\_\\__|_|  \__,_|\___|\__|
 *
*/

//Construct a K2piVars object from a K2piRecoEvent
//( plus SingleRecoTrack and KaonTrack )

#endif

namespace fn
{

    class K2piRecoEvent;
    class SingleRecoTrack;
    class KaonTrack;
    class K2piVars;
    class K2piData;

    void k2pi_extract( 
            bool mc, double weight,
            const fne::Event* event_,
            const K2piRecoEvent& k2pirc,
            const SingleRecoTrack& srt,
            const KaonTrack& kt,
            K2piVars& vars_ );

    void k2pi_extract_data( const K2piRecoEvent& k2pirc, 
            const SingleRecoTrack& srt, const KaonTrack& kt,
             K2piData& data_ );

    //--------------------------------------------------

    struct k2pi_mc_parts
    {
        const fne::McParticle * k;
        const fne::McParticle * pip;
        const fne::McParticle * pi0;
        std::vector<const fne::McParticle*>  photons;
    };

     k2pi_mc_parts extract_k2pi_particles( const fne::Event * e );


}

#endif
