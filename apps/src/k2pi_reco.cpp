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

    std::string folder = "";
    if ( vm.count( "channel" ) )
    {
        folder = vm["channel"].as<std::string>() ;
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

    //**************************************************
    //Selections
    //**************************************************

    int next_id = 0;

    //References
    auto& event = *event_data;
    auto& raw_lkr = event_data->raw_lkr;
    auto& fit_lkr = event_data->fit_lkr;
    auto& raw_dch = event_data->raw_dch;

    auto pass_function = [](){ return true;};
    LambdaCut auto_pass{ pass_function };
    auto_pass.set_name( "auto_pass" );
    auto_pass.set_id( next_id++ );

    //M2M CUT
    auto m2m_pip = [&raw_lkr]()
    {
        //Copy data into parameter array
        k2pi_params lkr_param_data;
        K2piLkrInterface lkr_interface( raw_lkr);
        copy( lkr_interface, lkr_param_data );

        //Extract 'fit' result
        k2pi_fit data_fit;
        compute_pion_mass( lkr_param_data, data_fit );

        //Lkr 4mom
        TLorentzVector& lkr_pip_4mom = data_fit.pip;
        double m2m =  lkr_pip_4mom.M2();
        double delta_m2m = m2m - std::pow(na62const::mPi, 2 );

        return ( fabs( delta_m2m ) <  0.005 ) ;
    };

    LambdaCut m2m_cut{ m2m_pip};
    m2m_cut.set_id( next_id++ );

    //--------------------

    //EOP CUT
    auto raw_eop = [&event, &raw_dch, &is_mc]()
    {
        if ( event.found_track_cluster )
        {
            double eop = extract_eop( event, raw_dch, is_mc );
            return ( 0.2 < eop && eop < 0.8 );
        }
        else
        {
            return true;
        }
    };

    LambdaCut eop_cut{ raw_eop};
    eop_cut.set_name( "eop_cut" );
    eop_cut.set_id( next_id++ );

    //--------------------

    //PHOTON SEPARATION
    auto photon_sep = [&raw_lkr]()
    {
        return extract_photon_sep( raw_lkr ) > 20 ;
    };

    LambdaCut photon_sep_cut{ photon_sep};
    photon_sep_cut.set_name( "photon_sep_cut" );
    photon_sep_cut.set_id( next_id++ );

    //--------------------

    //TRACK CLUSTER SEPARATION
    auto track_cluster_sep = [&raw_lkr, &raw_dch]()
    {
        return extract_min_track_cluster_sep( raw_lkr, raw_dch ) > 30 ;
    };

    LambdaCut track_cluster_sep_cut{ track_cluster_sep};
    track_cluster_sep_cut.set_name( "track_cluster_sep_cut" );
    track_cluster_sep_cut.set_id( next_id++ );

    //--------------------

    //MIN PHOTON RADIUS
    auto min_photon_radius = [&raw_lkr]
    {
        return extract_min_photon_radius( raw_lkr ) > 15;

    };

    LambdaCut min_photon_radius_cut{ min_photon_radius };
    min_photon_radius_cut.set_name( "min_photon_radius_cut" );
    min_photon_radius_cut.set_id( next_id++ );

    //CHI2 CUT
    auto max_chi2 = [&event]
    {
        return event.lkr_fit_chi2 < 0.016 ;
    };

    LambdaCut chi2_cut{ max_chi2 };
    chi2_cut.set_name( "chi2_cut" );
    chi2_cut.set_id( next_id++);

    CompositeSelection full_selection( { &m2m_cut, &eop_cut, &track_cluster_sep_cut, &min_photon_radius_cut } );
    full_selection.set_name( "full_selection" );
    full_selection.set_id( next_id++);

    CompositeSelection fit_selection( { &auto_pass, &chi2_cut, &eop_cut, &track_cluster_sep_cut, &min_photon_radius_cut} );
    fit_selection.set_name( "fit_selection" );
    fit_selection.set_id( next_id++);

    std::vector<Selection*> selections {
        &auto_pass, &m2m_cut, &eop_cut, 
            &track_cluster_sep_cut, &full_selection, &min_photon_radius_cut,
            &chi2_cut, &fit_selection
    };

    //**************************************************
    //Analysis
    //**************************************************

    DchAnalysis raw_dch_plotter( auto_pass, tfout, "raw_k2pi_plots", *event_data, "raw", is_mc );
    DchAnalysis raw_fit_dch_plotter( auto_pass, tfout, "raw_fit_k2pi_plots", *event_data, "fit", is_mc );
    DchAnalysis select_dch_plotter( full_selection, tfout, "select_k2pi_plots", *event_data, "raw", is_mc );
    DchAnalysis select_fit_dch_plotter( fit_selection, tfout, "select_fit_k2pi_plots", *event_data, "fit", is_mc );

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //IMPORTANT
    //DchAnalysis implements a cut on CDA!
    //(in addition to anything in the selections above)
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    Summary summary( auto_pass, fit_selection, std::cout );
    summary.set_name( "FIT SELECTION SUMMARY" );

    std::vector<Analysis*> analyses { 
        &raw_dch_plotter, &select_dch_plotter,
            &raw_fit_dch_plotter, &select_fit_dch_plotter ,
            &summary,
    };

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

        for( auto& sel: selections ) { sel->new_event(); }
        for( auto& an: analyses ) { an->new_event(); }
    }

    std::cerr << echain->read_info();

    for( auto& an: analyses )
    {
        an->end_processing();
    }
    tfout.cd();
}
