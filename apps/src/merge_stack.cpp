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
#include <vector>


std::pair<std::unique_ptr<TH1>, std::unique_ptr<OwningStack>> extract( TFile& tfin, std::vector<std::string> paths)
{
    std::vector<std::string> stack_paths;
    std::vector<std::string> data_paths;

    for( auto fp : paths  )
    {
        stack_paths.push_back( ( path{fp} / "hnu_stack" ).string() );
        data_paths.push_back( ( path{fp} / "hdata" ).string() );
    }

    auto stacks = extract_object_list<THStack>( tfin, stack_paths );
    auto owning_stack_sum =  sum_stacks(begin( stacks ), end( stacks ) );

    auto data_hists = extract_hist_list<TH1D>( tfin, data_paths );
    auto data_sum = sum_hists( begin( data_hists), end( data_hists ) );

    return std::make_pair( std::move( data_sum ),  std::move( owning_stack_sum ) );
} 

void merge( TFile& tfin, TFile& tfout, std::string output_folder, std::vector<std::string> paths  )
{
    auto data_stack = extract( tfin, paths );
    cd_p( &tfout, output_folder );
    data_stack.first->Write("hdata");
    data_stack.second->Write("hnu_stack");
}

std::pair<std::unique_ptr<TH1>, std::unique_ptr<TH1>> get_trigger_efficiency( TFile& tf )
{
    std::vector<std::string> sels
    {
        "p5.data.q1.v4.neg/sig_up_trig_eff"
            "p5.data.q1.v4.neg/sig_dn_trig_eff"
            "p5.data.q1.v4.pos/sig_up_trig_eff"
            "p5.data.q1.v4.pos/sig_dn_trig_eff"
    };

    TriggerApp ta{ tf};
    ta.set_sels( sels );
    ta.init();

    auto h_passed = std::unique_ptr<TH1D> ( tclone( ta.get_h_passed()) );
    auto h_all = std::unique_ptr<TH1D> ( tclone(ta.get_h_all()) );

    std::cout << h_all->GetNbinsX() << std::endl;

    return std::make_pair( std::move(h_all), std::move(h_passed) );
}

std::unique_ptr<TH1D> get_cumulative_hist( TH1& h, double centre )
{
    TAxis * ax = h.GetXaxis();
    int nbins  = h.GetNbinsX();
    int centre_bin = ax->FindBin( centre );
    double bin_centre = ax->GetBinCenter( centre_bin );

    int centre_up;
    int centre_down;

    if ( bin_centre > centre )
    {
        centre_up = centre_bin;
        centre_down = centre_bin - 1 ;
    }
    else
    {
        centre_down = centre_bin;
        centre_up = centre_bin + 1;
    }

    int upper_dist = nbins - centre_up;
    int lower_dist = centre_down;

    int half_nbins = std::min( upper_dist, lower_dist ) - 1;
    int min_bin = centre_down  -  ( half_nbins - 1 );
    int max_bin = centre_up +  ( half_nbins - 1 );

    auto result = makeTH1D( "h_cum", "Cumulative", ( max_bin - min_bin + 1 ) / 2,
            h.GetXaxis()->GetBinLowEdge( centre_up ),
            h.GetXaxis()->GetBinUpEdge( max_bin ), h.GetXaxis()->GetTitle() );

    int my_min_bin = centre_down;
    int my_max_bin = centre_up;
    while( my_max_bin - centre_down < half_nbins )
    {
        double val = h.Integral( my_min_bin, my_max_bin );
        double fill_point = ax->GetBinCenter( my_max_bin ) - ax->GetBinLowEdge( centre_up );
        result->SetBinContent( result->GetXaxis()->FindBin( fill_point ), val );
        ++my_max_bin;
        --my_min_bin;
    }

    return result;
}

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
            trig_eff->set_sels( sels );
            trig_eff->init();

            trig_effs.insert( std::make_pair( name, std::move( trig_eff ) ) );
        }
    }

    //**************************************************
    //Merge stacks
    //**************************************************

    const  auto stack_nodes = config_node["stacks"];

    for ( auto& stack_node : stack_nodes )
    {
        std::string name = get_yaml<std::string>( stack_node , "name" );
        std::string filename = get_yaml<std::string>( stack_node, "filename" );
        std::string destfolder = get_yaml<std::string>( stack_node, "destfolder" );
        auto& tfin = tfl.get_tfile_opt( filename, "" );

        std::vector<std::string> paths 
            = get_yaml<std::vector<std::string>>( stack_node, "selections" );

        auto data_stack  = extract( tfin, paths );

        TH1 * hbg = static_cast<TH1*>( data_stack.second->Stack().GetStack()->Last() );
        hbg->SetLineColor( kBlack );
        hbg->SetFillStyle( 0 );


        cd_p( &tfout, destfolder );
        data_stack.first->Write("hdata");
        data_stack.second->Write("hstack");
        hbg->SetLineColor( kOrange + 2 );
        hbg->Write("hbg_raw");

        if ( stack_node["cumulative_check" ] && ! stack_node["trigeff"] )
        {
                double cum_centre = get_yaml<double>( stack_node, "cumulative_check" );
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
            trig_eff->correct_hist( *hbg );
            hbg->SetLineColor( kViolet +2 );
            hbg->Write( "hbg_corr" );

            if ( stack_node["cumulative_check" ] )
            {
                double cum_centre = get_yaml<double>( stack_node, "cumulative_check" );
                auto h_cum_data = get_cumulative_hist( *data_stack.first, cum_centre );
                auto h_cum_mc = get_cumulative_hist( *hbg, cum_centre );
                h_cum_mc->Write( "hbg_cum" );
                h_cum_data->Write( "h_cum" );
                h_cum_data->Divide( h_cum_mc.get() );
                h_cum_data->Write( "h_cum_rat" );
            }
        }
    }

    return EXIT_SUCCESS;

}
