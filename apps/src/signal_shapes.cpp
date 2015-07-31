#include "easy_app.hh"
#include "root_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "fiducial_functions.hh"
#include "TFile.h"
#include "Limiter.hh"
#include <cmath>
#include <numeric>
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
    os << "Background: " << hlr.background << " ± " << hlr.background_err << "\n" ;
    os << "Acc Sig limit: " << hlr.acc_sig_ul << "\n";
    os << "Orig Sig limit: " << hlr.orig_sig_ul << " " << hlr.rolke_orig_sig_ul << "\n";
    os.setf(std::ios_base::scientific, std::ios_base::floatfield);
    os << "BR limit: " << hlr.ul_br  << " U2 limit: " << hlr.ul_u2 << "\n";
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
        << std::setw(15) << "ul_acc_sig"
        << std::setw(15) << "ul_orig_sig"
        << std::setw(15) << "ul_br"
        << std::setw(15) << "ul_u2"
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
        << std::setw(15) << hlr.background_err;

    os.setf(std::ios_base::scientific,
            std::ios_base::floatfield);

    os
        << std::setw(15) <<  hlr.acc_sig_ul
        << std::setw(15) <<  hlr.orig_sig_ul
        << std::setw(15) <<  hlr.ul_br
        << std::setw(15) <<  hlr.ul_u2
        << std::endl;
}

int main()
{
    //**************************************************
    //Signal shapes
    //**************************************************

    //Decide which masses to look at
    std::vector<int> masses;
    for( int m = 250 ; m != 390 ; m += 5 )
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
    { "halo", "k3pi", "k2pig", "k3pi0", "km3", "km2" };

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

    //Output streams
    std::ofstream ofs( "output/lim.txt" );
    print_headings( ofs );
    std::ostream& os = std::cout;

    for ( const auto& mass : masses )
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
}
