#include "HistStacker.hh"
#include "yaml_help.hh"
#include "root_help.hh"
#include "Rtypes.h"
#include <iomanip>

namespace fn
{

    HistFormatter::HistFormatter( std::string filename )
    {
        auto config_node = YAML::LoadFile( filename );
        const auto& channel_colors = config_node["channels"];

        for ( const auto& channel : channel_colors )
        {
            std::string name = channel.first.as<std::string>();
            const auto& format_node = channel.second;
            int value = root_color( get_yaml<std::string>( format_node, "kcolor" ) )
                + get_yaml<int>( format_node, "colormod" );

            type_colors_.insert( make_pair( name, value ) );
        }
    }

    void HistFormatter::format( TH1& h , std::string type ) const
    { 
        Color_t color = type_colors_.at( type );
        //std::cerr << std::setw(15) << "Coloring " << std::setw(10) << type << " " << color << std::endl;
        h.SetFillColor( color );
        h.SetLineColor( color );
        h.SetLineWidth( 1 );
    }

    void format_data_hist( TH1 &h, const YAML::Node& instruct )
    {
        h.SetMarkerSize( 2 );
        h.SetMarkerColor( kGray + 2);
        h.SetLineColor( kGray +2 );
        h.SetLineWidth( 1 );
        //h.Sumw2();

        if ( get_yaml<bool>( instruct, "do_blind" ) )
        {
            double min_blind =  get_yaml<double>( instruct, "min_blind" ) ;
            double max_blind =  get_yaml<double>( instruct, "max_blind" ) ;

            int min_blind_bin = h.GetXaxis()->FindBin( min_blind);
            int max_blind_bin = h.GetXaxis()->FindBin( max_blind );

            //std::cerr << "Blind range: " << min_blind << " - " <<  max_blind << "\n";
            //std::cerr << "Blind bin range: " << min_blind_bin << " - " <<  max_blind_bin << "\n";

            for ( int bin = min_blind_bin ; bin != max_blind_bin + 1 ; ++bin )
            {
                h.SetBinContent( bin, 0 );
            }
        }
    }

    //--------------------------------------------------

    HistStacker::HistStacker( const YAML::Node& stack_instructions, FNTFile& tfin,
            scaling_map& scaling_info, const HistFormatter& formatter)
        :stack_instructions_( stack_instructions), tfin_( tfin ),
        scaling_info_( scaling_info ), formatter_( formatter )
    { }

    void HistStacker::create_stack()
    {
        const YAML::Node & stack_node = stack_instructions_["stack"];
        assert(stack_node.IsSequence());

        if ( stack_instructions_["stack_scaling"] )
        {
            default_scaling_ = get_yaml<std::string>( stack_instructions_, "stack_scaling" );
        }

        for ( auto  item : stack_node )
        {
            const YAML::Node& instruct =  item["item"];
            auto h = load_hist(instruct);
            scale_hist( *h, instruct );
            color_hist( *h, instruct );
            //de_zero_hist( *h );
            stack_.Add( std::move( h ) );
        }
    }

    std::unique_ptr<TH1> HistStacker::load_hist( const YAML::Node& instruct )
    {
        using std::vector;
        using std::string;

        auto channels = get_yaml<vector<string>>( instruct, "channels");
        auto hsummed = get_summed_histogram( tfin_, std::begin( channels), std::end( channels ) );
        hsummed->SetName( get_yaml<string>( instruct, "name" ).c_str() );
        return hsummed;
    }

    void HistStacker::scale_hist(  TH1& h, const YAML::Node& instruct )
    {
        using std::string;

        string name = get_yaml<string>( instruct, "name" );

        string scaling_name = "";

        if ( instruct["scaling"] )
        {
            scaling_name = get_yaml<string>( instruct, "scaling" );
        }
        else if ( default_scaling_ )
        {
            scaling_name = *default_scaling_;
        }
        else
        {
            throw std::runtime_error( "No scalign info for " + name );
        }

        try
        {
            const auto& scaling = scaling_info_.at( scaling_name);
            double scale_factor = scaling->scale_hist( h, instruct );
            scale_factors.push_back( std::make_pair( name, scale_factor ) );
        }
        catch( std::exception& e )
        {
            std::cout << "Problem looking for : " << scaling_name << std::endl;
            for ( auto& si : scaling_info_ )
            {
                std::cout << si.first << std::endl;
            }
            throw;
        }
    }

    void HistStacker::color_hist(  TH1& h, const YAML::Node& instruct  )
    {
        using std::string;
        formatter_.format( h, get_yaml<string>( instruct, "type" ));
    }

    void HistStacker::write( std::string name)
    {
        stack_.Write( name );
        for( const auto& sf : scale_factors )
        {
            store_value( sf.first, sf.second );
        }
    }

    void HistStacker::write_total( std::string name)
    {
        auto htotal = stack_.GetTotal();
        htotal->SetFillStyle(0);
        htotal->Write( name.c_str() );
    }

    std::unique_ptr<TH1> HistStacker::get_total_copy()
    {
        std::unique_ptr<TH1> result{ static_cast<TH1*>( stack_.GetTotal()->Clone( "htotal" )) };
        result->SetDirectory(0);
        return result;
    }

    void de_zero_hist( TH1& h )
    {
        int nBins = h.GetNbinsX();

        for ( int ibin = 0 ; ibin != nBins +1 ; ++ ibin )
        {
            double value = h.GetBinContent( ibin );
            if ( value < 0 ){ h.SetBinContent( ibin, 0 );
            }
        }
    }

}
