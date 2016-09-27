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
            int mass, double nsigma_width, bool fancyfit )
        :tf_(tf), fid_weights_(fid_weights), regions_( regions ),
        mass_( mass ), nsigma_width_( nsigma_width), fancy_fit_( fancyfit )
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
        //std::vector<std::string> regs { "signal_upper_muv", "signal_lower_muv" };

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

        std::cout << "hnu fid_weight: " << fid_total_ << std::endl;

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

        //Find 1 sigma width
        while ( cum_sum < 0.6827 * sig_total_ )
        {
            --min_bin;
            ++max_bin;
            cum_sum = h_sig_->Integral( min_bin, max_bin );
        }

        width_low_edge_ = h_sig_->GetXaxis()->GetBinLowEdge( min_bin );
        width_up_edge_ = h_sig_->GetXaxis()->GetBinUpEdge( max_bin );

        width_ = width_up_edge_ - width_low_edge_;

        //stretch according to nsigma_width
        width_ = nsigma_width_ * width_;
        min_bin = h_sig_->GetXaxis()->FindBin( mean_ - width_ / 2 );
        max_bin = h_sig_->GetXaxis()->FindBin( mean_ + width_ / 2 );
        width_low_edge_ = h_sig_->GetXaxis()->GetBinLowEdge( min_bin );
        width_up_edge_ = h_sig_->GetXaxis()->GetBinUpEdge( max_bin );

        //width_total_ = cum_sum;
        width_total_ = h_sig_->Integral( min_bin, max_bin );
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
            km2_flux_paths_ = std::vector<std::string> { 
                "scalefactors/neg_ext_lower",
                    "scalefactors/pos_ext_lower" };
        }

    std::map<std::string, double> Limiter::get_channel_backgrounds( const HnuLimParams& params )
    {
        //Deterimine signal region
        double sig_min = params.get_width_min();
        double sig_max = params.get_width_max();

        return get_channel_backgrounds( sig_min, sig_max );
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
        double raw_trig_err = trig_eff_err.second;
        background *= trig_eff;

        //**************************************************
        //Determine background error
        //**************************************************
        double sqr_background_err  = 0;

        //triggger eff
        double sq_trig_eff_contrib = fn::sq( raw_trig_err / trig_eff * background );
        res.raw_trig_sqerr = sq_trig_eff_contrib;
        sqr_background_err += sq_trig_eff_contrib;

        //scattering
        double sq_scat_contrib =  0;
        if ( scat_contrib_ )
        {
            //std::cerr << "DOING SCAT" << std::endl;
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

        double sq_halo_k3pi_err = fn::sq( compute_halo_k3pi_err( sig_min, sig_max ) );
        sqr_background_err +=  sq_halo_k3pi_err;
        res.error_budget["err_k3pi"] = sq_halo_k3pi_err;

        //halo shape err
        double sq_halo_shape_err = fn::sq( compute_halo_shape_err( sig_min, sig_max ) );
        sqr_background_err += sq_halo_shape_err;
        res.error_budget["err_hshape"] = sq_halo_shape_err;

        //square here becuase we sqrt everyting later
        res.error_budget["halototal" ] =fn::sq(get_halo_val( sig_min, sig_max ));

        double sq_km3_br_err = fn::sq( compute_km3_br_err(  sig_min,  sig_max ));
        sqr_background_err += sq_km3_br_err;
        res.error_budget["err_km3" ] =sq_km3_br_err;

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

#if 0
        //Dodgy efficiency acceptance
        double dodge_eff = trig_eff *  params.get_width_acceptance();
        double dodge_eff_err = trig_err * params.get_width_acceptance();
#endif

        double acceptance = params.get_width_acceptance();

        res.ses_br =  1 / (acceptance * km2_flux);

        //std::cout << "bg before trig dodge: " << background << std::endl;
        //std::cout << "bg_err before trig dodge: " << background_err << std::endl;
        //std::cout << "sq_trig_eff_contrib before trig dodge: " << sq_trig_eff_contrib << std::endl;

        //**************************************************
        //Dodgy trig eff compensation
        //**************************************************

        //compute ul for signal at 90% CL without extra trig
        {
            TRolke raw_rolke_eff;
            raw_rolke_eff.SetBounding(true);
            raw_rolke_eff.SetCL( 0.9 );
            raw_rolke_eff.SetGaussBkgKnownEff( background, background, background_err , params.get_width_acceptance() );
            double raw_ul = raw_rolke_eff.GetUpperLimit();
            double raw_ul_mu = raw_ul * acceptance;
            double sq_extra_trig = fn::sq( raw_ul_mu  / ( 1  - raw_ul_mu  / background) * ( 1 - trig_eff ) );

            sq_trig_eff_contrib += sq_extra_trig;
            background_err = std::sqrt( sqr_background_err + sq_extra_trig );

            res.error_budget["err_trig"] = sq_trig_eff_contrib;

            // std::cout << "raw_ul: " <<  raw_ul << std::endl;
            // std::cout << "raw_ul_mu: " <<  raw_ul_mu << std::endl;
            // std::cout << "trig_rel_err: " <<  raw_ul_mu  / ( 1 - raw_ul_mu / background ) * ( 1 - trig_eff ) /background  << std::endl;
            // std::cout << "sq_extra_trig: " << sq_extra_trig << std::endl;
            // std::cout << "sq_trig_eff_contrib after trig dodge: " << sq_trig_eff_contrib << std::endl;
            // std::cout << "bg_err after trig dodge: " << background_err << std::endl;
        }

        //**************************************************
        //Compute limits included extra trigger error
        //**************************************************

        //Rolke does the heavy lifting
        TRolke rolke_eff;
        rolke_eff.SetBounding(true);
        rolke_eff.SetCL( 0.9 );
        rolke_eff.SetGaussBkgKnownEff( background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( background, background, dodge_eff, dodge_eff_err,   background_err );
        double ul = rolke_eff.GetUpperLimit();

        double comb_sigma = std::sqrt( background + fn::sq(background_err ) );

        res.dt_pull = (data - background) / comb_sigma;

        //do brazil
        double high1_background = background + std::sqrt( background + fn::sq(background_err ) );
        rolke_eff.SetGaussBkgKnownEff( high1_background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( high1_background, background, dodge_eff, dodge_eff_err,   background_err );
        double high1_ul = rolke_eff.GetUpperLimit();

        double low1_background = background - std::sqrt( background  + fn::sq( background_err ) );
        rolke_eff.SetGaussBkgKnownEff( low1_background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( low1_background, background, dodge_eff, dodge_eff_err,   background_err );
        double low1_ul = rolke_eff.GetUpperLimit();

        double high2_background = background + 2 *  std::sqrt( background + fn::sq( background_err ) ) ;
        rolke_eff.SetGaussBkgKnownEff( high2_background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( high2_background, background, dodge_eff, dodge_eff_err,   background_err );
        double high2_ul = rolke_eff.GetUpperLimit();

        double low2_background = background - 2 * std::sqrt( background + fn::sq( background_err ) );
        rolke_eff.SetGaussBkgKnownEff( low2_background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( low2_background, background, dodge_eff, dodge_eff_err,   background_err );
        double low2_ul = rolke_eff.GetUpperLimit();

        double high3_background = background + 3 *  std::sqrt( background + fn::sq( background_err ) ) ;
        rolke_eff.SetGaussBkgKnownEff( high3_background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( high3_background, background, dodge_eff, dodge_eff_err,   background_err );
        double high3_ul = rolke_eff.GetUpperLimit();

        double low3_background = background - 3 * std::sqrt( background + fn::sq( background_err ) );
        rolke_eff.SetGaussBkgKnownEff( low3_background, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( low3_background, background, dodge_eff, dodge_eff_err,   background_err );
        double low3_ul = rolke_eff.GetUpperLimit();

        //do data
        rolke_eff.SetGaussBkgKnownEff( data, background, background_err , params.get_width_acceptance() );
        //rolke_eff.SetGaussBkgGaussEff( data , background, dodge_eff, dodge_eff_err,   background_err );
        double dt_ul =  rolke_eff.GetUpperLimit();
        double dt_ll =  rolke_eff.GetLowerLimit();

        res.ses_u2 = br_to_mix( res.ses_br, 0.001 * params.get_mass() );

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

        double high3_ul_br = high3_ul / km2_flux;
        double high3_ul_u3 = br_to_mix( high3_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double low3_ul_br = low3_ul / km2_flux;
        double low3_ul_u3 = br_to_mix( low3_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double dt_ul_br = dt_ul / km2_flux;
        double dt_ul_u2 = br_to_mix( dt_ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        double dt_ll_br = dt_ll / km2_flux;
        double dt_ll_u2 = br_to_mix( dt_ll_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        res.kaon_flux = km2_flux;

        res.trig_eff = trig_eff;
        res.raw_trig_err = raw_trig_err;
        res.background = background;
        res.background_err = background_err ;
        res.dt_obs = data;
        res.acc_sig_ul = ul * params.get_width_acceptance();
        res.orig_sig_ul = ul;
        res.rolke_orig_sig_ul = ul;


        res.ul_mu =  ul * acceptance;
        res.ul_br =  ul_br;
        res.ul_u2 = ul_u2;

        res.comb_sigma = comb_sigma;

        res.high1_ul_mu = high1_ul * acceptance;
        res.high1_ul_br =  high1_ul_br;
        res.high1_ul_u2 = high1_ul_u2;

        res.low1_ul_mu = low1_ul * acceptance;
        res.low1_ul_br =  low1_ul_br;
        res.low1_ul_u2 = low1_ul_u2;

        res.high2_ul_mu = high2_ul * acceptance;
        res.high2_ul_br =  high2_ul_br;
        res.high2_ul_u2 = high2_ul_u2;

        res.low2_ul_mu = low2_ul * acceptance ;
        res.low2_ul_br =  low2_ul_br;
        res.low2_ul_u2 = low2_ul_u2;

        res.high3_ul_mu = high3_ul * acceptance;
        res.high3_ul_br =  high3_ul_br;
        res.high3_ul_u2 = high3_ul_u3;

        res.low3_ul_mu = low3_ul * acceptance ;
        res.low3_ul_br =  low3_ul_br;
        res.low3_ul_u2 = low3_ul_u3;

        res.dt_ul_mu = dt_ul * acceptance;
        res.dt_ul_br =  dt_ul_br;
        res.dt_ul_u2 = dt_ul_u2;

        res.dt_ll_mu = dt_ll * acceptance;
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
            else
            {
                total_background += chan_background;
            }
        }

        return total_background;
    }
    double Limiter::compute_km3_br_err( double sig_min, double sig_max )
    {
        double total_background = 0;
        std::string chan = "km3";
        for ( auto& pol : pols_ )
        {
            for ( auto& region : regions_ )
            {
                auto h = get_bg_hist( chan, pol, region);
                double contrib = integral( *h, sig_min, sig_max );
                total_background +=  contrib;
            }
        }

        return 0.01 * total_background;
    }

    std::map<std::string,double> Limiter::get_channel_backgrounds(double sig_min, double sig_max )
    {
        double total_background = 0;
        std::map<std::string,double> result;

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

            result.insert( std::make_pair( chan, chan_background ) );
            total_background += chan_background;
        }

        return result;
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

    void HaloErrors::set_shape_error_factor( double shape_error_factor )
    {
        shape_error_factor_ = shape_error_factor;
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
            //total_sqerr += fn::sq( 0.1 * halo_contrib );
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

            double k3pi_scale_rel_err = 0.02 / 0.561;

            //now do the actual error calculation
            //total_sqerr += fn::sq(halo_scale) * 
            //   ( n_raw  + fn::sq(k3pi_scale) * n_peak * ( n_peak * k3pi_scale_rel_err + 1.0 ) );

            total_sqerr += fn::sq(halo_scale) * ( n_raw  + fn::sq(k3pi_scale) * n_peak );
            //total_sqerr += fn::sq(halo_scale * k3pi_scale * n_peak * k3pi_scale_rel_err );
        }

        return std::sqrt(total_sqerr);
    }

    double HaloErrors::compute_halo_k3pi_err( double sig_min, double sig_max ) const
    {
        double total_sqerr = 0;

        for ( const auto& hr : halo_resources_ )
        {
            double halo_scale = hr.halo_scale;
            double k3pi_scale = hr.k3pi_scale;
            path log_folder = hr.halo_log_path;

            auto h_raw = hr.h_raw;
            auto h_corr = hr.h_corr;
            auto h_peak = hr.h_peak;

            double n_raw   = integral( *h_raw, sig_min, sig_max );
            double n_corr  = integral( *h_corr, sig_min, sig_max );

            //peak was already scaled by k3pi_scale
            double n_peak  = integral( *h_peak, sig_min, sig_max ) / k3pi_scale;
            double n_check  = halo_scale * ( n_raw - k3pi_scale * n_peak );

            //compare with what is stored in the computed bg file
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

            double k3pi_scale_rel_err = 0.02 / 0.561;
            //double k3pi_scale_rel_err = 0.20 / 0.561;
            //double k3pi_scale_rel_err = 0.06 / 0.561;
            total_sqerr += fn::sq(halo_scale * k3pi_scale * n_peak * k3pi_scale_rel_err );
        }

        return std::sqrt(total_sqerr);
    }

    double HaloErrors::get_halo_val( double sig_min, double sig_max ) const
    {
        double total = 0;
        for ( const auto& hr : halo_resources_ )
        {
            auto h_final = hr.h_final;
            double n_final = integral( *h_final, sig_min, sig_max );
            total += n_final;
        }
        return total;
    }

    double HaloErrors::compute_halo_shape_err( double sig_min, double sig_max ) const
    {
        if ( ! shape_error_factor_ )
        {
            return 0;
        }

        return get_halo_val( sig_min, sig_max ) * (*shape_error_factor_);
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

    double Limiter::compute_halo_k3pi_err( double sig_min, double sig_max )
    {
        if ( halo_errors_ )
        {
            return (*halo_errors_)->compute_halo_k3pi_err( sig_min, sig_max );
        }
        else
        {
            return 0;
        }
    }

    double Limiter::compute_halo_shape_err( double sig_min, double sig_max )
    {
        if ( halo_errors_ )
        {
            return (*halo_errors_)->compute_halo_shape_err( sig_min, sig_max );
        }
        else
        {
            return 0;
        }
    }
    double Limiter::get_halo_val( double sig_min, double sig_max )
    {
        if ( halo_errors_ )
        {
            return (*halo_errors_)->get_halo_val( sig_min, sig_max );
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
