#include "Shuffler.hh"
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
#include <boost/filesystem/path.hpp>
#include <iomanip>
#include <iostream>

namespace fn
{
    using boost::filesystem::path;

    void do_the_shuffle(const YAML::Node& output_node,
            TFile& output_file,
            std::map<std::string, MultiScaling>& scaling_info)
    {
        //Setup input
        path stack_input_file = get_yaml<std::string>( output_node, "input_file" );
        TFile tfstackin( stack_input_file.string().c_str() );
        RootTFileWrapper rtfw_stack( tfstackin );
        ChannelHistExtractor ce_stack( rtfw_stack );


        HistFormatter formatter( "input/shuffle/colors.yaml" );

        auto data_channels = get_yaml<std::vector<std::string>>(
                output_node["data_plot"], "channels" );

        //Setup output
        TFile& tfout = output_file;

        //get plot list
        const YAML::Node& plots = output_node["plots"];

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

            HistStacker hs( output_node , ce_stack, scaling_info, formatter );
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

            format_data_hist( *hdata, output_node["data_plot"] );

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
    }
}
