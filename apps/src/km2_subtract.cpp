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
#include "Km2Sub.hh"

double km2g_br( double xi, int ni, double mkva )
{
    //Bijnens Nucl Phys B 396 (1993) 81
    //Definitions pg 89
    double N = 8.348e-02;
    return N * std::pow( mkva , ni ) * xi;
}

int main( int argc, char * argv[] )
{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/halo_subtract.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );
    {
        std::ofstream sflaunch( "kslaunch.log", std::ofstream::app );
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

    path output_filename = "output/km2_sub.root";

    if ( vm.count( "output" ) )
    {
        output_filename = vm["output"].as<path>() ;
    }

    TFile tfout( output_filename.string().c_str(), "RECREATE" );

    path full_filename = "output/km2_full.root";

    if ( vm.count( "log" ) )
    {
        full_filename = vm["log"].as<path>() ;
    }

    TFile tflog( full_filename.string().c_str(), "RECREATE" );

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


    Km2Sub km2_sub{ tfout, tflog, mission.string() };
    km2_sub.process_all_plots();
}
