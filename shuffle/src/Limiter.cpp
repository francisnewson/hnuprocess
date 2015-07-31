#include "Limiter.hh"
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
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
        return get_mean() - 0.5 * get_width();
    }

    double HnuLimParams::get_width_max() const
    {
        return get_mean() + 0.5 * get_width();
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

        width_ = h_sig_->GetXaxis()->GetBinUpEdge( max_bin )
            - h_sig_->GetXaxis()->GetBinLowEdge( min_bin );

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

    double ScatterContrib::get_scatter_err( double sig_min, double sig_max ) const
    {
        double noscat = integral( *h_noscat_, sig_min, sig_max );
        double scat = integral( *h_scat_, sig_min, sig_max );
        return (scat - noscat) / scat;
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

        //**************************************************
        //Determine background 
        //**************************************************

        //Determine #background events
        double background = get_background( sig_min, sig_max );

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
        sqr_background_err = fn::sq( trig_err / trig_eff * background );

        //scattering
        if ( scat_contrib_ )
        {
            double scat = (*scat_contrib_)->get_scatter_err( sig_min, sig_max );
            std::cout << "scat: "<< scat  << std::endl;
            double scat_err = background * scat;
            sqr_background_err +=  fn::sq( scat_err );
        }

        //muv_eff
        double muv_err = 0.01 * background;
        sqr_background_err += fn::sq( muv_err );

        //km2 flux
        std::pair<double,double> km2_flux_and_err = get_km2_flux_and_err();
        double km2_flux = km2_flux_and_err.first;
        double km2_flux_err = km2_flux_and_err.second;
        sqr_background_err += fn::sq( km2_flux_err / km2_flux * background );

        //halo
        double halo_scale_err = compute_halo_scale_err( sig_min, sig_max );
        sqr_background_err += fn::sq( halo_scale_err );

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
        rolke_eff.SetGaussBkgKnownEff( background, background, background_err , params.get_width_acceptance() );
        double ul = rolke_eff.GetUpperLimit();

        //Convert for literature
        double ul_br = ul / km2_flux;
        double ul_u2 = br_to_mix( ul_br, 0.001 * params.get_mass()  ); //MeV -> GeV

        //Ouput object
        HnuLimResult res{};
        res.trig_eff = trig_eff;
        res.trig_err = trig_err;
        res.background = background;
        res.background_err = background_err ;
        res.acc_sig_ul = ul;
        res.orig_sig_ul = ul / params.get_width_acceptance();
        res.rolke_orig_sig_ul = ul;
        res.ul_br =  ul_br;
        res.ul_u2 = ul_u2;

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
        const TVectorD * vd = 0;

        double total_flux = 0;
        double total_sqerr = 0;

        for ( const auto& flux_path : km2_flux_paths_ )
        {
            path p{ flux_path };
            p /= "fiducial_flux" ;

            vd = get_object<TVectorD>( bg_file_, p / "fiducial_flux" );
            total_flux += (*vd)[0];

            vd = get_object<TVectorD>( bg_file_, p / "fiducial_flux_err" );
            total_sqerr += (*vd)[0];
        }

        double total_err = std::sqrt( total_sqerr );
        return std::make_pair( total_flux, total_err );
    }

    double Limiter::compute_halo_scale_err( double sig_min, double sig_max )
    {
        std::vector<std::pair<std::string,std::string>> halo_scale_pairs
        {
            { "neg/signal/lower_muv/hnu_stack_hists/halo_neg", "neg_lower" },
                { "neg/signal/upper_muv/hnu_stack_hists/halo_neg", "neg_upper" },
                { "pos/signal/lower_muv/hnu_stack_hists/halo_pos", "pos_lower" },
                { "pos/signal/upper_muv/hnu_stack_hists/halo_pos", "pos_upper" },
        };

        double total_sqerror = 0;

        for ( auto hp : halo_scale_pairs )
        {
            double halo_scale 
                = retrieve_value( bg_file_, path{hp.second} / "halo_scale" );

            double halo_scale_err 
                = retrieve_value( bg_file_, path{hp.second} / "halo_scale_err" );

            auto h_halo = extract_hist<TH1D>( bg_file_, hp.first );
            double halo_contrib = integral( *h_halo, sig_min, sig_max );
            total_sqerror += fn::sq(  halo_contrib * halo_scale_err / halo_scale );
        }

        return std::sqrt( total_sqerror );
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

    double retrieve_value( TFile& tf , boost::filesystem::path name )
    {
        const TVectorD * vd = get_object<TVectorD>( tf, name );
        return (*vd)[0];
    }
}
