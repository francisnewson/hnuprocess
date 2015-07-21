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
        ( "full,f", po::value<path>(),  "Specify full root file")
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

    if ( vm.count( "full" ) )
    {
        full_filename = vm["full"].as<path>() ;
    }

    TFile tffull( full_filename.string().c_str(), "RECREATE" );

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

    //Load fiducial weights
    std::map<std::string, double> fid_weights 
        = extract_root_fiducial_weights( config_node["fid"] );

    std::vector<std::string> plots = get_yaml<std::vector<std::string>>
        ( config_node, "plots" );

    //Set up input file
    path input_file = get_yaml<std::string>( config_node["files"], "input" );
    TFile tf( input_file.string().c_str() );
    RootTFileWrapper rtfw( tf );
    ChannelHistExtractor ce( rtfw );

    std::map<std::string, std::string> names
        = config_node["names"].as<std::map<std::string, std::string>>();

    std::string ib = names["ib"];
    std::string intp = names["intp"];
    std::string intm = names["intm"];
    std::string sdp = names["sdp"];
    std::string sdm = names["sdm"];
    std::string full_name = names["full"];

    double br_km2 = 0.6355;

    //for each plot
    for( const auto& plot_name : plots )
    {
        ce.set_post_path( plot_name );

        for ( const auto& pol : std::vector<std::string>{ ".pos", ".neg" } )
        {

            //km2 IB
            std::cerr << "IB" << std::endl;
            std::vector<std::string> ib_channels = { ib + pol };

            std::unique_ptr<TH1> h_total = get_summed_histogram(
                    ce, begin( ib_channels ), end( ib_channels));

            auto h_raw = uclone( h_total );

            //km2 intp
            std::cerr << "INTP" << std::endl;
            std::vector<std::string> intp_channels = { intp + pol };

            std::unique_ptr<TH1> h_intp = get_summed_histogram(
                    ce, begin( intp_channels ), end( intp_channels));

            double fid_rat_intp = fid_weights[ ib + pol ] /  fid_weights[intp + pol ];
            double br_intp = km2g_br( -1.78e-3,  1 , -0.15 );
            std::cerr << "BR(intp): " << br_intp << std::endl;
            std::cerr << "FIDRAT(inpt): " << fid_rat_intp  
                << " " << fid_weights[ ib + pol ] << " " <<  fid_weights[ intp + pol ] << std::endl;

            double scale_intp = fid_rat_intp * br_intp / br_km2;
            h_total->Add( h_intp.get(), scale_intp );
            h_intp->Scale(scale_intp);

            //----------

            //km2 intm
            std::cerr << "INTM" << std::endl;
            std::vector<std::string> intm_channels = { intm + pol };

            std::unique_ptr<TH1> h_intm = get_summed_histogram(
                    ce, begin( intm_channels ), end( intm_channels));

            double fid_rat_intm = fid_weights[ ib + pol ] /  fid_weights[intm + pol ];
            double br_intm = km2g_br( 1.23e-2,  1 , -0.21 );
            std::cerr << "BR(intm): " << br_intm << std::endl;

            double scale_intm = fid_rat_intm * br_intm / br_km2;
            h_total->Add( h_intm.get(), scale_intm );
            h_intm->Scale(scale_intm);

            //----------

            //km2 sdm
            std::cerr << "SDM" << std::endl;
            std::vector<std::string> sdm_channels = { sdm + pol };

            std::unique_ptr<TH1> h_sdm = get_summed_histogram(
                    ce, begin( sdm_channels ), end( sdm_channels));

            double fid_rat_sdm = fid_weights[ ib + pol ] /  fid_weights[sdm + pol ];
            double br_sdm = km2g_br( 1.18e-2,  2 , -0.21 );
            std::cerr << "BR(sdm): " << br_sdm << std::endl;

            double scale_sdm = fid_rat_sdm * br_sdm / br_km2;
            h_total->Add( h_sdm.get(), scale_sdm );
            h_sdm->Scale(scale_sdm);

            //----------

            //km2 sdp
            std::cerr << "SDP" << std::endl;
            std::vector<std::string> sdp_channels = { sdp + pol };

            std::unique_ptr<TH1> h_sdp = get_summed_histogram(
                    ce, begin( sdp_channels ), end( sdp_channels));

            double fid_rat_sdp = fid_weights[ ib + pol ] /  fid_weights[sdp + pol ];
            double br_sdp = km2g_br( 1.18e-2,  2 , -0.15 );
            std::cerr << "BR(sdp): " << br_sdp << std::endl;

            double scale_sdp = fid_rat_sdp * br_sdp / br_km2;
            h_total->Add( h_sdp.get(), scale_sdp );
            h_sdp->Scale(scale_sdp);


            //save plots
            path write_path = path{full_name + pol }/plot_name;

            cd_p( &tffull, write_path.parent_path()  );
            h_total->Write( write_path.filename().string().c_str() );

            cd_p( &tfout, write_path.parent_path()  );
            h_total->Write( write_path.filename().string().c_str() );
            h_raw->Write( (write_path.filename().string() + "_raw").c_str() );
            h_intp->Write((write_path.filename().string() + "_intp").c_str() );
            h_intm->Write((write_path.filename().string() + "_intm").c_str() );
            h_sdp->Write((write_path.filename().string() + "_sdp").c_str() );
            h_sdm->Write((write_path.filename().string() + "_sdm").c_str() );
        }
    }
}
