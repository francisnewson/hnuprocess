#include "easy_app.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "yaml_help.hh"
#include "HistStore.hh"
#include "TGraphAsymmErrors.h"
#include "THStack.h"
#include "TLegend.h"
#include "TriggerApp.hh"
#include "OSLoader.hh"
#include "merging.hh"
#include <vector>

int main( int argc, char * argv[] )
{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/merge_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );
    {
        std::ofstream sflaunch( "mlaunch.log", std::ofstream::app );
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

    path output_filename = "output/merge.root";

    if ( vm.count( "output" ) )
    {
        output_filename = vm["output"].as<path>() ;
    }

    path mission;
    if ( ! vm.count( "mission" ) )
    {
        std::cerr << "**ERROR** Must specify mission file!" << std::endl;
        return false;
    }
    else
    {
        mission = vm["mission"].as<path>();
    }

    TFile tfout { output_filename.string().c_str(), "RECREATE" };

    //--------------------------------------------------

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

    TFileLoader tfl;

    //**************************************************
    //Load trigger efficiencies
    //**************************************************

    std::map<std::string, std::unique_ptr<TriggerApp>> trig_effs;

    const auto trig_nodes = config_node["trig_effs"];
    if ( ! trig_nodes )
    {
        std::cerr << "No trig effs" << std::endl;
    }
    else
    {
        for( const auto& trig_node :  trig_nodes )
        {
            std::string name = get_yaml<std::string>( trig_node , "name" );
            std::string filename = get_yaml<std::string>( trig_node, "filename" );
            auto sels = get_yaml<std::vector<std::string>>( trig_node, "selections" );

            auto& tfile = tfl.get_tfile_opt( filename, "" );
            auto trig_eff = make_unique<TriggerApp>( tfile );
            if ( trig_node["numname"] )
            {
                trig_eff->set_num_denom( 
                        get_yaml<std::string>( trig_node, "numname" ),
                        get_yaml<std::string>( trig_node, "denomname" ) );

            }
            trig_eff->set_sels( sels );
            trig_eff->init();

            trig_effs.insert( std::make_pair( name, std::move( trig_eff ) ) );
        }
    }


    //**************************************************
    //Error
    //**************************************************

    //for now just hardcode the final case

    //halo errors
    TFile tf_halo_log{  "tdata/staging/log/halo_sub_log.q11t.root"  };

    std::string bg_filename = "tdata/staging/all.mass_plots.root";
    TFile tfin_bg{ bg_filename.c_str() };


    //Scattering
    TFile tfin_scat { "tdata/staging/km2_scat.root"};
    TFile tfin_noscat { "tdata/staging/km2_noscat.root"};
    ScatterContrib sc{ tfin_noscat, tfin_scat };

    //**************************************************
    //Merge stacks
    //**************************************************

    const  auto stack_nodes = config_node["stacks"];

    for ( auto& stack_node : stack_nodes )
    {
        //Load main files and paths
        std::string name = get_yaml<std::string>( stack_node , "name" );
        std::string filename = get_yaml<std::string>( stack_node, "filename" );
        std::string destfolder = get_yaml<std::string>( stack_node, "destfolder" );
        auto& tfin = tfl.get_tfile_opt( filename, "" );

        std::cerr << "\nProcessing " << name << std::endl;

        std::vector<std::string> paths 
            = get_yaml<std::vector<std::string>>( stack_node, "selections" );

        std::pair<std::unique_ptr<TH1>, std::unique_ptr<OwningStack>> 
            data_stack  = extract_data_stack( tfin, paths );

        TH1 * hbg = static_cast<TH1*>( data_stack.second->Stack().GetStack()->Last() );
        hbg->SetLineColor( kBlack );
        hbg->SetFillStyle( 0 );

        //save raw data and background plots
        cd_p( &tfout, destfolder );
        data_stack.first->Write("hdata");
        data_stack.second->Write("hstack");
        hbg->SetLineColor( kOrange + 2 );
        hbg->Write("hbg_raw");

        //Set up errors
        HistErrors hist_errors{};
        std::unique_ptr<HaloErrors> he;
        if ( stack_node["trigeff"] )
        {
            std::string trig_name = get_yaml<std::string>( stack_node, "trigeff" );
            hist_errors.set_trigger( *trig_effs[trig_name] ); 
        }
        if ( stack_node["scattererr"] && stack_node["scattererr"].as<bool>() )
        {
            hist_errors.set_scatter_contrib( sc );
        }
        if ( stack_node["halolog"] )
        {
            std::string halo_logfile = get_yaml<std::string>( stack_node, "halolog" );
            auto& tf_halo = tfl.get_tfile_opt( halo_logfile, "" );

            he = make_unique<HaloErrors>( tfin, tf_halo);
            std::vector<HaloErrors::halo_info> all_halo_info;

            for ( auto new_info : stack_node["haloinfo"] )
            {
                HaloErrors::halo_info new_hi{
                    get_yaml<std::string>( new_info, "bgpath" ),
                        get_yaml<std::string>( new_info, "scalepath" ),
                        get_yaml<std::string>( new_info, "logpath" ) };
                all_halo_info.push_back( new_hi );
            }

            if ( stack_node["haloshapeerr"] )
            {
                double shape_err = get_yaml<double>( stack_node, "haloshapeerr" );
                he->set_shape_error_factor( shape_err );
            }

            he->set_halo_info( all_halo_info );
            hist_errors.set_halo_errors( *he );
        }

        cd_p( &tfout, destfolder );

        if ( stack_node["cumulative_check" ] && ! stack_node["trigeff"] )
        {
            double cum_centre = get_yaml<double>( stack_node, "cumulative_check" );
            std::cerr << "Cumulative check ...  " << std::endl;
            auto h_cum_data = get_cumulative_hist( *data_stack.first, cum_centre );
            auto h_cum_mc = get_cumulative_hist( *hbg, cum_centre );
            h_cum_mc->Write( "hbg_cum" );
            h_cum_data->Write( "h_cum" );
            h_cum_data->Divide( h_cum_mc.get() );
            h_cum_data->Write( "h_cum_rat" );
        }

        if ( stack_node["trigeff"] )
        {
            std::string trig_name = get_yaml<std::string>( stack_node, "trigeff" );
            const auto& trig_eff = trig_effs.at( trig_name );
            std::cerr << "Trigger correction ...  " << std::endl;
            trig_eff->correct_hist( *hbg );
            std::cerr << "Done!" << std::endl;
            hbg->SetLineColor( kViolet +2 );
            hbg->Write( "hbg_corr" );

            if ( stack_node["cumulative_check" ] )
            {
                double cum_centre = get_yaml<double>( stack_node, "cumulative_check" );
                std::cerr << "Cumulative check ...  "  << std::endl;
                auto h_cum_data = get_cumulative_hist( *data_stack.first, cum_centre );
                auto h_cum_mc = get_cumulative_hist( *hbg, cum_centre );
                std::cerr << "Writing cum histograms ...  "  << std::endl;
                h_cum_mc->Write( "hbg_cum" );
                h_cum_data->Write( "h_cum" );
                h_cum_data->Divide( h_cum_mc.get() );
                h_cum_data->Write( "h_cum_rat" );
                std::cerr << "Done"  << std::endl;
            }
        }

        if ( stack_node["errors"] && stack_node["errors"].as<bool>() )
        {
            std::cerr << "Error computation ...  " << std::endl;
            auto h_errors = hist_errors.compute_errors( *static_cast<TH1D*>( hbg ) );
            std::cerr << "Done!" << std::endl;
            h_errors->Write( "hbg_err" );
            auto h_err_ratio = uclone(data_stack.first);
            h_err_ratio->Divide( h_errors.get() );
            h_err_ratio->Write( "h_err_ratio" );
        }
    }

    return EXIT_SUCCESS;

}
