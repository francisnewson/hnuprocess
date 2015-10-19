#include "easy_app.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "HistStacker.hh"
#include "TFile.h"
#include "TVectorD.h"
#include "OwningStack.hh"
#include "Km2Scaling.hh"
#include "FlexiScaling.hh"
#include "sample_scaler.hh"
#include "yaml-cpp/exceptions.h"
#include "RatioCanvas.hh"
#include "fiducial_functions.hh"
#include "Shuffler.hh"
#include "RecoParser.hh"

int main( int argc, char * argv[] )
{
    //try
    //{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/shuffle_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream sflaunch( "slaunch.log", std::ofstream::app );
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

    path output_filename = "output/shuffle.root";

    if ( vm.count( "output" ) )
    {
        output_filename = vm["output"].as<path>() ;
    }

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
        auto mangled_config = boost::filesystem::path{ "tmp" } / mission.filename();
        pre_parse( mission, mangled_config );
        config_node = YAML::LoadFile(mangled_config.string() );
    }
    catch( YAML::Exception& e )
    {
        std::cerr << e.what() << std::endl;
    }

    std::cerr << "Parsed " << mission.string() << std::endl;

    //**************************************************
    //Compute scaling
    //**************************************************


    //Load fiducial weights
    std::map<std::string, double> fiducial_weights;

    const auto& weights_node = config_node["weights"];
    if( ! weights_node )
    {
        throw std::runtime_error( "Can't find weights node!" );
    }


    std::string fid_filename = get_yaml<std::string>( config_node["weights"], "fid_file" );
    bool from_root = fn::contains( fid_filename, "root" );

    if ( !from_root )
    {
        fiducial_weights =  YAML::LoadFile(fid_filename).as<std::map<std::string, double>>();
    }
    else
    {
        if ( fid_filename == "use_multi_root"  )
        {fiducial_weights = extract_all_root_fiducial_weights( config_node["weights"] );}
        else
        {
            fiducial_weights = extract_root_fiducial_weights( config_node["weights"]  );
        }
    }

    //print fiducial weights
    print_fid_weights( fiducial_weights, std::cout );

    //check fiducial weights
    if ( config_node["check_weights"] )
    {
        auto check_fid_weights = extract_all_root_fiducial_weights( config_node["check_weights"] );
    }
    else
    {
        std::cout << "Not checking any fiducial weights" << std::endl;

    }

    //Load branching ratios
    std::string br_filename = "input/shuffle/branching_ratios.yaml" ;
    auto branching_ratios = YAML::LoadFile(br_filename).as<std::map<std::string, double>>();

    //Load scaling info
    const YAML::Node& scaling_node = config_node["scaling"];
    //std::map<std::string, std::unique_ptr<SampleScaler>> scaling_info;
    scaling_map scaling_info;

    for ( const auto& scaling_method : scaling_node )
    {
        const YAML::Node & method_node = scaling_method["scaling_method"];
        std::string method_name = get_yaml<std::string>( method_node, "name" );
        std::string method_strategy  = get_yaml<std::string>( method_node, "strategy" );

        std::unique_ptr<SampleScaler> scaler  = 
            get_sample_scaler( method_node, fiducial_weights, branching_ratios );

        auto insertion  =
            scaling_info.insert( std::make_pair( method_name, std::move( scaler )) );
        auto& this_method = insertion.first->second;

        //Compute scaling and print
        this_method->compute_scaling();

        std::cout << "\n##  " << std::setw(15) <<  method_name << std::setw(10)  << method_strategy 
            << "   ##" << std::string(16, '-' ) << "\n";

        std::cout << "Halo scale: " << this_method->get_halo_scale() 
            << " ± " << this_method->get_halo_scale_error() << "\n";

        //std::cout << "Peak scale: " << this_method->get_peak_scale() 
            //<< " ± " << this_method->get_peak_scale_error() << "\n";

        std::cout << "Fiducial flux: " << this_method->get_fiducial_flux() << "\n";

        std::cout << std::string( 50, '-' ) << "\n";
    }


    //**************************************************
    //Build Stack
    //**************************************************

    //Setup output
    TFile tfout( output_filename.string().c_str() , "RECREATE" );

    if ( config_node["output"] )
    {
        do_the_shuffle( config_node["output"], tfout, scaling_info );
    }

    if ( config_node["output_list"] )
    {
        for ( const auto& output_node : config_node["output_list" ] )
        {
            do_the_shuffle( output_node, tfout, scaling_info );
        }
    }

    //Save scaling info
    for ( const auto& scaling : scaling_info  )
    {
        cd_p( &tfout, "scalefactors/" + scaling.first );

        TVectorD halo_scale(1);
        halo_scale[0] = scaling.second->get_halo_scale();
        halo_scale.Write( "halo_scale" );

        TVectorD halo_scale_err(1);
        halo_scale_err[0] = scaling.second->get_halo_scale_error();
        halo_scale_err.Write( "halo_scale_err" );

        TVectorD fiducial_flux(1);
        fiducial_flux[0] = scaling.second->get_fiducial_flux();
        fiducial_flux.Write( "fiducial_flux" );

        TVectorD fiducial_flux_err(1);
        fiducial_flux_err[0] = scaling.second->get_fiducial_flux_error();
        fiducial_flux_err.Write( "fiducial_flux_err" );
    }
    //}
    // catch ( fn::Xcept<fn::MissingInfo>& e )
    // {
    //     std::cerr << "fn::Xcept<fn::MissingInfo> " << e.what() << std::endl;
    //     throw;
    // }



    //**************************************************
    //Print scaling info
    //**************************************************

    std::cout << "SCALE FACTORS: " << std::endl;
    std::cout
        << std::setw(20) << "Name"
        << std::setw(20) << "Halo scale"
        << std::setw(20) << "Km2 Flux"
        << "\n";

    for ( const auto & scaling_method : scaling_info )
    {
        std::cout 
            << std::setw(20) << scaling_method.first
            << std::setw(20) << scaling_method.second->get_halo_scale()
            << std::setw(20) << scaling_method.second->get_fiducial_flux()
            << "\n";
    }


    exit(0);

    //**************************************************
    //Compute limits
    //**************************************************

    //ce_stack.set_rebin( 1 );
    // UpperLimitExtractor ule( config_node["output"], config_node["mass_window"], ce_stack, scaling_info );
}
