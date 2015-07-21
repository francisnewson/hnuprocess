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

    //Load branching ratios
    std::string br_filename = "input/shuffle/branching_ratios.yaml" ;
    auto branching_ratios = YAML::LoadFile(br_filename).as<std::map<std::string, double>>();

    //CALIBRATION
    YAML::Node calib_node = config_node["calibration"];
    if ( ! calib_node ){  throw std::runtime_error("Missing calibration node"); }

    //Load fiducial weights
    std::map<std::string, double> calib_fid_weights 
        = extract_root_fiducial_weights( calib_node["fid"] );

    //estimate k- flux from km2
    YAML::Node calib_km2_node = calib_node[ "km2_method" ];
    MultiScaling calib_km2_scaling = 
        MultiScaling( calib_km2_node, calib_fid_weights, branching_ratios );

    calib_km2_scaling.compute_scaling();
    std::cout << "\nCALIBRATION: km2\n";
    print_scaling( calib_km2_scaling, std::cout );

    //estimate k- flux from k3pi
    YAML::Node calib_k3pi_node = calib_node[ "k3pi_method" ];
    MultiScaling calib_k3pi_scaling = 
        MultiScaling( calib_k3pi_node, calib_fid_weights, branching_ratios );

    calib_k3pi_scaling.compute_scaling();
    std::cout << "\nCALIBRATION: k3pi\n";
    print_scaling( calib_k3pi_scaling, std::cout );

    //determine correction factor
    double correction_factor = 
        calib_k3pi_scaling.get_fiducial_flux() / calib_km2_scaling.get_fiducial_flux();

    std::cerr << "\nCorrection factor: " << correction_factor << std::endl;

    //save log plots
    //Set up input file
    path scale_input_file = get_yaml<std::string>( calib_k3pi_node["mc"], "input_file" );
    TFile scale_tf( scale_input_file.string().c_str() );
    RootTFileWrapper scale_rtfw( scale_tf );
    ChannelHistExtractor scale_ce( scale_rtfw );

    //Get channel lists
    std::vector<std::string> scale_halo_channels 
        = calib_k3pi_node["channels"]["halo_channels"].as<std::vector<std::string>>();

    std::vector<std::string> scale_peak_channels 
        =  calib_k3pi_node["channels"]["peak_channels"].as<std::vector<std::string>>();

    std::vector<std::string> scale_data_channels 
        =  calib_k3pi_node["channels"]["data_channels"].as<std::vector<std::string>>();

    scale_ce.set_post_path( get_yaml<std::string>( calib_k3pi_node["mc"], "plot_path" ) );

    std::unique_ptr<TH1> scale_hpeak = get_summed_histogram(
            scale_ce, begin( scale_peak_channels ), end( scale_peak_channels));

    std::unique_ptr<TH1> scale_hdata = get_summed_histogram(
            scale_ce, begin( scale_data_channels ), end( scale_data_channels));

    std::unique_ptr<TH1> scale_hhalo = get_summed_histogram(
            scale_ce, begin( scale_halo_channels ), end( scale_halo_channels));

    YAML::Node dummy = YAML::Load( "do_blind: false" );
    HistFormatter formatter( "input/shuffle/colors.yaml" );

    formatter.format( *scale_hpeak, "k3pi" );
    formatter.format( *scale_hhalo, "halo" );
    format_data_hist( *scale_hdata, dummy  );

    scale_hpeak->Rebin(50);
    scale_hhalo->Rebin(50);
    scale_hdata->Rebin(50);

    cd_p( &tflog, "k3pi_scaling" );
    scale_hdata->Write( "h_data" );

    scale_hhalo->Scale( calib_k3pi_scaling.get_halo_scale() ); 
    scale_hhalo->Write( "h_halo" );

    auto  scale_hpeak_k3pi = uclone( scale_hpeak );
    scale_hpeak_k3pi->Scale( calib_k3pi_scaling.get_peak_scale() );
    scale_hpeak_k3pi->Write( "h_peak_k3pi_scale" );

    THStack h_k3pi_scale;
    h_k3pi_scale.Add( scale_hhalo.get() );
    h_k3pi_scale.Add( scale_hpeak_k3pi.get() );
    h_k3pi_scale.Write( "hs_k3pi_scale" );

    auto  scale_hpeak_km2 = uclone( scale_hpeak );
    scale_hpeak_km2->Scale( calib_km2_scaling.get_peak_scale() );
    scale_hpeak_km2->Write( "h_peak_km2_scale" );

    THStack h_km2_scale;
    h_km2_scale.Add( scale_hhalo.get() );
    h_km2_scale.Add( scale_hpeak_km2.get() );
    h_km2_scale.Write( "hs_km2_scale" );

    std::cerr << "--------------------------------------------------\n";


    YAML::Node subtractions_node = config_node["subtractions"];

    for ( const auto& sub_node : subtractions_node )
    {
        std::cout << "\n" << sub_node["name"].as<std::string>()  << "\n";

        //Load fiducial weights
        std::map<std::string, double> sub_fid_weights 
            = extract_root_fiducial_weights( sub_node["fid"] );

        //estimate k- flux from km2
        YAML::Node scaling = sub_node["scaling"];
        MultiScaling this_scaling = 
            MultiScaling( scaling, sub_fid_weights, branching_ratios );

        this_scaling.compute_scaling();
        std::cout << "CALIBRATION: km2\n";
        print_scaling( this_scaling, std::cout );

        //use correction factor
        double corrected_flux = correction_factor * this_scaling.get_fiducial_flux();
        std::cout << "Corrected flux: " << corrected_flux << std::endl;

        //Set up input file
        path input_file = get_yaml<std::string>( sub_node["plotting"], "input_file" );
        TFile tf( input_file.string().c_str() );
        RootTFileWrapper rtfw( tf );
        ChannelHistExtractor ce( rtfw );

        //Get channel lists
        std::vector<std::string> halo_channels 
            = sub_node["plotting"]["channels"]["halo_channels"].as<std::vector<std::string>>();

        std::vector<std::string> peak_channels 
            = sub_node["plotting"]["channels"]["peak_channels"].as<std::vector<std::string>>();

        //Loop over requested plots
        std::vector<std::string> plot_names = 
            sub_node["plotting"]["plots"].as<std::vector<std::string>>();

        std::string peakname = get_yaml<std::string>( sub_node["plotting"], "peakname");
        std::string haloname = get_yaml<std::string>( sub_node["plotting"], "haloname");
        std::string peakfid = get_yaml<std::string>( sub_node["plotting"], "peakfid");

        for ( auto& plot_name : plot_names )
        {
            //Load raw plots
            ce.set_post_path( plot_name );
            std::unique_ptr<TH1> hpeak = get_summed_histogram(
                    ce, begin( peak_channels ), end( peak_channels));

            std::unique_ptr<TH1> hhalo = get_summed_histogram(
                    ce, begin( halo_channels ), end( halo_channels));

            //use corrected flux
            double br = branching_ratios.at( peakname);
            double fid_weight = at( sub_fid_weights, peakfid, "Missing sub_fid_weight: " + peakfid );
            double scale_factor = corrected_flux  * br / fid_weight;
            hpeak->Scale( scale_factor );

            //So subtraction!
            hhalo->Add( hpeak.get() , -1 );

            //save plots
            path write_path = path{haloname}/plot_name;
            cd_p( &tfout, write_path.parent_path()  );
            hhalo->Write( write_path.filename().string().c_str() );
        }
    }
}
