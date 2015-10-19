#include "easy_app.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "HistExtractor.hh"
#include "HistStacker.hh"
#include "TFile.h"
#include "OwningStack.hh"
#include "Km2Scaling.hh"
#include "yaml-cpp/exceptions.h"
#include "fiducial_functions.hh"
#include "MultiScaling.hh"
#include <boost/io/ios_state.hpp>
#include "HaloSub.hh"

void print_scaling( MultiScaling& ms, std::ostream& os )
{
    boost::io::ios_flags_saver fs( os );
    os.setf(std::ios::fixed, std::ios::floatfield);
    os.precision(4);

    os << std::setw(20) << "Halo scale: "
        << std::setw(10)  << ms.get_halo_scale() << " ± " << ms.get_halo_scale_error() << "\n";

    os << std::setw(20) << "Peak scale: " 
        << std::setw(10) << ms.get_peak_scale() << " ± " << ms.get_peak_scale_error() << "\n";

    os.setf( std::ios::scientific );
    os << std::setw(20) << "Fiducial flux: " << std::setw(10)  << ms.get_fiducial_flux() << "\n";
}

int main( int argc, char * argv[] )
{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/halo_subtract.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );
    {
        std::ofstream sflaunch( "hslaunch.log", std::ofstream::app );
        write_launch ( argc, argv, sflaunch );
    }

    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "mission,m", po::value<path>(),  "Specify mission yaml file")
        ( "output,o", po::value<path>(),  "Specify output root file")
        ( "log,l", po::value<path>(),  "Specify log root file")
        ;

    po::options_description desc("Allowed options");
    desc.add( general );


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

    path output_filename = "output/halo_sub.root";

    if ( vm.count( "output" ) )
    {
        output_filename = vm["output"].as<path>() ;
    }

    TFile tfout( output_filename.string().c_str(), "RECREATE" );

    path log_filename = "output/halo_sub_log.root";

    if ( vm.count( "log" ) )
    {
        log_filename = vm["log"].as<path>() ;
    }

    TFile tflog( log_filename.string().c_str(), "RECREATE" );

    path mission;
    std::string mission_name;
    if ( ! vm.count( "mission" ) )
    {
        std::cerr << "**ERROR** Must specify mission file!" << std::endl;
        return false;
    }
    else
    {
        mission = vm["mission"].as<path>();
        mission_name = mission.stem().string();
    }

    YAML::Node config_node; 

    std::cerr << "About to parse : " << mission.string() << std::endl;
    try
    {
        config_node = YAML::LoadFile( mission.string() );
    }
    catch( YAML::Exception& e )
    {
        std::cerr << e.what() << std::endl;
    }

    std::cerr << "Parsed " << mission.string() << std::endl;

    //--------------------------------------------------

    //Do fiducial checks
    YAML::Node fidcheck_nodes = config_node["fidcheck" ];
    for ( const auto& fid_node : fidcheck_nodes )
    {
        std::cout << "Checking " << fid_node["name"].as<std::string>()  << "..."  << std::endl;
        auto test_node =  fid_node["trees"].as<std::vector<std::map<std::string, std::string>>>() ;
        bool check = check_fiducial_weights( test_node);
        if ( check )
        {
            std::cout <<  "passed" << std::endl;
        }
        else
        {
            std::cout <<  "failed" << std::endl;
            return EXIT_FAILURE; 
        }
    }

    //Do halo subtraction
    HaloSub hs{ tfout, tflog, mission.string() };
    hs.find_correction();
    //hs.save_log_plots();
    hs.do_subtractions();

    //Do fiducial copying
    if ( config_node["copyfid" ] )
    {
        for ( const auto& copy_node : config_node["copyfid" ] )
        {
            copy_fids( tfout, copy_node );
        }

    }
}
