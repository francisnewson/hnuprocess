#include "TMath.h"
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TCut.h"
#include "TH1F.h"
#include "TSystem.h"
#include "K2piVars.hh"
#include "Track.hh"
#include <iostream>
#include "HistStore.hh"
#include "Counter.hh"
#include "logging.hh"
#include "K2piPlots.hh"
#include "easy_app.hh"
#include "stl_help.hh"
#include <boost/algorithm/string/replace.hpp>
#include "NA62Constants.hh"

int main( int argc, char * argv[] )
{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/hnu_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream flaunch( "k2pi_launch.log", std::ofstream::app );
        write_launch ( argc, argv, flaunch );
    }
    
    //start logging
    fn::logger slg;

    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");
    general.add_options()
        ( "help,h", "Display this help message")
        ( "input-file,i", po::value<path>(), "Specify input file")
        ( "output-file,o", po::value<path>(), "Specify output file")
        ;

    po::options_description desc("Allowed options");
    desc.add( general );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    BOOST_LOG_SEV( slg, always_print) << "Parsing program options ... ";

    //Process program options--------------------------------------------
    //help
    if ( vm.count( "help" ) )
    {
        std::cerr << desc << "\n";
        std::cerr << "Exiting because help was requested." << "\n";
        return false;
    }

    //input file
    if ( ! vm.count ("input-file" ) )
    {
        std::cerr << "**ERROR** Must specify input file!" << std::endl;
        return false;
    }
    path input_file = vm["input-file"].as<path>() ;
    BOOST_LOG_SEV( slg, always_print) << "Reading from " << input_file.string() ;

    //Default computed output name
    std::string input_name = input_file.filename().string();
    boost::replace_last( input_name, "tree", "plots" );
    path output_file = path{"output"}/ input_name;

    //optional user output name
    if ( vm.count ("output-file" ) )
    {
        output_file = vm["output-file"].as<path>() ;
    }

    BOOST_LOG_SEV( slg, always_print) << "Writing to   " << output_file.string() ;

    //Open tree
    TFile * tfile = new TFile( input_file.string().c_str() );
    TTree * k2pi = 0;
    tfile->GetObject("k2pi", k2pi);

    k2pi->SetCacheSize( 10000000 );
    k2pi->AddBranchToCache("*");

    double nEntries = k2pi->GetEntriesFast();

    //Connect event
    const fn::K2piVars * vars = new fn::K2piVars;
    k2pi->SetBranchAddress( "k2piVars", &vars );

    TFile tfout( output_file.string().c_str() , "RECREATE");
    fn::Counter counter( slg, nEntries);

    K2piPlots k2pi_plots( vars, tfout );

    //Prepare mass cut
    double mass2_width = 0.005;
    double m2pip = std::pow( na62const::mPi, 2 );
    double min_mass2 = m2pip - mass2_width;
    double max_mass2 = m2pip + mass2_width;

    int npassed = 0;

    for ( Int_t i = 0 ; i < nEntries ; ++i )
    {
        k2pi->GetEntry( i );
        counter.new_event();

        double reco_mass2 = vars->data.p4pip_lkr.M2();
        if ( reco_mass2  > max_mass2 ) continue;
        if ( reco_mass2  < min_mass2 ) continue;

        k2pi_plots.new_event();
        ++npassed;
    }

    std::cerr << npassed << " out of " << nEntries 
        << " events passed the mass cut" << std::endl;

    tfout.cd();
    k2pi_plots.end_processing();
}
