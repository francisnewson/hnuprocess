#include "easy_app.hh"
#include "root_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "fiducial_functions.hh"
#include "TFile.h"
#include "Limiter.hh"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <boost/io/ios_state.hpp>
#include <boost/regex.hpp>

//print individual result to screen (human readable)
void print_result( std::ostream& os, const HnuLimParams& hlp, const HnuLimResult& hlr )
{
    boost::io::ios_flags_saver fs( os );
    os.setf(std::ios::fixed, std::ios::floatfield);
    os.precision(4);

    os << "===========================================\n";
    os << hlp.get_chan() << "\n";
    os << "-------------------------------------------\n";
    os << "Mean: " << hlp.get_mass() << " MeV/c^2 -> " << hlp.get_mean() << " GeV^2/c^4\n";
    os << "Width: " << hlp.get_width() << "\n";
    os << "Acc: " << hlp.get_full_acceptance() << " ->  " 
        << hlp.get_width_acceptance() << " ± " << hlp.get_width_acceptance_err() << "\n";
    os << "-------------------------------------------\n";
    os << "Trig eff: " << hlr.trig_eff << " ± " << hlr.trig_err << "\n";
    os << "Sig range: " << hlr.sig_min << " - " << hlr.sig_max << "\n";
    os << "Background: " << hlr.background << " ± " << hlr.background_err << "\n" ;
    os << "sqrt(Background): " << std::sqrt(hlr.background ) << "\n" ;
    os << "Data observed: " << hlr.dt_obs << "\n";
    os << "Acc Sig limit: " << hlr.acc_sig_ul << "\n";
    os << "Orig Sig limit: " << hlr.orig_sig_ul << " " << hlr.rolke_orig_sig_ul << "\n";
    os.setf(std::ios_base::scientific, std::ios_base::floatfield);
    os << "BR limit: " << hlr.ul_br  << " U2 limit: " << hlr.ul_u2 << "\n";
    os << "Dt BR limit: " <<  hlr.dt_ul_br << " Dt U2 limit: " << hlr.dt_ul_u2 <<  "\n";

    //Determine longest errname
    std::vector<std::string> names;
    std::vector<double> lengths;
    std::vector<double> values;
    const auto& err_budget = hlr.error_budget;
    for ( const auto& p : err_budget )
    {
        names.push_back( p.first );
        lengths.push_back( p.first.size() );
        values.push_back( p.second );
    }
    double max_length = *std::max_element( begin( lengths ), end( lengths ) );
    for ( const auto & name : names )
    { os << std::setw( max_length + 2 ) << name ; }
    os << "\n";

    for ( const auto & val : values )
    { os << std::setw( max_length + 2 ) << std::sqrt(val) ; }

    os << "\n";
}

//headings for writing to file (script readable)
void print_headings( std::ostream& os )
{
    os 
        << std::setw(20) << "channel"
        << std::setw(15) << "mass"
        << std::setw(15) << "mean"
        << std::setw(15) << "sqrt(mean)"
        << std::setw(15) << "full_acc"
        << std::setw(15) << "width"
        << std::setw(15) << "final_acc"
        << std::setw(15) << "final_acc_err"
        << std::setw(15) << "trig_eff"
        << std::setw(15) << "trig_err"
        << std::setw(15) << "bg"
        << std::setw(15) << "bg_err"
        << std::setw(15) << "dt_obs"
        << std::setw(15) << "ul_acc_sig"
        << std::setw(15) << "ul_orig_sig"
        << std::setw(15) << "ul_br"
        << std::setw(15) << "ul_u2"
        << std::setw(15) << "high1_ul_br"
        << std::setw(15) << "high1_ul_u2"
        << std::setw(15) << "low1_ul_br"
        << std::setw(15) << "low1_ul_u2"
        << std::setw(15) << "high2_ul_br"
        << std::setw(15) << "high2_ul_u2"
        << std::setw(15) << "low2_ul_br"
        << std::setw(15) << "low2_ul_u2"
        << std::setw(15) << "dt_ul_br"
        << std::setw(15) << "dt_ul_u2"
        << std::setw(15) << "dt_ll_u2"
        << std::setw(15) << "err_scat"
        << std::setw(15) << "err_trig"
        << std::setw(15) << "err_muv"
        << std::setw(15) << "err_flux"
        << std::setw(15) << "err_hscale"
        << std::setw(15) << "err_hval"
        << std::endl;
}

//writing to file (script readable)
void print_flat_result( std::ostream& os, const HnuLimParams& hlp, const HnuLimResult& hlr )
{
    boost::io::ios_flags_saver fs( os );
    os.setf(std::ios::fixed, std::ios::floatfield);
    os.precision(5);

    os 
        << std::setw(20) << hlp.get_chan()
        << std::setw(15) << hlp.get_mass()
        << std::setw(15) << hlp.get_mean()
        << std::setw(15) << std::sqrt(hlp.get_mean() )
        << std::setw(15) << hlp.get_full_acceptance()
        << std::setw(15) << hlp.get_width()
        << std::setw(15) << hlp.get_width_acceptance()
        << std::setw(15) << hlp.get_width_acceptance_err()
        << std::setw(15) << hlr.trig_eff
        << std::setw(15) << hlr.trig_err
        << std::setw(15) << hlr.background
        << std::setw(15) << hlr.background_err
        << std::setw(15) << hlr.dt_obs;

    os.setf(std::ios_base::scientific,
            std::ios_base::floatfield);

    os
        << std::setw(15) <<  hlr.acc_sig_ul
        << std::setw(15) <<  hlr.orig_sig_ul
        << std::setw(15) <<  hlr.ul_br
        << std::setw(15) <<  hlr.ul_u2
        << std::setw(15) <<  hlr.high1_ul_br
        << std::setw(15) <<  hlr.high1_ul_u2
        << std::setw(15) <<  hlr.low1_ul_br
        << std::setw(15) <<  hlr.low1_ul_u2
        << std::setw(15) <<  hlr.high2_ul_br
        << std::setw(15) <<  hlr.high2_ul_u2
        << std::setw(15) <<  hlr.low2_ul_br
        << std::setw(15) <<  hlr.low2_ul_u2
        << std::setw(15) <<  hlr.dt_ul_br
        << std::setw(15) <<  hlr.dt_ul_u2
        << std::setw(15) <<  hlr.dt_ll_u2
        << std::setw(15) <<  hlr.error_budget.at("err_scat")
        << std::setw(15) <<  hlr.error_budget.at("err_trig")
        << std::setw(15) <<  hlr.error_budget.at("err_muv")
        << std::setw(15) <<  hlr.error_budget.at("err_flux")
        << std::setw(15) <<  hlr.error_budget.at("err_hscale")
        << std::setw(15) <<  hlr.error_budget.at("err_hval")
        << std::endl;
}

int main()
{
    //**************************************************
    //Signal shapes
    //**************************************************

    //Decide which masses to look at
    std::vector<int> masses;
    for( int m = 250 ; m != 380 ; m += 1 )
    { masses.push_back( m ); }

    //Load acceptance plots
    std::string acc_filename = "tdata/km2_acc/all.km2_acc.root" ;
    auto fid_weights = extract_fiducial_weights
        ( acc_filename, "", "sample_burst_count/bursts", "weight"  );

    //set up acceptance files
    TFile tfin_acc {acc_filename.c_str()};
    TFile tfout_acc{ "output/signal_shapes.root", "RECREATE" };

    //**************************************************
    //Background estimates
    //**************************************************

    //Decide which backgrounds to consider
    std::vector<std::string> background_channels
    { "halo", "k3pi", "k2pi", "k3pi0", "km3", "km2" };

    //Load background file
    std::string bg_filename = "tdata/staging/all.mass_plots.root";
    TFile tfin_bg{ bg_filename.c_str() };
    std::vector<std::string> signal_regions { "signal_upper_muv", "signal_lower_muv" };

    Limiter lm{ tfin_bg, signal_regions };
    lm.set_bg_channels( background_channels);

    //Trigger corrections
    std::string trig_filename = "tdata/halo_control/all.halo_control.root" ;
    TFile tfin_trig{ trig_filename.c_str() };

    TriggerApp trig( tfin_trig );
    trig.set_sels( { 
            "p5.data.q1.v4.neg/sig_up_trig_eff",
            "p5.data.q1.v4.neg/sig_dn_trig_eff",
            "p5.data.q1.v4.pos/sig_up_trig_eff",
            "p5.data.q1.v4.pos/sig_dn_trig_eff"
            } );
    trig.init();

    lm.set_trigger( trig );

    //Scattering
    TFile tfin_scat { "tdata/staging/km2_scat.root"};
    TFile tfin_noscat { "tdata/staging/km2_noscat.root"};
    ScatterContrib sc{ tfin_noscat, tfin_scat };

    lm.set_scatter_contrib( sc );

    //halo errors
    TFile tf_halo_log{  "tdata/staging/log/halo_sub_log.q11t.root"  };

    HaloErrors he{ tfin_bg, tf_halo_log };
    he.set_halo_info( {
            { "neg/signal_lower_muv/h_m2m_kmu/hnu_stack_hists/halo_neg", "neg_lower", "p6.halo.q11t.neg/signal_lower_muv_plots/h_m2m_kmu" },
            { "pos/signal_lower_muv/h_m2m_kmu/hnu_stack_hists/halo_pos", "pos_lower", "p6.halo.q11t.pos/signal_lower_muv_plots/h_m2m_kmu" } ,
            { "neg/signal_upper_muv/h_m2m_kmu/hnu_stack_hists/halo_neg", "neg_upper", "p6.halo.q11t.neg/signal_upper_muv_plots/h_m2m_kmu" },
            { "pos/signal_upper_muv/h_m2m_kmu/hnu_stack_hists/halo_pos", "pos_upper", "p6.halo.q11t.pos/signal_upper_muv_plots/h_m2m_kmu" } } );

    lm.set_halo_errors( he );

    //Output streams
    std::ofstream ofs( "output/lim.txt" );
    print_headings( ofs );
    std::ostream& os = std::cout;

    for ( const auto& mass : masses )
    {
        try
        {
            //Set up signal parameters
            HnuLimParams hlp( tfin_acc, fid_weights, signal_regions, mass );

            //write acceptance plots
            tfout_acc.cd();
            hlp.write();

            //Get limits
            auto limres = lm.get_limit( hlp );

            //Print limit results
            print_result( os, hlp, limres );
            print_flat_result( ofs, hlp, limres );
        }
        catch (std::exception & e)
        {
            std::cerr << "Problem with " << mass << e.what() << std::endl;
            continue;
        }
    }
}
