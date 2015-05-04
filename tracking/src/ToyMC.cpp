#include "ToyMC.hh"
#include "stl_help.hh"
#include <functional>
#include <iostream>
#include <iomanip>
#include <cassert>
namespace toymc
{

    void track_params::shift_x( double xshift ) { x += xshift; }
    void track_params::shift_y( double yshift ) { y += yshift; }

    void track_params::kick_tx( double txkick ) { tx += txkick; }
    void track_params::kick_ty( double tykick ) { ty += tykick; }

    void track_params::transfer_z( double dz ){ z += dz; }

    //--------------------------------------------------

    ToyMC *  ToyMCLibrary::add_toy( std::unique_ptr<ToyMC>  toymc )
    {
        using namespace fn;
        toy_lib_.push_back( std::move( toymc ) );
        return  toy_lib_.back().get();
    }

    //--------------------------------------------------

    ToyMCComposite::ToyMCComposite( const std::vector<ToyMC*>& children )
        :children_( children )
    {}

    track_params ToyMCComposite::noisy_transfer( track_params tp ) const
    {
        for ( auto mc : children_ )
        {
            std::cout << std::setw(10) << tp.z << "]"
                << ">" << std::setw(10) << mc->get_length()<< ">" ;
            tp = mc->transfer( tp );
            std::cout << "[" << std::setw(10) << tp.z << std::endl;
        }
        return tp;
    }

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
                { return cum_total + mc->get_length(); } );
    }

    void ToyMCComposite::set_length( double length)
    {
        throw std::runtime_error( "Set length not implemented for ToyMCComposite" );
    }

    void ToyMCComposite::add_child( ToyMC * child )
    {
        children_.push_back( std::move(child) );
    }

    ToyMCComposite * ToyMCComposite::clone()
    { return new ToyMCComposite( *this ); }

    //--------------------------------------------------

    ToyMCScatter::ToyMCScatter( RNGBase& gen, double rad_length, double length )
        :gen_( gen), rad_length_( rad_length), length_( length ),
        normal_( 0.0, 1.0 )
    {
        assert( length_ > 0 );
        double nrad = length_ / rad_length_;
        std::cout << "nrad: " << nrad << std::endl;
        multiplier_ = 13.6e-3 * std::sqrt( nrad ) * ( 1 + 0.038 * std::log( nrad ) );
    }

    double ToyMCScatter::get_length() const { return length_; }

    void ToyMCScatter::set_length(double length) 
    {
        length_ = length;
        double nrad = length_ / rad_length_;
        multiplier_ = 13.6e-3 * std::sqrt( nrad ) * ( 1 + 0.038 * std::log( nrad ) );
    }

    track_params ToyMCScatter::transfer( track_params tp) const
    {
        //determine scattering rms from momentum
        double theta_0 = multiplier_ / tp.get_mom() ;

        //generate 4 random numbers
        double x_r1 = normal_( gen_ );
        double x_r2 = normal_( gen_ );

        double y_r1 = normal_( gen_ );
        double y_r2 = normal_( gen_ );

        //shift due to intial angle
        tp.shift_x( length_ * tp.tx );

        //shift due to scattering ( holds for small tx )
        tp.shift_x( get_shift( x_r1, x_r2, theta_0, length_  ) );

        //angular kick
        tp.kick_tx( get_kick(  x_r2, theta_0 ) );

        //shift due to intial angle
        tp.shift_y( length_ * tp.ty );

        //shift due to scattering ( holds for small tx )
        tp.shift_y( get_shift( y_r1, y_r2, theta_0, length_ ) );

        tp.kick_ty( get_kick(  y_r2, theta_0 ) );

        tp.transfer_z( length_ ) ;


        return tp;
    }

    ToyMCScatter * ToyMCScatter::clone()
    {
        return new ToyMCScatter( *this );
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


    ToyMCThickScatter::ToyMCThickScatter( RNGBase& gen, double rad_length, 
            double length, int n_div )
        :toy_scatter_( gen, rad_length, length / double( n_div ) ),
        n_div_( n_div ), length_( length )
    {}

    double ToyMCThickScatter::get_length() const
    { return length_ ;}

    void ToyMCThickScatter::set_length(double length)
    {
        length_ = length;
        toy_scatter_.set_length( length_ /double( n_div_) );
    }

    track_params ToyMCThickScatter::transfer( track_params tp ) const
    {
        for ( int i = 0 ; i != n_div_ ; ++ i )
        {
            tp = toy_scatter_.transfer( tp );
        }

        return tp;
    }

    ToyMCThickScatter * ToyMCThickScatter::clone()
    {
        return new ToyMCThickScatter( *this );
    }

    //--------------------------------------------------
    ToyMCDipoleBend::ToyMCDipoleBend( double mom_kick, int polarity )
        :mom_kick_( mom_kick), polarity_( polarity )
    {}

    void ToyMCDipoleBend::set_magnet_polarity( int polarity )
    { polarity_ = polarity; }

    track_params ToyMCDipoleBend::transfer( track_params tp ) const
    {
        double px = tp.tx * tp.p;
        px -= mom_kick_ * tp.q;
        tp.tx = px / tp.p;
        return tp;
    }

    double ToyMCDipoleBend::get_length() const
    {
        return 0;
    }

    void ToyMCDipoleBend::set_length(double length)
    {
        throw std::runtime_error( "Attempt to set length of ToyMCDipoleBend");
    }

    ToyMCDipoleBend * ToyMCDipoleBend::clone()
    { return new ToyMCDipoleBend( *this ); }

    //--------------------------------------------------

    ToyMCPropagate::ToyMCPropagate( double length )
        :length_( length ){}

    double ToyMCPropagate::get_length() const
    { return length_; }


    void ToyMCPropagate::set_length(double length)
    {
        length_ = length;
    }


    track_params ToyMCPropagate::transfer( track_params tp ) const
    {
        tp.x = tp.x + length_ * std::tan( tp.tx );
        tp.y = tp.y + length_ * std::tan( tp.ty );
        tp.z = tp.z + length_;

        return tp;
    }

    ToyMCPropagate * ToyMCPropagate::clone()
    {
        return new ToyMCPropagate( * this );
    }

    //--------------------------------------------------
}
