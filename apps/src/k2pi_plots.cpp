#include "TMath.h"
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TCut.h"
#include "TH1F.h"
#include "TSystem.h"
#include "K2piVars.hh"
#include "Track.hh"
#include <iostream>
#include <iterator>
#include "HistStore.hh"
#include "Counter.hh"
#include "logging.hh"
#include "K2piPlots.hh"
#include "easy_app.hh"
#include "stl_help.hh"
#include <boost/algorithm/string/replace.hpp>
#include "NA62Constants.hh"
#include <boost/filesystem/fstream.hpp>
#include "EChain.hh"

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

    BOOST_LOG_SEV( slg, always_print) << "Reading from " << input_description;

    std::vector<path> input_paths( begin( input_files), end( input_files ) );
    EChain<fn::K2piVars> * echain = 0;

    if ( vm.count( "number" ) )
    {
    echain = new EChain<fn::K2piVars>{ slg, input_paths, "k2pi", "k2piVars", vm["number"].as<Long64_t>() };
    }
    else
    {
    echain = new EChain<fn::K2piVars>{ slg, input_paths, "k2pi", "k2piVars" };
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
    const fn::K2piVars * vars = echain->get_event_pointer();

    TFile tfout( output_file.string().c_str() , "RECREATE");

    std::string folder = "";
    if ( vm.count( "channel" ) )
    {
        folder = vm["channel"].as<std::string>() ;
    }
    K2piPlots k2pi_plots( vars, tfout, folder );

    //Prepare mass cut
    double mass2_width = 0.005;
    double m2pip = std::pow( na62const::mPi, 2 );
    double min_mass2 = m2pip - mass2_width;
    double max_mass2 = m2pip + mass2_width;

    //Prepare z cut
    double min_z = -1800; //cm
    double max_z = +7000; //cm

    //Prepare DCH1 cut
    double min_rdch =  20; //cm
    double max_rdch = 100; //cm

    int npassed = 0;
    Long64_t event_count = echain->get_max_event();
    fn::Counter counter( slg, event_count);
    
    //C-C now activates remote stop
    connect_signals();
    int events_read = 0;

    for ( Int_t i = 0 ; i < event_count ; ++i )
    {
        if ( remote_stop() )
        {
            BOOST_LOG_SEV( slg, always_print )
                << "Stopping after " << i << " events";
            break;
        }

        ++events_read; 
        //k2pi->GetEntry( i );
        echain->load_next_event_header();
        echain->load_full_event();

        counter.new_event();

        const TVector3& neutral_vertex = vars->data.neutral_vertex;
        const TLorentzVector& p4pip_lkr = vars->data.p4pip_lkr;
        Track pion_track_lkr{ neutral_vertex, p4pip_lkr.Vect() };
        TVector3 pion_at_dch_lkr = pion_track_lkr.extrapolate( na62const::zDch1 );
        double track_radius_dch = pion_at_dch_lkr.Perp();
        if ( track_radius_dch < min_rdch ) continue;
        if ( track_radius_dch > max_rdch ) continue;

        double reco_mass2 = vars->data.p4pip_lkr.M2();
        if ( reco_mass2  > max_mass2 ) continue;
        if ( reco_mass2  < min_mass2 ) continue;

        double z = vars->data.neutral_vertex.Z();
        if ( z > max_z ) continue;
        if ( z < min_z ) continue;

        k2pi_plots.new_event();
        ++npassed;

    }

    std::cerr << npassed << " out of " << events_read 
        << " events passed the mass cut" << std::endl;

    std::cerr << echain->read_info();

    tfout.cd();
    k2pi_plots.end_processing();
}
