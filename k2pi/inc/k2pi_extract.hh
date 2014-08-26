#ifndef K2PI_EXTRACT
#define K2PI_EXTRACT

#include "Event.hh"

namespace fn
{

    class K2piRecoEvent;
    class SingleRecoTrack;
    class KaonTrack;
    class K2piVars;

    void k2pi_extract( 
            bool mc, double weight,
            const fne::Event* event_,
            const K2piRecoEvent& k2pirc,
            const SingleRecoTrack& srt,
            const KaonTrack& kt,
            K2piVars& vars_ );

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
