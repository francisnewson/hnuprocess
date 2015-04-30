#include "muon_functions.hh"
#include "mc_help.hh"
#include "Mc.hh"
#include "NA62Constants.hh"
#include "Track.hh"

namespace fn
{
    std::pair<double, double> muv_impact(
            const fne::Mc & mce, std::size_t muon_pos,  int polarity )
    {
        const auto& muon = 
            *static_cast<const fne::McParticle*>( mce.particles[muon_pos] );

        Track muon_track{ muon.production_vertex, muon.momentum.Vect() };

        //if produced before MPN33 we have to apply kick
        if ( muon_track.get_point().Z() < na62const::zMagnet )
        {
            Track downstream_track{
                muon_track.extrapolate( na62const::zMagnet ),
                    muon_track.get_direction() 
                        + TVector3( -1 * polarity * na62const::mpn33_kick, 0, 0) };

            muon_track = std::move(downstream_track);
        }

        return std::make_pair(
                muon_track.extrapolate( na62const::zMuv2 ).X(),
                muon_track.extrapolate( na62const::zMuv1 ).Y()
                );
    }

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
