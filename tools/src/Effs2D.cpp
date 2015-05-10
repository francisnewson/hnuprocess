#include "Effs2D.hh"
#include <iostream>
#include "Xcept.hh"
#include "root_help.hh"
namespace fn
{

    //**************************************************
    //Effs2D
    //**************************************************

    Effs2D::Effs2D( std::vector<double> xedges, 
            std::vector<double> yedges, 
            std::vector<double> effs )
        :xedges_( xedges ), yedges_( yedges ), effs_( effs )
    {
        std::size_t nbins = ( xedges_.size() -1 ) * (yedges_.size() -1 );
        if ( effs_.size() != nbins )
        {
            throw std::runtime_error( "Eff2D: Wrong number of effs" );
        }
    }

    void Effs2D::print_eff_map( std::ostream& os ) const
    {
        os << "Effs2D\n";

        for ( int j = yedges_.size()-2 ; j != 0 ; --j )
        {
            for ( int i = 0; i !=  xedges_.size() -2 ; ++i )

            {
                int bin = (j) * ( xedges_.size() -1 ) + (i );
                try
                {
                    double eff = effs_.at( bin );
                    os << ( eff > 0.1 ) << " " ;
                }
                catch( std::out_of_range& e )
                {
                    os << e.what() << std::endl;
                    os << i << " " << j << " "<< bin << std::endl;
                }
            }
            os << std::endl;
        }
    }

    double Effs2D::efficiency( double x, double y ) const
    {
        //Check we are in range of grid
        if ( x <= xedges_.front() || x >= xedges_.back() ){ return 0; }
        if ( y <= yedges_.front() || y >= yedges_.back() ){ return 0; }

        std::size_t xbin = get_bin( x, xedges_ );
        std::size_t ybin = get_bin( y, yedges_ );

        std::size_t eff_bin = ybin * ( xedges_.size() - 1 ) + xbin;

        double result = 0;
        try {
            result = effs_.at( eff_bin );
        }
        catch( std::out_of_range& e )
        {
            std::cout << e.what() << std::endl;
            std::cout << "pos_x: " << x << "\npos_y: " << y <<"\n" ;
            std::cout << "bin_x: " << xbin << "\nbin_y: " << ybin <<"\n" ;
            std::cout << "min_x: " << xedges_.front() << "\nmax_x: " << xedges_.back()  <<"\n" ;
            std::cout << "min_y: " << yedges_.front() << "\nmax_y: " << yedges_.back()  <<"\n" ;
            throw;
        }
        return result;
    }

    std::size_t Effs2D::get_bin( double val, const std::vector<double>& edges ) const
    {
        auto it = edges.cbegin();
        while (  val >= *it ){ ++it; }
        --it;
        std::size_t bin = std::distance( edges.cbegin() , it );
        return bin;
    }

    //--------------------------------------------------

    EffsTH2D::EffsTH2D( const TH2D &h )
        :heffs_( h  ){}

    void EffsTH2D::set_eff_hist( TH2D  h )
    {
        heffs_ = std::move(h);
    }

    double EffsTH2D::efficiency( double x, double y ) const
    {
        return heffs_.GetBinContent( heffs_.FindFixBin( x, y ) );
    }

}
