#include "muon_functions.hh"
#include "mc_help.hh"
#include "Mc.hh"
#include "NA62Constants.hh"
#include "Track.hh"

namespace fn
{
    std::size_t find_muon( const fne::Mc& mce )
    {
        for ( std::size_t n = 0 ; n != mce.npart ; ++n )
        {
            const auto& part = 
                *static_cast<const fne::McParticle*>( mce.particles[n] );

            if( part.type == cmc::muon_type )
                return n;
        }
        return mce.npart;
    }
}
