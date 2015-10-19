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

int main()
{
    //**************************************************
    //Signal shapes
    //**************************************************

    //Decide which masses to look at
    std::vector<int> masses;
    for( int m = 200 ; m != 380 ; m += 1 )
    { masses.push_back( m ); }

    //Load acceptance plots
    std::string acc_filename = "tdata/km2_acc/all.km2_acc.root" ;

    TFile tfin_acc {acc_filename.c_str()};
    auto fid_weights = extract_fiducial_weights
        ( acc_filename, "", "sample_burst_count/bursts", "weight"  );

    std::vector<std::string> signal_regions 
    { "signal_upper_loose_plots", "signal_lower_loose_plots" };

        std::cout 
           << std::setw(15)  << "mass" 
           << std::setw(15)  << "mean"
           << std::setw(15)  << "signal_width"
           << std::setw(15)  << "fit_sigma"
           << std::setw(15)  << "full_acc"
           << std::setw(15)  << "width_acc"
           << std::setw(15)  << "width_acc_err"
           << "\n";

    for ( const auto& mass : masses )
    {
        //Set up signal parameters

        try
        {
        HnuLimParams hlp( tfin_acc, fid_weights, signal_regions, mass );

        auto h = hlp.get_signal_hist();
        TF1 hfit{ "hfit", "gaus"  };
        h.Fit( "hfit", "Q" );

        std::cout 
           << std::setw(15)  << mass 
           << std::setw(15)  << hlp.get_mean()
           << std::setw(15)  << hlp.get_width()
           << std::setw(15)  << hfit.GetParameter( 2 )
           << std::setw(15)  << hlp.get_full_acceptance()
           << std::setw(15)  << hlp.get_width_acceptance()
           << std::setw(15)  << hlp.get_width_acceptance_err()
           << "\n";
        }
        catch( std::runtime_error& e )
        {
            std::cerr << e.what() << std::endl;
            continue;
        }
    }

}
