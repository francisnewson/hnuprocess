#include "easy_app.hh"
#include "stl_help.hh"
#include "TH2D.h"
#include "yaml_help.hh"
#include "TFile.h"
#include "TEfficiency.h"
#include "root_help.hh"
#include "HistStore.hh"
#include "TColor.h"
#include "TROOT.h"
#include "TGraphAsymmErrors.h"
#include <boost/filesystem/path.hpp>

using boost::filesystem::path;

struct eff_1d_plots
{
    std::unique_ptr<TH1D> htotal;
    std::unique_ptr<TH1D> hpassed;
};

eff_1d_plots get_eff_1d_plots( TFile& tfin, path hist_folder, std::string channel, 
        std::string passed_name, std::string total_name, int rebin )
{
    path pos_folder = path{channel + ".pos"} / hist_folder;
    path neg_folder = path{channel + ".neg"} / hist_folder;

    //Load histograms
    auto h_pos_passed = extract_hist<TH1D>( tfin, pos_folder/ passed_name );
    auto h_pos_total  = extract_hist<TH1D>( tfin, pos_folder/ total_name );

    auto h_neg_passed = extract_hist<TH1D>( tfin, neg_folder/ passed_name );
    auto h_neg_total  = extract_hist<TH1D>( tfin, neg_folder/ total_name );

    //rebin
    h_pos_passed->Rebin( rebin );
    h_neg_passed->Rebin( rebin );
    h_pos_total->Rebin( rebin );
    h_neg_total->Rebin( rebin );

    //combine
    h_pos_passed->Add( h_neg_passed.get() );
    h_pos_total->Add( h_neg_total.get() );

    eff_1d_plots result{ 
        std::unique_ptr<TH1D>{ static_cast<TH1D*>( h_pos_total->Clone("htotal") )},
            std::unique_ptr<TH1D>{ static_cast<TH1D*>( h_pos_passed->Clone("hpassed") )}
    };

    result.htotal->SetDirectory(0);
    result.hpassed->SetDirectory(0);

    return result;
}

struct eff_2d_plots
{
    std::unique_ptr<TH2D> htotal;
    std::unique_ptr<TH2D> hpassed;
};

eff_2d_plots get_eff_2d_plots( TFile& tfin, path hist_folder, std::string channel, std::string passed_name, std::string total_name, int rebin )
{
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

    eff_2d_plots result{ 
        std::unique_ptr<TH2D>{ static_cast<TH2D*>( h_pos_total->Clone("htotal") )},
            std::unique_ptr<TH2D>{ static_cast<TH2D*>( h_pos_passed->Clone("hpassed") )}
    };

    result.htotal->SetDirectory(0);
    result.hpassed->SetDirectory(0);

    return result;
}

struct eff_plots
{
    std::unique_ptr<TH1> htotal;
    std::unique_ptr<TH1> hpassed;
};

eff_plots get_eff_plots( const YAML::Node& config, TFile& tfin )
{
    int rebin               = get_yaml<int>( config, "rebin" );
    std::string channel     = get_yaml<std::string>( config, "channel" );
    std::string type        = get_yaml<std::string>( config, "type" );
    path hist_folder        = get_yaml<std::string>( config, "hist_folder" );
    std::string passed_name = get_yaml<std::string>( config, "passed_name" );
    std::string total_name  = get_yaml<std::string>( config, "total_name" );

    eff_plots result;

    if ( type == "1d" )
    {
        eff_1d_plots temp_plots = get_eff_1d_plots( tfin, hist_folder, channel, passed_name, total_name, rebin );
        result.htotal.reset( temp_plots.htotal.release() );
        result.hpassed.reset( temp_plots.hpassed.release() );
    }
    else if ( type == "2d" )
    {
        eff_2d_plots temp_plots = get_eff_2d_plots( tfin, hist_folder, channel, passed_name, total_name, rebin );
        result.htotal.reset( temp_plots.htotal.release() );
        result.hpassed.reset( temp_plots.hpassed.release() );
    }

    result.htotal->Sumw2();
    result.hpassed->Sumw2();

    return result;
}


void print_effs( std::ostream& os,  TH2D& hpassed, TEfficiency& teff )
{
    //extract axes
    TAxis * tx = hpassed.GetXaxis();
    TAxis * ty = hpassed.GetYaxis();
    int nXBins = hpassed.GetNbinsX();
    int nYBins = hpassed.GetNbinsY();

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
            int global_bin  = hpassed.GetBin( ix, iy );
            effs.push_back( teff.GetEfficiency( global_bin ) );
        }
    }

    std::map<std::string,std::vector<double>> output_data = {
        { "xbins" , x_edges }, {"ybins", y_edges}, {"effs", effs } };

    YAML::Node output_yaml{ output_data};
    os << output_yaml << std::endl;
}

int main( int argc, char * argv[] )
{

    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/extract_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream flaunch( "elaunch.log", std::ofstream::app );
        write_launch ( argc, argv, flaunch );
    }


    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "mission,m", po::value<path>(),  "Specify mission yaml file")
        ( "output,o", po::value<path>(),  "Specify output file")
        ( "root,r", po::value<path>(),  "Specify output root file")
        ( "plots,p", po::value<path>(),  "Specify plots root file")
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

    bool output_to_root = vm.count( "root" );

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



    //--------------------------------------------------

    std::unique_ptr<TFile> tfout;
    std::string output_root_file;
    if ( output_to_root )
    {
        output_root_file = vm["root"].as<path>().string();
        tfout.reset( TFile::Open(  output_root_file.c_str() , "RECREATE" ) );
    }

    //**************************************************
    //Load plots
    //**************************************************

    const YAML::Node& channels = config_node["channels"];
    const YAML::Node& plots = config_node["plots"];

    for ( const auto& channel_node : channels )
    {
        std::string channel = get_yaml<std::string>( channel_node, "name" );
        int color = get_yaml<int>( channel_node, "color");

        for( const YAML::Node& plot_node : plots )
        {
            //get plots
            YAML::Node plot = plot_node;
            plot["channel"] = channel;
            eff_plots ep = get_eff_plots( plot, tfin );


            std::unique_ptr<TH1> rat_xy{ static_cast<TH1*>( ep.hpassed->Clone("hrat_xy")) };
            rat_xy->Divide( ep.htotal.get() );
            rat_xy->SetLineColor( color );

            std::string plot_name = get_yaml<std::string>( plot, "name" );
            auto plot_folder =  boost::filesystem::path{ plot_name } / channel;

            if ( output_to_root )
            {
                cd_p( tfout.get(), plot_folder  );
                rat_xy->Write( "rat" );

                //compute efficiency
                if ( true || TEfficiency::CheckConsistency( *ep.hpassed, *ep.htotal ) )
                {
                    TEfficiency eff_xy( *ep.hpassed, *ep.htotal );
                    eff_xy.Write( "eff" );

                    if ( plot["type"].as<std::string>() == "1d" )
                    {
                        TGraphAsymmErrors * g = eff_xy.CreateGraph();
                        g->SetLineColor( color );
                        g->Write("g");
                    }
                }

                std::cerr << "Efficiencies written to " << output_root_file << "::" << plot_folder << std::endl;
            }

#if 0
            //print effs to file
            if ( output_to_file )
            {
                os << "# MUV XY effs producted from " <<  mission.string() << std::endl;
                print_effs( os, *static_cast<TH2D*>(ep.hpassed.get()),  eff_xy );

                std::cerr << "Efficiencies written to " << output_location << std::endl;
            }
#endif 
        }
    }

    //**************************************************
    //Produce ratios
    //**************************************************

    tfout->ReOpen( "UPDATE");

    const YAML::Node& ratios = config_node["ratios"];

    for ( const YAML::Node& ratio : ratios )
    {
        std::cout << ratio << std::endl;
        auto source  = get_yaml<std::string>(ratio, "source" );
        auto top     = get_yaml<std::string>( ratio,"top" );
        auto bottom  = get_yaml<std::string>(ratio, "bottom" );
        auto dest    = get_yaml<std::string>( ratio, "dest" );

        path ptop = path{ source } / top / "rat";
        path pbottom = path{ source } / bottom / "rat";

        auto htop = extract_hist<TH1>( *tfout, ptop );
        auto hbottom = extract_hist<TH1>( *tfout, pbottom );

        htop->Divide( hbottom.get() );

        cd_p( tfout.get(), dest );
        htop->Write("ratrat");

        if ( const auto& external = ratio["external"] )
        {
            std::string output_file = external.as<std::string>();
            TFile texternal( output_file.c_str(), "RECREATE" );
            htop->Write("rat_p");
        }
    }
}

