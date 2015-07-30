#include "easy_app.hh"
#include "TH2D.h"
#include "yaml_help.hh"
#include "TFile.h"
#include "TTree.h"
#include "TEfficiency.h"
#include "root_help.hh"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <set>
#include "MuvBursts.hh"
#include "BadBurst.hh"
#include "TCanvas.h"
#include "TGraph.h"
#include <iomanip>
#include <limits>
#include "TNtuple.h"
#include "muv_burst_functions.hh"

using boost::filesystem::path;

void do_run_range( TFile& tfin, po::variables_map& vm )
{
    auto run = vm["run_range"].as<int>();
    CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };

    int min_burst = 0;
    int max_burst = std::numeric_limits<int>::max();

    if ( vm.count("start_burst" ) )
    {
        min_burst = vm["start_burst"].as<int>();
   }

    if ( vm.count("end_burst" ) )
    {
        max_burst = vm["end_burst"].as<int>();
    }

    path output_file = "output/burst_list.dat";
    if ( vm.count( "output" ) )
    {
        output_file = vm["output"].as<path>();
    }

    boost::filesystem::ofstream os{output_file};

    print_bursts( ct.get_tree(), run, min_burst, max_burst, os );

    std::cerr << "Output written to " << output_file.string() << std::endl;
}


//I don't know what this achieves (FN 2015-07-29)
void do_merge( TFile& tfin, po::variables_map& vm )
{
    path output_file = "output/muv_bursts.root";
    if ( vm.count( "output" ) )
    {
        output_file = vm["output"].as<path>();
    }

    CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };
    TTree& tt = ct.get_tree();
    TFile tfout( output_file.string().c_str(), "RECREATE") ;
    ct.get_tree().CloneTree()->Write("muv_burst" );

    Long64_t run;
    Long64_t burst_time;
    Double_t pass;
    Double_t total;

    TTree my_ttree( "split_muv_bursts", "Muv bursts");
    my_ttree.Branch( "run", &run );
    my_ttree.Branch( "burst_time", &burst_time );
    my_ttree.Branch( "pass", &pass );
    my_ttree.Branch( "total", &total );

    muv_burst mb;
    tt.SetBranchAddress( "muv_burst", &mb );

    Long64_t tt_entries = tt.GetEntries();
    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    {
        tt.GetEntry( ient );
        run = mb.run;
        burst_time = mb.burst_time;
        total = mb.total;
        pass = mb.pass;

        my_ttree.Fill();
    }

    my_ttree.Write();

    tfout.Close();
}


void do_bursts( TFile& tfin, po::variables_map& vm )
{
    double threshold = vm["bursts"].as<double>();

    path output_file = "output/bad_muv_bursts.dat";
    if ( vm.count( "output" ) )
    {
        output_file = vm["output"].as<path>();
    }
    CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };

    std::vector<fn::BadBurst::BurstId> bad_bursts;

    if ( vm.count( "level" ) )
    {
        double level  = vm["level"].as<double>();
        bad_bursts = find_stat_bad_bursts( ct.get_tree(), threshold, level );
    }
    else
    {
        bad_bursts = find_bad_bursts( ct.get_tree(), threshold );
    }

    std::sort( begin( bad_bursts ), end( bad_bursts ) );

    boost::filesystem::ofstream os{output_file};

    for ( auto bb : bad_bursts )
    {
        os << std::setw(10) << bb.run << std::setw(20) << bb.burst_time << "\n";
    }

    std::cerr << "Output written to " << output_file.string() << std::endl;

}

int main( int argc, char * argv[] )
{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/hnu_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream flaunch( "mvlaunch.log", std::ofstream::app );
        write_launch ( argc, argv, flaunch );
    }

    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description basic("basic");
    basic.add_options()
        ( "help,h", "Display this help message")
        ( "input,i", po::value<path>(),  "Specify input root file")
        ( "output,o", po::value<path>(),  "Specify output (default varies)")
        ;

    po::options_description rrange("run_range");
    rrange.add_options()
        ( "run_range,r", po::value<int>(),  "Run range")
        ( "start_burst,s", po::value<int>(),  "Start burst")
        ( "end_burst,e", po::value<int>(),  "End burst")
        ;

    po::options_description bbursts("bad_bursts");
    bbursts.add_options()
        ( "bursts,b", po::value<double>(),  "Specify bad burst threshold")
        ( "level,l", po::value<double>(),  
          "Specify significance level (ignore for flat cut on eff)" )
        ;

    po::options_description extra("other");
    extra.add_options()
        ( "plots,p", "Make burst plots")
        ( "text,t",  "Make bad burst text file")
        ( "merge,m", "merge trees (no idea)")
        ;

    po::options_description desc("Allowed options");
    desc.add( basic ).add( rrange ).add(bbursts).add( extra );

    //**************************************************
    //Parse options
    //**************************************************

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    if ( vm.count( "help" ) )
    {
        std::cerr << desc << std::endl;
        std::cerr << "Exiting because help was requested" << std::endl;
        exit( 1 );
    }

    if ( vm.count("plots") + vm.count("bursts") 
            + vm.count("run_range") + vm.count( "text" ) > 1 )
    {
        std::cerr << "Can only do plots OR bursts OR run_range OR text!\n";
        exit(1);
    }

    //**************************************************
    //Setup input
    //**************************************************

    if ( !vm.count( "input" ) )
    {
        std::cerr << "Must specify input file!" << std::endl;
        exit(1);
    }

    TFile tfin( vm["input"].as<path>().string().c_str() );

    //--------------------------------------------------

    //**************************************************
    //Run requested task
    //**************************************************

    if ( vm.count("run_range" ) )
    {
        do_run_range(tfin, vm);
    }

    if ( vm.count("plots") )
    {
        path output_folder = "output/muv_bursts/";
        if ( vm.count( "output" ) )
        {
            output_folder = vm["output"].as<path>();
        }

        CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };
        muv_burst_plots( &ct.get_tree(), output_folder );

    }

    if ( vm.count( "text" ) )
    {
        path output_file = "output/muv_bursts/all.txt";
        if ( vm.count( "output" ) )
        {
            output_file = vm["output"].as<path>();
        }

        CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };
        muv_burst_text( &ct.get_tree(), output_file );
    }

    if ( vm.count( "merge") )
    {
        do_merge( tfin, vm );
    }

    if ( vm.count( "bursts") )
    {
        do_bursts( tfin, vm );
    }
}
