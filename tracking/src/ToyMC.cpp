#include "ToyMC.hh"
#include "stl_help.hh"
#include <functional>
namespace toymc
{

    void track_params::shift_x( double xshift ) { x += xshift; }
    void track_params::shift_y( double yshift ) { y += yshift; }

    void track_params::kick_tx( double txkick ) { tx += txkick; }
    void track_params::kick_ty( double tykick ) { ty += tykick; }

    //--------------------------------------------------

    void ToyMCLibrary::add_toy( ToyMC * toymc )
    {
        using namespace fn;
        toy_lib_.push_back( std::unique_ptr<ToyMC>{ toymc } );
    }

    //--------------------------------------------------

    track_params ToyMCComposite::transfer( track_params tp ) const
    {
        for ( auto mc : children_ )
        {
            tp = mc->transfer( tp );
        }
        return tp;
    }

    double ToyMCComposite::get_length() const
    {
        return std::accumulate( begin( children_), end(children_ ), 0.0,
                []( const double cum_total, const ToyMC* mc )
                { return cum_total * mc->get_length(); } );
    }

    void ToyMCComposite::add_child( ToyMC * child )
    {
        children_.push_back( child );
    }

    //--------------------------------------------------

    ToyMCScatter::ToyMCScatter( RNGBase& gen, double rad_length, double length )
        :gen_( gen), rad_length_( rad_length), length_( length ),
        normal_( 0.0, 1.0 )
    {
        double nrad = length_ / rad_length_;
        multiplier_ = 13.6e-3 * std::sqrt( nrad ) * ( 1 + 0.038 * std::log( nrad ) );
    }

    double ToyMCScatter::get_length() const { return length_; }

    track_params ToyMCScatter::transfer( track_params tp) const
    {
        //determine scattering rms from momentum
        double theta_0 = multiplier_ / tp.get_mom() ;

        //generate 4 random numbers
        double x_r1 = normal_( gen_ );
        double x_r2 = normal_( gen_ );

        double y_r1 = normal_( gen_ );
        double y_r2 = normal_( gen_ );

        tp.shift_x( get_shift( x_r1, x_r2, theta_0, length_  ) );
        tp.kick_tx( get_kick(  x_r2, theta_0 ) );

        tp.shift_y( get_shift( y_r1, y_r2, theta_0, length_ ) );
        tp.kick_ty( get_kick(  y_r2, theta_0 ) );

        return tp;
    }

    double get_shift( double r1, double r2, double theta_0, double dz )
    {
        return r1 * dz * theta_0  / std::sqrt( 12 ) + r2 * dz * theta_0 / 2;
    }

    double get_kick(  double r2, double theta_0 )
    {
        return r2 * theta_0;
    }

    //--------------------------------------------------
}
