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

    double mu_error_0902_sc( double mom, int muv_plane )
    {
        //Copied from compact/rlib/anasrc/murec0902.c

        double total_rad_lengths = 
            + na62const::len_lkr / na62const::X0_lkr /* scattering in LKr */
            + na62const::len_hac / na62const::X0_iron /* scattering in HAC */
            + muv_plane * na62const::len_muv_iron / na62const::X0_iron; /* scattering in MUVs */

        double zMuv = 0;

        switch( muv_plane )
        {
            case 1: zMuv = na62const::zMuv1; break;
            case 2: zMuv = na62const::zMuv2; break; 
            case 3: zMuv = na62const::zMuv3; break;
            default:
                    throw std::runtime_error(
                            "Unknown MUV plane: " + std::to_string( muv_plane ) );
        }

        double p = 1000 * mom; //GeV to MeV to match COmPACT

        double scattering_error = ( zMuv - na62const::zLkr )
            * (13.6 / p) * std::sqrt( total_rad_lengths / 3 );

        double dch_error = ( (0.1 + 8.5 * (1000 / p ) ) / 10000 ) 
            * ( zMuv - na62const::z_tracking ) ;

        double total_error  = std::hypot( scattering_error, dch_error );

        return total_error;
    }
}

