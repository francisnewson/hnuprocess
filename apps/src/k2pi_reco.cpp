#include "TMath.h"
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TCut.h"
#include "TH1F.h"
#include "TSystem.h"
#include "K2piEventData.hh"
#include "Track.hh"
#include <iostream>
#include <iterator>
#include "HistStore.hh"
#include "Counter.hh"
#include "logging.hh"
#include "easy_app.hh"
#include "stl_help.hh"
#include <boost/algorithm/string/replace.hpp>
#include "NA62Constants.hh"
#include <boost/filesystem/fstream.hpp>
#include "FunctionCut.hh"
#include "DchPlotter.hh"
#include "K2piInterface.hh"
#include "k2pi_fitting.hh"
#include "k2pi_reco_functions.hh"
#include "EChain.hh"
#include "GlobalStatus.hh"
#include "Summary.hh"
#include "k2pi_selections.hh"
#include "PionPlotter.hh"

//A global bool which can be
//used to store stop signals
bool&  remote_stop()
{
    static bool rs{false};
    return rs;
}

//Handle signals by setting remote_stop
void sig_handler( int sig )
{ 
    if ( sig )
        remote_stop() = true;
}


void connect_signals()
{
    //Press ctrl-C to save and close
    signal (SIGXCPU, sig_handler);
    signal (SIGINT, sig_handler);
    signal (SIGTERM, sig_handler);
    signal (127, sig_handler);
}

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
    auto  core = boost::log::core::get();
    severity_level min_sev = startup;
    // Set a global filter so that only error messages are logged
    core->set_filter(boost::log::expressions::is_in_range(
                boost::log::expressions::attr< severity_level >("Severity"), 
                min_sev,  severity_level::maximum) );


    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");
    general.add_options()
        ( "help,h", "Display this help message")
        ( "input-file,i", po::value<path>(), "Specify input file")
        ( "list-file,l", po::value<path>(), "Specify input list")
        ( "output-file,o", po::value<path>(), "Specify output file")
        ( "channel,c", po::value<std::string>(), "Specify channel name")
        ( "number,n", po::value<Long64_t>(), "Specify number of events")
        ( "study,s", po::value<std::vector<std::string>>(), "Studies" )
        ( "view-studies,v", "Studies" )
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

    if ( vm.count( "view-studies" ) )
    {
        std::cerr <<  "Available studies: \n"
            << "    dch\n" 
            << "    pion\n" ;

        return 0;
    }

    //input file
    if ( ! vm.count ("input-file" ) && ! vm.count( "list-file")  )
    {
        std::cerr << "**ERROR** Must specify input file!" << std::endl;
        return false;
    }

    std::vector<std::string> input_files;

    std::string input_description;

    if ( vm.count( "input-file" ) )
    {
        path input_file = vm["input-file"].as<path>() ;
        input_files.push_back( input_file.string() );
        input_description = input_file.string();
    }

    if ( vm.count( "list-file" ) )
    {
        path file_list = vm["list-file"].as<path>() ;
        boost::filesystem::ifstream ifs( file_list );
        input_files.insert( begin( input_files),
                std::istream_iterator<std::string>( ifs ),
                std::istream_iterator<std::string>() );
        input_description = file_list.string();
    }

    if ( !vm.count( "study" ) )
    {
        std::cerr << "No studies selected. Exiting" << std::endl;
    }
    std::vector<std::string> studies =  vm["study"].as<std::vector<std::string>>();

    BOOST_LOG_SEV( slg, always_print) << "Reading from " << input_description;

    std::vector<path> input_paths( begin( input_files), end( input_files ) );
    EChain<fn::K2piEventData> * echain = 0;

    if ( vm.count( "number" ) )
    {
        echain = new EChain<fn::K2piEventData>{ slg, input_paths, "T", "K2piEventData", vm["number"].as<Long64_t>() };
    }
    else
    {
        echain = new EChain<fn::K2piEventData>{ slg, input_paths, "T", "K2piEventData" };
    }

    //Default computed output name
    std::string output_description = path{input_description}.filename().string();
    boost::replace_last( output_description, "tree", "plots" );
    path output_file = path{"output"}/ ( "k2piplots_" + output_description) ;
    output_file.replace_extension( "root" );

    //optional user output name
    if ( vm.count ("output-file" ) )
    {
        output_file = vm["output-file"].as<path>() ;
    }

    BOOST_LOG_SEV( slg, always_print) 
        << "Writing to   " << output_file.string() ;

    //Connect event
    fn::K2piEventData * event_data = echain->get_event_pointer();

    TFile tfout( output_file.string().c_str() , "RECREATE");

    std::string channel = "";
    if ( vm.count( "channel" ) )
    {
        channel = vm["channel"].as<std::string>() ;
    }

    Long64_t event_count = echain->get_max_event();
    fn::Counter counter( slg, event_count);

    //C-C now activates remote stop
    connect_signals();
    int events_read = 0;

    boost::optional<bool> opt_mc = echain->get_is_mc();
    assert( opt_mc );
    bool is_mc = *opt_mc;

    K2PIGStatus k2pigs{ echain->get_event_pointer(), is_mc };
    raw_global_status() = &k2pigs;

    K2piRecoBag k2pirb( *event_data, is_mc, tfout, channel );

    //add studies
    for( auto study: studies )
    {
        if ( study == "dch" )
        {
            std::cerr << "Adding dch study" << std::endl;
            add_dch_study( k2pirb );
        }
        else if ( study == "pion" )
        {
            std::cerr << "Adding dch study" << std::endl;
            add_pion_study( k2pirb );
        }
        else
        {
            std::cerr << "Skipping unrecognized study: " << study << std::endl;
        }
    }

    //References
    auto& event = *event_data;

    for ( Int_t i = 0 ; i < event_count ; ++i )
    {
        if ( remote_stop() )
        {
            BOOST_LOG_SEV( slg, always_print )
                << "Stopping after " << i << " events";
            break;
        }

        ++events_read; 
        echain->load_next_event_header();
        echain->load_full_event();

        counter.new_event();
        k2pirb.new_event();

    }

    std::cerr << echain->read_info();

    BOOST_LOG_SEV( slg, always_print) 
        << "Wrote to   " << output_file.string() ;

    k2pirb.end_processing();

    tfout.cd();
}
