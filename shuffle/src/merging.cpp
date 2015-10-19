#include "merging.hh"
#include <boost/filesystem/path.hpp>
#include "root_help.hh"
#include "HistStore.hh"
#include "ezETAProgressBar.hh"

using boost::filesystem::path;
namespace fn
{
    std::pair<std::unique_ptr<TH1>, std::unique_ptr<OwningStack>>
        extract_data_stack( TFile& tfin, std::vector<std::string> paths )
        {
            std::vector<std::string> stack_paths;
            std::vector<std::string> data_paths;

            for( auto fp : paths  )
            {
                stack_paths.push_back( ( path{fp} / "hnu_stack" ).string() );
                data_paths.push_back( ( path{fp} / "hdata" ).string() );
            }

            auto stacks = extract_object_list<THStack>( tfin, stack_paths );
            bool do_zero_hist = true;
            auto owning_stack_sum =  sum_stacks(begin( stacks ), end( stacks ), do_zero_hist );

            auto data_hists = extract_hist_list<TH1D>( tfin, data_paths );
            auto data_sum = sum_hists( begin( data_hists), end( data_hists ) );

            return std::make_pair( std::move( data_sum ),  std::move( owning_stack_sum ) );
        }

    //--------------------------------------------------

    std::unique_ptr<TH1D> get_cumulative_hist( TH1& h, double centre )
    {
        TAxis * ax = h.GetXaxis();
        int nbins  = h.GetNbinsX();
        int centre_bin = ax->FindBin( centre );
        double bin_centre = ax->GetBinCenter( centre_bin );

        int centre_up;
        int centre_down;

        if ( bin_centre > centre )
        {
            centre_up = centre_bin;
            centre_down = centre_bin - 1 ;
        }
        else
        {
            centre_down = centre_bin;
            centre_up = centre_bin + 1;
        }

        int upper_dist = nbins - centre_up;
        int lower_dist = centre_down;

        int half_nbins = std::min( upper_dist, lower_dist ) - 1;
        int min_bin = centre_down  -  ( half_nbins - 1 );
        int max_bin = centre_up +  ( half_nbins - 1 );

        auto result = makeTH1D( "h_cum", "Cumulative", ( max_bin - min_bin + 1 ) / 2,
                h.GetXaxis()->GetBinLowEdge( centre_up ),
                h.GetXaxis()->GetBinUpEdge( max_bin ), h.GetXaxis()->GetTitle() );

        int my_min_bin = centre_down;
        int my_max_bin = centre_up;
        ez::ezETAProgressBar prog( half_nbins );
        prog.start();
        while( my_max_bin - centre_down < half_nbins )
        {
            ++prog;
            double val = h.Integral( my_min_bin, my_max_bin );
            double fill_point =
                ax->GetBinCenter( my_max_bin ) - ax->GetBinLowEdge( centre_up );

            result->SetBinContent( result->GetXaxis()->FindBin( fill_point ), val );
            ++my_max_bin;
            --my_min_bin;
        }

        return result;
    }

    //--------------------------------------------------

    std::unique_ptr<TH1D> HistErrors::compute_errors( TH1D& h )
    {
        std::unique_ptr<TH1D> result { tclone( h ) };

        int nBins = result->GetNbinsX();

        ez::ezETAProgressBar prog( nBins );
        prog.start();

        for ( int ibin = 1 ; ibin != nBins + 1 ; ++ibin )
        {
            ++prog;
            double low_edge = result->GetBinLowEdge( ibin );
            double up_edge = result->GetBinLowEdge( ibin + 1 );
            double err = get_err( h, low_edge, up_edge );
            result->SetBinError( ibin, err );
        }
        return result;
    }

    double HistErrors::get_err( TH1D& bg, double sig_min, double sig_max )
    {
        double background = integral( bg, sig_min, sig_max );

        double sqerr = 0;

        //std::cerr << sig_min << std::endl;

        if ( trig_ && background < 10000 )
        {
            auto trig_eff_err = (*trig_ )->get_eff_err( sig_min, sig_max ); 
            if( trig_eff_err.first != 0 )
            {
                if (  std::isnan( trig_eff_err.first ) || std::isnan( trig_eff_err.second ) 
                        || trig_eff_err.second <= 0 || trig_eff_err.first <= 0   )
                    {std::cerr << "trig:" << trig_eff_err.second * background 
                        << " "  <<  trig_eff_err.first << std::endl;}

                sqerr += fn::sq(trig_eff_err.second / trig_eff_err.first * background );
            }
        }

        if ( std::isnan( sqerr ) ) std::cerr << "NAN 1" << std::endl;

        if ( scat_contrib_ )
        {
            double scat = ( *scat_contrib_ ) ->get_rel_scatter_err( sig_min, sig_max );
            if (  std::isnan(scat) || scat < 0 )
                std::cerr << "scat:" << scat*background << std::endl;
            sqerr +=  fn::sq( background * scat );
        }

        if ( std::isnan( sqerr ) ) std::cerr << "NAN 2" << std::endl;

        double sq_muv_err = fn::sq( 0.01 * background );
        if (  std::isnan(sq_muv_err) || sq_muv_err < 0  )
            std::cerr << "muv:" << sq_muv_err << std::endl;
        sqerr += sq_muv_err;

        if ( std::isnan( sqerr ) ) std::cerr << "NAN 3" << std::endl;

        double sq_flux_err = fn::sq( 0.005 * background );
        if (  std::isnan(sq_flux_err)  )
            std::cerr << "flux:" << sq_flux_err << std::endl;
        sqerr += sq_flux_err;

        if ( std::isnan( sqerr ) ) std::cerr << "NAN 4" << std::endl;

        if ( halo_errors_  )
        {
            double sq_halo_scale_err = fn::sq( (*halo_errors_)->compute_halo_scale_err( sig_min, sig_max ) );
            if (  std::isnan(sq_halo_scale_err) ||  sq_halo_scale_err < 0   )
                std::cerr << "halo_scale:" << sq_halo_scale_err << std::endl;
            sqerr += sq_halo_scale_err;

            if ( std::isnan( sqerr ) ) std::cerr << "NAN 5" << std::endl;

            double sq_halo_val_err = fn::sq( (*halo_errors_)->compute_halo_val_err( sig_min, sig_max ) );
            if (  std::isnan(sq_halo_val_err) || sq_halo_val_err < 0   )
                std::cerr << "halo_val:" << sq_halo_val_err << std::endl;
            sqerr += sq_halo_val_err;

            if ( std::isnan( sqerr ) ) std::cerr << "NAN 6" << std::endl;

            double sq_halo_shape_err = fn::sq( (*halo_errors_)->compute_halo_shape_err( sig_min, sig_max ) );

            sqerr += sq_halo_shape_err;
        }

        if ( std::isnan( sqerr ) )
        {
            std::cerr << sig_min << " " << sqerr << std::endl;
        }
        if ( std::isnan( std::sqrt(sqerr) ) )
        {
            std::cerr << sig_min << " " << sqerr << std::endl;
        }

        return std::sqrt( sqerr );
    }

    void HistErrors::set_scatter_contrib( const ScatterContrib& sc )
    { scat_contrib_ = &sc ; }

    void HistErrors::set_trigger( const TriggerApp& trig )
    { trig_ = &trig; }

    void HistErrors::set_halo_errors( const HaloErrors& he )
    { halo_errors_ = &he; }
}
