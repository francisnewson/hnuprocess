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
        << std::setw(15) <<  hlr.trig_eff
        << std::setw(15) <<  hlr.trig_err
        << std::setw(15) <<  hlr.background
        << std::setw(15) <<  hlr.background_err;

    os.setf(std::ios_base::scientific, std::ios_base::floatfield);
    os
        << std::setw(15) <<  hlr.acc_sig_ul
        << std::setw(15) <<  hlr.orig_sig_ul
        << std::setw(15) <<  hlr.ul_br
        << std::setw(15) <<  hlr.ul_u2
        << std::endl;
}

int main()
{
    std::string filename = "tdata/km2_acc/all.km2_acc.root" ;

    auto fid_weights = extract_fiducial_weights
        ( filename, "", "sample_burst_count/bursts", "weight"  );

    TFile tf {filename.c_str()};

    TFile tfout{ "output/signal_shapes.root", "RECREATE" };

    std::vector<int> masses;
    for( int m = 250 ; m != 390 ; m += 5 )
    { masses.push_back( m ); }

    std::ostream& os = std::cout;


    std::vector<std::string> regions { "signal_upper_muv", "signal_lower_muv" };

    std::string bg_filename = "tdata/staging/all.mass_plots.root";
    TFile tfbg{ bg_filename.c_str() };

    std::string trig_filename = "tdata/halo_control/all.halo_control.root" ;
    TFile tftrig{ trig_filename.c_str() };

    std::vector<std::string> background_channels
    { "halo", "k3pi", "k2pig", "k3pi0", "km3", "km2" };

    TFile tfscat { "tdata/staging/km2_scat.root"};
    TFile tfnoscat { "tdata/staging/km2_noscat.root"};

    ScatterContrib sc{ tfnoscat, tfscat };

    Limiter lm{ tfbg, tftrig, regions };
    lm.set_bg_channels( background_channels);
    lm.set_trig_regions( { "sig_up_trig_eff", "sig_dn_trig_eff" } );
    lm.set_scatter_contrib( sc );

    std::ofstream ofs( "output/lim.txt" );
    print_headings( ofs );


    for ( const auto& mass : masses )
    {

        HnuLimParams hlp( tf, fid_weights, regions, mass );

        auto limres = lm.get_limit( hlp );
        print_result( os, hlp, limres );
        print_flat_result( ofs, hlp, limres );

        tfout.cd();
        hlp.write();
    }
}
