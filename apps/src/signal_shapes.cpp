#include "easy_app.hh"
#include "root_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "fiducial_functions.hh"
#include "TFile.h"
#include <cmath>
#include <numeric>
#include <boost/io/ios_state.hpp>
#include <boost/regex.hpp>

int main()
{
    std::string filename = "tdata/km2_acc/all.km2_acc.root" ;

    auto fid_weights = extract_fiducial_weights
        ( filename, "", "sample_burst_count/bursts", "weight"  );

    //print_fid_weights( fid_weights, std::cout );

    TFile tf {filename.c_str()};
    tf.Print();


    std::vector<std::string> channels{
        "mc.p5.hnu.200.nodk", "mc.p5.hnu.205.nodk",
            "mc.p5.hnu.210.nodk", "mc.p5.hnu.215.nodk",
            "mc.p5.hnu.220.nodk", "mc.p5.hnu.225.nodk",
            "mc.p5.hnu.230.nodk", "mc.p5.hnu.235.nodk",
            "mc.p5.hnu.240.nodk", "mc.p5.hnu.245.nodk",
            "mc.p5.hnu.250.nodk", "mc.p5.hnu.255.nodk",
            "mc.p5.hnu.260.nodk", "mc.p5.hnu.265.nodk",
            "mc.p5.hnu.270.nodk", "mc.p5.hnu.275.nodk",
            "mc.p5.hnu.280.nodk", "mc.p5.hnu.285.nodk",
            "mc.p5.hnu.290.nodk", "mc.p5.hnu.295.nodk",
            "mc.p5.hnu.300.nodk", "mc.p5.hnu.305.nodk",
            "mc.p5.hnu.310.nodk", "mc.p5.hnu.315.nodk",
            "mc.p5.hnu.320.nodk", "mc.p5.hnu.325.nodk",
            "mc.p5.hnu.330.nodk", "mc.p5.hnu.335.nodk",
            "mc.p5.hnu.340.nodk", "mc.p5.hnu.345.nodk",
            "mc.p5.hnu.350.nodk", "mc.p5.hnu.355.nodk",
            "mc.p5.hnu.360.nodk", "mc.p5.hnu.365.nodk",
            "mc.p5.hnu.370.nodk", "mc.p5.hnu.375.nodk",
            "mc.p5.hnu.380.nodk", "mc.p5.hnu.385.nodk"
    };

    std::vector<std::string> pols { "pos", "neg" };
    std::vector<std::string> regs { "signal_upper_muv", "signal_lower_muv" };

    std::ostream& os = std::cout;

    os 
        << std::setw(20) << "channel"
        << std::setw(12) << "mass"
        << std::setw(12) << "mean"
        << std::setw(12) << "sqrt(mean)"
        << std::setw(12) << "acc"
        << std::endl;

    for ( const auto& chan : channels )
    {
        std::vector<std::unique_ptr<TH1D>> hists;
        std::vector<double> this_fid_weight;

        try
        {

            for ( const auto& pol : pols  )
            {
                for( const auto& reg : regs )
                {
                    path p = path{ (chan + "." + pol ) } / reg / "h_m2m_kmu";
                    auto h = extract_hist<TH1D>( tf,  p );
                    hists.emplace_back( std::move( h ) );
                }

                this_fid_weight.push_back( fid_weights[ (chan + "." + pol) ] );
            }

        }
        catch( std::exception& e )
        {
            //std::cerr << e.what() << std::endl;
            continue;
        }

        auto hsum = sum_hists( begin( hists ), end( hists ) );
        double mean = hsum->GetMean();
        double int_total = integral( *hsum, 0, 0.2 );
        double fid_total = std::accumulate( begin( this_fid_weight ), end( this_fid_weight ) , 0.0 );
        double acc = int_total / fid_total;

        boost::regex re{".*\\.([0-9]+).*"};
        boost::cmatch matches;
        boost::regex_match( chan.c_str(), matches, re );

        std::string mass = matches[1];


        boost::io::ios_flags_saver fs( os );
        os.setf(std::ios::fixed, std::ios::floatfield);
        os.precision(4);
    os 
        << std::setw(20) << chan
        << std::setw(12) << mass
        << std::setw(12) << mean
        << std::setw(12) << std::sqrt(mean)
        << std::setw(12) << acc
        << std::endl;
    }
}
