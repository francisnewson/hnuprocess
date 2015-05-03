#include "easy_app.hh"
#include "TH2D.h"
#include "yaml_help.hh"
#include "TFile.h"
#include "TEfficiency.h"
#include "root_help.hh"
#include <boost/filesystem/path.hpp>

using boost::filesystem::path;

int main( int argc, char * argv[] )
{

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

    std::ofstream of;
    std::string output_location = "std::cout";
    bool output_to_file = vm.count( "output" );
    if ( output_to_file )
    {
        output_location = vm["output"].as<path>().string() ;
        of.open( output_location );
    }
    std::ostream& os =  output_to_file  ? of : std::cout;

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

    std::string input_file = get_yaml<std::string>( config_node, "input_file" );
    TFile tfin( input_file.c_str() );

    int rebin = get_yaml<int>( config_node, "rebin" );

    //Sort out folder names
    std::string channel = get_yaml<std::string>( config_node, "channel" );
    path hist_folder = get_yaml<std::string>( config_node, "hist_folder" );
    std::string passed_name = get_yaml<std::string>( config_node, "passed_name" );
    std::string total_name = get_yaml<std::string>( config_node, "total_name" );

    path pos_folder = path{channel + ".pos"} / hist_folder;
    path neg_folder = path{channel + ".neg"} / hist_folder;

    //Load histograms
    auto h_pos_passed = extract_hist<TH2D>( tfin, pos_folder/ passed_name );
    auto h_pos_total  = extract_hist<TH2D>( tfin, pos_folder/ total_name );

    auto h_neg_passed = extract_hist<TH2D>( tfin, neg_folder/ passed_name );
    auto h_neg_total  = extract_hist<TH2D>( tfin, neg_folder/ total_name );

    //rebin
    h_pos_passed->Rebin2D( rebin , rebin );
    h_neg_passed->Rebin2D( rebin , rebin );
    h_pos_total->Rebin2D( rebin , rebin );
    h_neg_total->Rebin2D( rebin , rebin );

    //combine
    h_pos_passed->Add( h_neg_passed.get() );
    h_pos_total->Add( h_neg_total.get() );

    //compute efficiency
    TEfficiency eff_xy( *h_pos_passed, *h_pos_total );

    //extract axes
    TAxis * tx = h_pos_passed->GetXaxis();
    TAxis * ty = h_pos_passed->GetYaxis();
    int nXBins = h_pos_passed->GetNbinsX();
    int nYBins = h_pos_passed->GetNbinsY();

    //Collect x_edges
    std::vector<double> x_edges;
    for ( int ix = 1 ; ix <= nXBins ; ++ix )
    {
        x_edges.push_back( tx->GetBinLowEdge( ix ) );
    }
    x_edges.push_back( tx->GetBinLowEdge( nXBins ) + tx->GetBinWidth( nXBins ) );

    //Collect y_edges
    std::vector<double> y_edges;
    for ( int iy = 1 ; iy <= nYBins ; ++iy )
    {
        y_edges.push_back( ty->GetBinLowEdge( iy ) );
    }
    y_edges.push_back( ty->GetBinLowEdge( nXBins ) + ty->GetBinWidth( nXBins ) );

    std::vector<double> effs;
    for ( int ix = 1 ; ix <= nXBins ; ++ix )
    {
        for ( int iy = 1 ; iy <= nYBins ; ++iy )
        {
            int global_bin  = h_pos_passed->GetBin( ix, iy );
            effs.push_back( eff_xy.GetEfficiency( global_bin ) );
        }
    }

    std::map<std::string,std::vector<double>> output_data = {
        { "xbins" , x_edges }, {"ybins", y_edges}, {"effs", effs } };

    YAML::Node output_yaml{ output_data};

    os << "# MUV XY effs producted from " <<  mission.string() << std::endl;
    os << output_yaml << std::endl;

    std::cerr << "MUV effs written to: " << output_location << std::endl;
}
