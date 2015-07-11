#include "MuvXYP.hh"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TEfficiency.h"
#include "stl_help.hh"
#include "root_help.hh"

namespace fn
{
    namespace muveff
    {
        void ReweightXY::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {
            Double_t eff = hw_.GetBinContent( xBin, yBin );
            h.SetBinContent( gBin, h.GetBinContent( gBin ) * eff );
        }
        //--------------------------------------------------

        void ReweightP::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {
            Double_t eff = hp_.GetBinContent( pBin );
            h.SetBinContent( gBin, h.GetBinContent( gBin ) * eff );
        }
        //--------------------------------------------------

        CutXY::CutXY( const YAML::Node& params )
        {
            std::string shape = get_yaml<std::string>( params, "shape");

            if ( shape == "rectangles" )
            {
                std::vector<rectangle> recs=
                    get_yaml<std::vector<rectangle>>( params, "points" );

                cut_ = AreaCut{ recs };
            }
        }
        //--------------------------------------------------

        void CutXY::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {

            if ( ! cut_.allowed( { x, y } ) )
            {
                h.SetBinContent( gBin, 0);
            }
        }

        //--------------------------------------------------

        CutP::CutP( double min_p, double max_p)
            :min_p_( min_p), max_p_ ( max_p ){}

        void CutP::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {
            if ( p < min_p_ || p > max_p_ )
            {
                h.SetBinContent( gBin, 0);
            }
        }
        //--------------------------------------------------

        void MuvAcc::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {
            const double min_x_ = -135.0;
            const double max_x_ = +135.0;
            const double min_y_ = -135.0;
            const double max_y_ = +135.0;
            const double min_u_ = -135.0;
            const double max_u_ = +135.0;
            const double min_v_ = -135.0;
            const double max_v_ = +135.0;

            const double cospi4 = std::cos( 3.141592654 / 4.0 );
            const double sinpi4 = std::cos( 3.141592654 / 4.0 );

            double u = x * cospi4 - y * sinpi4;
            double v = x * sinpi4 + y * cospi4;

            bool bad_inner =  ( ( fabs( x ) < 11 ) && ( fabs( y ) < 11 ) );
            bool good_inner = ! bad_inner;

            bool good_outer =  ( true
                    && ( x > min_x_) && (x < max_x_ )
                    && ( y > min_y_) && (y < max_y_ )
                    && ( u > min_u_) && (u < max_u_ )
                    && ( v > min_v_) && (v < max_v_ )
                    );

            bool good_p  = (p > 5 );

            if (! ( good_inner && good_outer && good_p ) )
            {
                h.SetBinContent( gBin, 0);
            }
        }

        //--------------------------------------------------

        CompProcess::CompProcess( std::vector<XYPProcess*> procs )
            :procs_( procs ){}

        void CompProcess::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {
            for ( auto& proc : procs_ )
            {
                (*proc)( h, gBin, xBin, yBin, pBin, x, y , p );
            }

        }

        //--------------------------------------------------

        std::unique_ptr<TH2D> xy_eff( TH3D& hpassed, TH3D& htotal )
        {
            auto h2dpassed = std::unique_ptr<TH2D>{ static_cast<TH2D*>(hpassed.Project3D( "yx" ) )};
            auto h2dtotal = std::unique_ptr<TH2D>{ static_cast<TH2D*>(htotal.Project3D( "yx" ) )};
            h2dpassed->Divide( h2dtotal.get() );
            return h2dpassed;
        }

        std::unique_ptr<TEfficiency> p_eff( TH3D& hpassed, TH3D& htotal )
        {
            auto h1dpassed = std::unique_ptr<TH1D>{ static_cast<TH1D*>(hpassed.Project3D( "z" ) )};
            auto h1dtotal = std::unique_ptr<TH1D>{ static_cast<TH1D*>(htotal.Project3D( "z" ) )};

            return make_unique<TEfficiency>( *h1dpassed, *h1dtotal );
        }

        std::unique_ptr<TH1D> get_ratio( TEfficiency& teff )
        {
            std::unique_ptr<TH1D> hpassed { static_cast<TH1D*>(tclone( *teff.GetPassedHistogram() ) ) };
            const TH1 * htotal = teff.GetTotalHistogram();
            hpassed->Divide(htotal );
            return hpassed;
        }

        //--------------------------------------------------

        RadialHist::RadialHist( std::string name, std::string title)
            :h_( name.c_str(), title.c_str(), 150, 0, 150 )
        {
            h_.GetXaxis()->SetTitle( "radius (cm)" );
        }


        void RadialHist::operator()( TH3D& h, Int_t gBin,
                Int_t xBin, Int_t yBin, Int_t pBin,
                double x, double y, double p )
        {
            double r = std::hypot( x , y );
            h_.Fill( r, h.GetBinContent( gBin ) );
        }

        std::unique_ptr<TEfficiency> r_eff( TH3D& hpassed, TH3D& htotal )
        {
            auto hp = RadialHist{ "hpassed", "Radial eff"};
            auto ht = RadialHist{ "htotal", "Radial eff"};

            process_hist( hpassed, hp );
            process_hist( htotal, ht );

            return make_unique<TEfficiency>( hp.hist(), ht.hist() );
        }
    }
}
