#include "easy_app.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "HistStacker.hh"
#include "TFile.h"
#include "OwningStack.hh"
#include "Km2Scaling.hh"
#include "yaml-cpp/exceptions.h"
#include "RatioCanvas.hh"
#include "fiducial_functions.hh"
#include "Shuffler.hh"

int main( int argc, char * argv[] )
{
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
        config_node = YAML::LoadFile( mission.string() );
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
    std::map<std::string, MultiScaling> scaling_info;

    for ( const auto& scaling_method : scaling_node )
    {
        const YAML::Node & method_node = scaling_method["scaling_method"];
        std::string method_name = get_yaml<std::string>( method_node, "name" );

        std::cout << "\n##  " << method_name << "  ##\n";

        //Create Km2Scaling object in map
        auto insertion  =
            scaling_info.emplace( std::make_pair(
                        method_name, MultiScaling(method_node, fiducial_weights, branching_ratios) ) );
        auto& this_method = insertion.first->second;

        //COmpute scaling and print
        this_method.compute_scaling();

        std::cout << "Halo scale: " << this_method.get_halo_scale() 
            << " ± " << this_method.get_halo_scale_error() << "\n";

        std::cout << "Peak scale: " << this_method.get_peak_scale() 
            << " ± " << this_method.get_peak_scale_error() << "\n";

        std::cout << "Fiducial flux: " << this_method.get_fiducial_flux() << "\n";

        std::cout << std::string( 50, '-' ) << "\n\n";
    }

    for ( const auto& scaling : scaling_info  )
    {
        //std::cout << scaling.second.get_km2_fid_weight() << std::endl;
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

#if 0
    //Setup input
    path stack_input_file = get_yaml<std::string>( config_node["output"], "input_file" );
    TFile tfstackin( stack_input_file.string().c_str() );
    RootTFileWrapper rtfw_stack( tfstackin );
    ChannelHistExtractor ce_stack( rtfw_stack );


    HistFormatter formatter( "input/shuffle/colors.yaml" );

    auto data_channels = get_yaml<std::vector<std::string>>(
            config_node["output"]["data_plot"], "channels" );

    //get plot list
    const YAML::Node& plots = config_node["output"]["plots"];
    for ( const auto& plot_node : plots )
    {
        std::string name = get_yaml<std::string>( plot_node, "name");
        std::string path = get_yaml<std::string>( plot_node, "path");

        std::cout << "Plotting: " <<  std::setw(40) << name << "   from    " << path <<  std::endl;

        ce_stack.set_rebin( plot_node["rebin"].as<int>( 1 ) );
        ce_stack.reset_collapse();
        ce_stack.set_collapse_x( plot_node["collapse_x"].as<bool>( false ) );
        ce_stack.set_collapse_y( plot_node["collapse_y"].as<bool>( false ) );

        ce_stack.set_post_path( path );

        HistStacker hs( config_node["output"] , ce_stack, scaling_info, formatter );
        hs.create_stack();

        TH1 * start = (*hs.begin());

        //Write stack
        cd_p( &tfout, name );
        hs.write( "hnu_stack" );
        cd_p( &tfout, boost::filesystem::path{ name }/ "hnu_stack_hists" );
        for ( auto& h : hs ) { h->Write(); }

        cd_p( &tfout, name );
        hs.write_total("hbg") ;

        std::unique_ptr<TH1> hdata =  get_summed_histogram( ce_stack,
                begin( data_channels), end( data_channels ) );

        format_data_hist( *hdata, config_node["output"]["data_plot"] );

        cd_p( &tfout, name );
        hdata->Write( "hdata" );

        std::cout << integral( *hs.get_total_copy(), -0.1, 0.01 )
            <<  " " << integral( *hdata, -0.01, 0.01 ) << std::endl;


        auto hdenom = hs.get_total_copy();
        std::unique_ptr<TH1> hratio{ static_cast<TH1*>( hdata->Clone("hratio") ) };
        hratio->SetDirectory(0);
        hratio->Sumw2();
        hratio->Divide( hdenom.get() );
        hratio->Write("hratio");

        std::unique_ptr<TH1> hresid{ static_cast<TH1*>( hdata->Clone("hresid") ) };
        hresid->SetDirectory(0);
        hresid->Sumw2();
        hresid->Add( hdenom.get(), - 1 );
        hresid->Write("hresid");

        RatioCanvas rc( *hdenom, *hdata, *hratio);
        rc.Write( "c_comp");

        std::cout << std::string(  50, ' ' ) 
            << "bg: " << hdenom->Integral() << " dt:" << hdata->Integral() << std::endl;
    }
#endif

    exit(0);


    //**************************************************
    //Save data plots
    //**************************************************


    //**************************************************
    //Compute limits
    //**************************************************

    //ce_stack.set_rebin( 1 );
    // UpperLimitExtractor ule( config_node["output"], config_node["mass_window"], ce_stack, scaling_info );
}
