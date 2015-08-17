#include "Limiter.hh"
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <iomanip>
#include "root_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "fiducial_functions.hh"
#include "TF1.h"
#include "TVectorD.h"
#include "TEfficiency.h"
#include "TRolke.h"
#include "NA62Constants.hh"
#include "TriggerApp.hh"

using boost::filesystem::path;

namespace fn
{
    HnuLimParams::HnuLimParams( TFile& tf, 
            const std::map<std::string, double>& fid_weights,
            const std::vector<std::string>& regions,
            int mass, bool fancyfit )
        :tf_(tf), fid_weights_(fid_weights), regions_( regions ),
        mass_( mass ), fancy_fit_( fancyfit )
    {
        chan_ = ( boost::format( "mc.p5.hnu.%1%.nodk") % mass_ ).str();
        build_totals();
        compute_shape();
    }

    int HnuLimParams::get_mass() const
    {
        return mass_;
    }

    double HnuLimParams::get_mean() const
    {
        return mean_;
    }

    double HnuLimParams::get_width() const
    {
        return width_;
    }

    const TH1D& HnuLimParams::get_signal_hist() const
    {
        return *h_sig_;
    }

    double HnuLimParams::get_full_acceptance() const
    {
        return sig_total_ / fid_total_ ;
    }

    double HnuLimParams::get_width_acceptance() const
    {
        return width_acc_;
    }

    double HnuLimParams::get_width_acceptance_err() const
    {
        return width_acc_err_;
    }

    std::string HnuLimParams::get_chan() const
    { 
        return chan_;
    }

    double HnuLimParams::get_width_min() const
    {
        return width_low_edge_;
    }

    double HnuLimParams::get_width_max() const
    {
        return width_up_edge_;
    }


    void HnuLimParams::build_totals()
    {
        std::vector<std::string> pols { "pos", "neg" };
        std::vector<std::string> regs { "signal_upper_muv", "signal_lower_muv" };

        std::vector<std::unique_ptr<TH1D>> hists;
        std::vector<double> this_fid_weight;


        for ( const auto& pol : pols  )
        {
            std::string polchan =
                ( boost::format( "%s.%s" ) % chan_ % pol ).str();

            for( const auto& reg : regions_ )
            {
                path p = path{ polchan  } / reg / "h_m2m_kmu";
                auto h = extract_hist<TH1D>( tf_,  p );
                hists.emplace_back( std::move( h ) );
            }

            this_fid_weight.push_back( fid_weights_.at( polchan ) );
        }

        fid_total_ = std::accumulate(
                begin( this_fid_weight ), end( this_fid_weight ) , 0.0 );

        h_sig_ = sum_hists( begin( hists ), end( hists ) );
        sig_total_ = integral( *h_sig_, 0, 0.2 );
    }

    void HnuLimParams::compute_shape()
    {
        if ( fancy_fit_ )
        {
            fit_ = make_unique<TF1>
                ( "double_gaus", "[0]*exp( -0.5 * ( ( x - [1]) / [2] )**2 )"
                  " + [3]*exp( -0.5 * ( ( x - [1] ) / [4] ) **2 )", 0, 1 );

            fit_->SetParameter( 0, h_sig_->GetMaximum() / 2 ) ;
            fit_->SetParameter( 1, h_sig_->GetMean() ) ;
            fit_->SetParameter( 2, 0.9 * h_sig_->GetRMS() ) ;
            fit_->SetParameter( 3, h_sig_->GetMaximum() / 2 ) ;
            fit_->SetParameter( 4, 1.1*  h_sig_->GetRMS() ) ;

            h_sig_->Fit( fit_.get() ,"quiet" );

            fita_ = make_unique<TF1>
                ( "gausa", "[0]*exp( -0.5 * ( ( x - [1]) / [2] )**2 )", 0, 1 );
            fita_->SetParameter( 0, fit_->GetParameter( 0 ) );
            fita_->SetParameter( 1, fit_->GetParameter( 1 ) );
            fita_->SetParameter( 2, fit_->GetParameter( 2 ) );
            fita_->SetLineColor( kGreen );

            fitb_ = make_unique<TF1>
                ( "gausb", "[0]*exp( -0.5 * ( ( x - [1]) / [2] )**2 )", 0, 1 );
            fitb_->SetParameter( 0, fit_->GetParameter( 3 ) );
            fitb_->SetParameter( 1, fit_->GetParameter( 1 ) );
            fitb_->SetParameter( 2, fit_->GetParameter( 4 ) );
            fitb_->SetLineColor( kBlue );

            h_sig_->GetListOfFunctions()->Add( fita_.get() );
            h_sig_->GetListOfFunctions()->Add( fitb_.get() );

            mean_  = h_sig_->GetFunction("double_gaus")->GetParameter(1);
        }
        else
        {
            mean_ = (0.001 * mass_ ) * ( 0.001 * mass_ );
        }

        //find quantiles
        int bin_mean = h_sig_->GetXaxis()->FindBin( mean_ );
        double cum_sum  = 0 ;
        int min_bin = bin_mean;
        int max_bin = bin_mean;

        while ( cum_sum < 0.65 * sig_total_ )
        {
            --min_bin;
            ++max_bin;
            cum_sum = h_sig_->Integral( min_bin, max_bin );
        }

        width_low_edge_ = h_sig_->GetXaxis()->GetBinLowEdge( min_bin );
        width_up_edge_ = h_sig_->GetXaxis()->GetBinUpEdge( max_bin );

        width_ = width_up_edge_ - width_low_edge_;

        width_total_ = cum_sum;
        auto width_acc_err = eff_err( width_total_ , fid_total_) ;
        width_acc_ = width_acc_err.first;
        width_acc_err_ = width_acc_err.second;
    }

    void HnuLimParams::write()
    {
        h_sig_->Write( chan_.c_str() );
    }

    //--------------------------------------------------

    ScatterContrib::ScatterContrib( TFile& tf_noscat, TFile& tf_scat )
        :tf_noscat_( tf_noscat ), tf_scat_( tf_scat )
    {
        std::vector<std::string> folders{ "up/pos", "up/neg", "dn/pos", "dn/neg" };

        std::vector<std::unique_ptr<TH1>> vh_noscat;
        std::vector<std::unique_ptr<TH1>> vh_scat;

        for ( auto& f : folders )
        {
            auto p = boost::filesystem::path{ "signal_muv" } / f / "h_m2m" / "hbg" ;
            vh_noscat.push_back( std::move(extract_hist( tf_noscat, p ) ) );
            vh_scat.push_back( std::move( extract_hist( tf_scat, p ) ) );
        }

        h_noscat_ = sum_hists( begin( vh_noscat ), end( vh_noscat ) );
        h_scat_ = sum_hists( begin( vh_scat ), end( vh_scat ) );
    }

    double ScatterContrib::get_rel_scatter_err( double sig_min, double sig_max ) const
    {
        double noscat = integral( *h_noscat_, sig_min, sig_max );
        double scat = integral( *h_scat_, sig_min, sig_max );
        if ( noscat == 0 )
        {
            return 0;
        }
        if ( scat == 0 )
        {
            return 1.0;
        }
        return fabs(scat - noscat) / scat;
    }

    //--------------------------------------------------
    Limiter::Limiter( TFile& bg_file,  std::vector<std::string> regions )
        :bg_file_( bg_file ), pols_{ "pos", "neg" },regions_( regions )
        {
            km2_flux_paths_ = std::vector<std::string> { "neg_lower","pos_lower" };
        }

    HnuLimResult Limiter::get_limit( const HnuLimParams& params )
    {
        //Deterimine signal region
        double sig_min = params.get_width_min();
        double sig_max = params.get_width_max();

        //Ouput object
        HnuLimResult res{};

        //**************************************************
        //Determine background 
        //**************************************************

        //Determine #background events
        double background = get_background( sig_min, sig_max );
        double data = get_data( sig_min, sig_max );
        res.sig_min = sig_min;
        res.sig_max = sig_max;

        //apply trigger efficiency
        auto trig_eff_err = get_trig_eff( sig_min, sig_max );
        double trig_eff = trig_eff_err.first;
        double trig_err = trig_eff_err.second;
        background *= trig_eff;

        //**************************************************
        //Determine background error
        //**************************************************
        double sqr_background_err  = 0;

        //triggger eff
        double sq_trig_eff_contrib = fn::sq( trig_err / trig_eff * background );
        sqr_background_err += sq_trig_eff_contrib;
        res.error_budget["err_trig"] = sq_trig_eff_contrib;

        //scattering
        double sq_scat_contrib =  0;
        if ( scat_contrib_ )
        {
            std::cerr << "DOING SCAT" << std::endl;
            double scat = (*scat_contrib_)->get_rel_scatter_err( sig_min, sig_max );
            sq_scat_contrib = fn::sq(background * scat );
        }
        sqr_background_err +=  sq_scat_contrib;
        res.error_budget["err_scat"] = sq_scat_contrib;

        //muv_eff
        double sq_muv_err = fn::sq( 0.01 * background );
        sqr_background_err +=  sq_muv_err ;
        res.error_budget["err_muv"] = sq_muv_err;

        //km2 flux
        std::pair<double,double> km2_flux_and_err = get_km2_flux_and_err();
        double km2_flux = km2_flux_and_err.first;
        double km2_flux_err = km2_flux_and_err.second;
        double sq_km2_flux_err_contrib = fn::sq( km2_flux_err / km2_flux * background );
        sqr_background_err += sq_km2_flux_err_contrib;
        res.error_budget["err_flux"] = sq_km2_flux_err_contrib;

        //halo scale
        double sq_halo_scale_err = fn::sq( compute_halo_scale_err( sig_min, sig_max ) );
        sqr_background_err +=  sq_halo_scale_err ;
        res.error_budget["err_hscale"] = sq_halo_scale_err;

        //halo measurement
        double sq_halo_val_err = fn::sq( compute_halo_val_err( sig_min, sig_max ) );
        sqr_background_err +=  sq_halo_val_err;
        res.error_budget["err_hval"] = sq_halo_val_err;

        //remember to square root
        double background_err = std::sqrt( sqr_background_err );

        //**************************************************
        //Compute limits
        //**************************************************

#if 0
        //100% signal efficiency for comparison
        TRolke rolke;
        rolke.SetGaussBkgKnownEff( background, background, background_err , 1.0 );
        double ul = rolke.GetUpperLimit();
#endif

        //Rolke does the heavy lifting
        TRolke rolke_eff;
        rolke_eff.SetBounding(true);
        rolke_eff.SetCL( 0.9 );
        rolke_eff.SetGaussBkgKnownEff( background, background, background_err , params.get_width_acceptance() );
        double ul = rolke_eff.GetUpperLimit();

        //do brazil
        double high1_background = background + std::sqrt( background );
        rolke_eff.SetGaussBkgKnownEff( high1_background, background, background_err , params.get_width_acceptance() );
        double high1_ul = rolke_eff.GetUpperLimit();

        double low1_background = background - std::sqrt( background );
        rolke_eff.SetGaussBkgKnownEff( low1_background, background, background_err , params.get_width_acceptance() );
        double low1_ul = rolke_eff.GetUpperLimit();

        double high2_background = background + 2 *  std::sqrt( background );
        rolke_eff.SetGaussBkgKnownEff( high2_background, background, background_err , params.get_width_acceptance() );
        double high2_ul = rolke_eff.GetUpperLimit();

        double low2_background = background - 2 * std::sqrt( background );
        rolke_eff.SetGaussBkgKnownEff( low2_background, background, background_err , params.get_width_acceptance() );
        double low2_ul = rolke_eff.GetUpperLimit();

        //do data
        rolke_eff.SetGaussBkgKnownEff( data, background, background_err , params.get_width_acceptance() );
        double dt_ul =  rolke_eff.GetUpperLimit();
        double dt_ll =  rolke_eff.GetLowerLimit();

        //Convert for literature
        double ul_br = ul / km2_flux;
        double ul_u2 = br_to_mix( ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double high1_ul_br = high1_ul / km2_flux;
        double high1_ul_u2 = br_to_mix( high1_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double low1_ul_br = low1_ul / km2_flux;
        double low1_ul_u2 = br_to_mix( low1_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double high2_ul_br = high2_ul / km2_flux;
        double high2_ul_u2 = br_to_mix( high2_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double low2_ul_br = low2_ul / km2_flux;
        double low2_ul_u2 = br_to_mix( low2_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double dt_ul_br = dt_ul / km2_flux;
        double dt_ul_u2 = br_to_mix( dt_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double dt_ll_br = dt_ll / km2_flux;
        double dt_ll_u2 = br_to_mix( dt_ll_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        res.trig_eff = trig_eff;
        res.trig_err = trig_err;
        res.background = background;
        res.background_err = background_err ;
        res.dt_obs = data;
        res.acc_sig_ul = ul;
        res.orig_sig_ul = ul / params.get_width_acceptance();
        res.rolke_orig_sig_ul = ul;

        res.ul_br =  ul_br;
        res.ul_u2 = ul_u2;

        res.high1_ul_br =  high1_ul_br;
        res.high1_ul_u2 = high1_ul_u2;

        res.low1_ul_br =  low1_ul_br;
        res.low1_ul_u2 = low1_ul_u2;

        res.high2_ul_br =  high2_ul_br;
        res.high2_ul_u2 = high2_ul_u2;

        res.low2_ul_br =  low2_ul_br;
        res.low2_ul_u2 = low2_ul_u2;

        res.dt_ul_br =  dt_ul_br;
        res.dt_ul_u2 = dt_ul_u2;

        res.dt_ll_br =  dt_ll_br;
        res.dt_ll_u2 = dt_ll_u2;

        return res;
    }

    double Limiter::get_background(double sig_min, double sig_max )
    {
        double total_background = 0;

        for ( auto& chan : bg_channels_ )
        {
            double chan_background = 0;
            for ( auto& pol : pols_ )
            {
                for ( auto& region : regions_ )
                {
                    auto h = get_bg_hist( chan, pol, region);

                    //treat negative values conservatively

                    double contrib = integral( *h, sig_min, sig_max );

                    chan_background +=  contrib;
                }
            }

            if ( chan_background < 0 )
            {
                std::cerr << "Ignoring negative value for "
                    << chan << std::endl;
            }

            total_background += chan_background;
        }

        return total_background;
    }

    double Limiter::get_data(double sig_min, double sig_max )
    {
        double total_data = 0;

        for ( auto& pol : pols_ )
        {
            for ( auto& region : regions_ )
            {
                auto h = get_dt_hist(  pol, region);
                total_data += integral( *h, sig_min, sig_max );

            }
        }
        return total_data;
    }

    void Limiter::set_bg_channels( std::vector<std::string> bgchans )
    {
        bg_channels_ = bgchans;
    }

    std::unique_ptr<TH1> Limiter::get_bg_hist
        ( std::string chan, std::string pol, std::string region )
        {
            auto p = boost::filesystem::path{ pol} /region / "h_m2m_kmu/hnu_stack_hists" / ( chan + "_" + pol ); 
            auto h = extract_hist( bg_file_, p );
            return h;
        }

    std::unique_ptr<TH1> Limiter::get_dt_hist
        ( std::string pol,  std::string region )
        {
            auto p = boost::filesystem::path{ pol} /region / "h_m2m_kmu/hdata" ;
            auto h = extract_hist( bg_file_, p );
            return h;
        }

    std::pair<double,double> Limiter::get_trig_eff( double sig_min, double sig_max )
    {
        if ( !trig_ )
        {
            throw std::runtime_error( "No trigger set in Limiter" );
        }

        return (*trig_)->get_eff_err( sig_min, sig_max );
    }

    std::pair<double,double> Limiter::get_km2_flux_and_err()
    {
        double total_flux = 0;
        double total_sqerr = 0;

        for ( const auto& flux_path : km2_flux_paths_ )
        {
            path p{ flux_path };

            total_flux += retrieve_value( bg_file_ , p / "fiducial_flux" );

            total_sqerr += fn::sq( retrieve_value( bg_file_ , p / "fiducial_flux_err" ) );
        }

        //Add a systematic error of 0.5%
        total_sqerr +=  fn::sq(0.005 * total_flux);

        double total_err = std::sqrt( total_sqerr );
        return std::make_pair( total_flux, total_err );
    }
    //--------------------------------------------------

    HaloErrors::HaloErrors( TFile& tfbg, TFile& tfhalolog )
        :tfbg_( tfbg ), tfhalolog_( tfhalolog )
    {}

    void HaloErrors::set_halo_info( std::vector<halo_info> info )
    {
        halo_info_sets_ = info;
        halo_resources_.clear();

        for ( const auto& info : halo_info_sets_ )
        {
            double halo_scale = 
                retrieve_value( tfbg_, path{ info.scale_path } / "halo_scale" ) ;

            double k3pi_scale 
                = retrieve_value( tfhalolog_, path{info.halo_log_path} / "k3pi_scale_factor" );

            path log_folder = info.halo_log_path;

            auto h_raw   = extract_hist<TH1D>( tfhalolog_, log_folder / "hraw" );
            auto h_corr  = extract_hist<TH1D>( tfhalolog_, log_folder / "hcorr" );
            auto h_peak  = extract_hist<TH1D>( tfhalolog_, log_folder / "hpeak" );
            auto h_final = extract_hist<TH1D>( tfbg_, info.bg_path );

            halo_resource hr;
            hr.bg_path = info.bg_path;
            hr.scale_path = info.scale_path;
            hr.halo_log_path = info.halo_log_path;

            hr.halo_scale = halo_scale;
            hr.k3pi_scale = k3pi_scale;

            hr.h_raw = h_raw.get();
            hr.h_corr = h_corr.get();
            hr.h_peak = h_peak.get();
            hr.h_final = h_final.get();

            hstore_.push_back( std::move( h_raw ) );
            hstore_.push_back( std::move( h_corr ) );
            hstore_.push_back( std::move( h_peak ) );
            hstore_.push_back( std::move( h_final ) );

            halo_resources_.push_back( hr );
        }
    }


    double HaloErrors::compute_halo_scale_err( double sig_min, double sig_max ) const
    {
        double total_sqerr = 0;

        for ( const auto & info : halo_info_sets_ )
        {
            double halo_scale = 
                retrieve_value( tfbg_, path{ info.scale_path } / "halo_scale" ) ;

            double halo_scale_err = 
                retrieve_value( tfbg_, path{ info.scale_path } / "halo_scale_err" ) ;

            auto h_halo = extract_hist<TH1D>( tfbg_, info.bg_path );
            double halo_contrib = integral( *h_halo, sig_min, sig_max );
            total_sqerr += fn::sq( halo_contrib * halo_scale_err / halo_scale );
            total_sqerr += fn::sq( 0.1 * halo_contrib );

        }

        return std::sqrt( total_sqerr );
    }

    double HaloErrors::compute_halo_val_err( double sig_min, double sig_max ) const
    {
        double total_sqerr = 0;

        for ( const auto& hr : halo_resources_ )
        {
            //double halo_scale = retrieve_value( tfbg_, path{ info.scale_path } / "halo_scale" ) ;
            double halo_scale = hr.halo_scale;

           // double k3pi_scale = retrieve_value( tfhalolog_, path{info.halo_log_path} / "k3pi_scale_factor" );
            double k3pi_scale = hr.k3pi_scale;

            path log_folder = hr.halo_log_path;

            //redo calculation from halo_sub
            //auto h_raw   = get_object<TH1D>( tfhalolog_, log_folder / "hraw" );
            //auto h_corr  = get_object<TH1D>( tfhalolog_, log_folder / "hcorr" );
            //auto h_peak  = get_object<TH1D>( tfhalolog_, log_folder / "hpeak" );
            auto h_raw = hr.h_raw;
            auto h_corr = hr.h_corr;
            auto h_peak = hr.h_peak;

            double n_raw   = integral( *h_raw, sig_min, sig_max );
            double n_corr  = integral( *h_corr, sig_min, sig_max );
            //peak was already scaled by k3pi_scale
            double n_peak  = integral( *h_peak, sig_min, sig_max ) / k3pi_scale;

            double n_check  = halo_scale * ( n_raw - k3pi_scale * n_peak );

            //compare with what is stored in the computed bg file
            //auto h_final = get_object<TH1D>( tfbg_, info.bg_path );
            auto h_final = hr.h_final;
            double n_final = integral( *h_final, sig_min, sig_max );

            //If there is a mismatch print everything 
            double prec = 0.00001;
            if ( fabs(n_check - n_final) > prec * fabs( n_check + n_final) ) 
            {
                throw std::runtime_error( "Mismatch redoing halo calculation" );
                print_mismatch( std::cerr, sig_min, sig_max, halo_scale,
                        n_raw, n_corr, n_peak, k3pi_scale, n_check, n_final );
            }

            double k3pi_scale_rel_err = 0.03;

            //now do the actual error calculation
            total_sqerr += fn::sq(halo_scale) * 
                ( n_raw  + fn::sq(k3pi_scale) * n_peak * ( n_peak * k3pi_scale_rel_err + 1.0 ) );
        }

        return std::sqrt(total_sqerr);
    }
    void HaloErrors::print_mismatch( std::ostream & os ,
            double sig_min, double sig_max,
            double halo_scale, double n_raw,
            double n_corr, double n_peak,
            double k3pi_scale, double n_check,
            double n_final ) const
    {
        os
            << "range: " << sig_min << " " << sig_max << "\n"
            << "halo_scale: " << halo_scale  << "\n"
            << "n_raw: " << n_raw  << "\n"
            << "n_corr: " << n_corr << "\n"
            << "n_peak: " << n_peak << "\n"
            << "k3pi_scale: " << k3pi_scale << "\n"
            << "n_check: " << std::setprecision(10) << n_check << "\n"
            << "n_final: " << std::setprecision(10) << n_final << "\n"
            << std::endl;

        uint64_t u_check;
        uint64_t u_final;
        memcpy(&u_check, &n_check, sizeof(n_check));
        memcpy(&u_final, &n_final, sizeof(n_final));
        os << std::hex << u_check << "\n" << u_final << std::endl;
    }

    //--------------------------------------------------
    double Limiter::compute_halo_scale_err( double sig_min, double sig_max )
    {
        if ( halo_errors_ )
        {
            return (*halo_errors_)->compute_halo_scale_err( sig_min, sig_max );
        }
        else
        {
            return 0;
        }
    }

    double Limiter::compute_halo_val_err( double sig_min, double sig_max )
    {
        if ( halo_errors_ )
        {
            return (*halo_errors_)->compute_halo_val_err( sig_min, sig_max );
        }
        else
        {
            return 0;
        }
    }

    void Limiter::set_halo_errors( const HaloErrors& he )
    {
        halo_errors_ = &he;
    }

    void Limiter::set_scatter_contrib( const ScatterContrib& sc )
    {
        scat_contrib_ = &sc;
    }

    void Limiter::set_trigger( const TriggerApp& trig )
    {
        trig_ = &trig;
    }

    //--------------------------------------------------


    double br_to_mix(double BR, double mh)
    {
        static double brkm2(0.633);
        double mK = na62const::mK;
        double mMu = na62const::mMu;

        auto sq = []( double n ){ return n * n ; } ;

        double f = sq(mh) * ( 1 - sq(mh/mK) +2 * (mMu/mK) + sq(mMu/mh)*(1- sq(mMu/mK)))
            / sq(mMu) *sq(1-sq(mMu/mK)) * sqrt(sq(1+sq(mh/mK) - sq(mMu/mK)) - 4 * sq(mh/mK));

        return (BR / brkm2) / f;
    }

    //--------------------------------------------------

}
