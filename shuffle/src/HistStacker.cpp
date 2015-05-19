#include "HistStacker.hh"
#include "yaml_help.hh"
#include "Rtypes.h"
#include <iomanip>

namespace fn
{
    RootColors::RootColors()
        :colors_{ 
            {"kWhite", kWhite}, {"kBlack", kBlack},
                {"kGray", kGray}, {"kRed", kRed},
                {"kGreen", kGreen}, {"kBlue", kBlue},
                {"kYellow", kYellow}, {"kMagenta", kMagenta},
                {"kCyan", kCyan}, {"kOrange", kOrange},
                {"kSpring", kSpring}, {"kTeal", kTeal},
                {"kAzure", kAzure}, {"kViolet", kViolet},
                {"kPink", kPink}
        } {}

    int RootColors::operator()( std::string name ) const
    { 
        int kcolor = 0;
        try {kcolor = colors_.at( name); } 
        catch ( std::out_of_range& e )
        { throw std::out_of_range( "Unkown ROOT color " + name );}
        return kcolor;
    }

    int root_color( std::string name )
    {
        static const RootColors rc;
        return rc( name );
    }

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
    }

    void format_data_hist( TH1 &h, const YAML::Node& instruct )
    {
        h.SetMarkerSize( 2 );
        h.SetMarkerColor( kRed);
        h.SetLineColor( kRed );

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
            std::map<std::string, Km2Scaling>& scaling_info, const HistFormatter& formatter)
        :stack_instructions_( stack_instructions), tfin_( tfin ),
        scaling_info_( scaling_info ), formatter_( formatter )
    { }

    void HistStacker::create_stack()
    {
        const YAML::Node & stack_node = stack_instructions_["stack"];
        assert(stack_node.IsSequence());

        for ( auto  item : stack_node )
        {
            const YAML::Node& instruct =  item["item"];
            auto h = load_hist(instruct);
            scale_hist( *h, instruct );
            color_hist( *h, instruct );
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

        const auto& scaling =
            scaling_info_.at( get_yaml<string>( instruct, "scaling" ) );

        scaling.scale_hist( h, instruct );
    }

    void HistStacker::color_hist(  TH1& h, const YAML::Node& instruct  )
    {
        using std::string;
        formatter_.format( h, get_yaml<string>( instruct, "type" ));
    }

    void HistStacker::write( std::string name)
    {
        stack_.Write( name );
    }

    void HistStacker::write_total( std::string name)
    {
        auto htotal = stack_.GetTotal();
        htotal->Write( name.c_str() );
    }

    std::unique_ptr<TH1> HistStacker::get_total_copy()
    {
        std::unique_ptr<TH1> result{ static_cast<TH1*>( stack_.GetTotal()->Clone( "htotal" )) };
        result->SetDirectory(0);
        return result;
    }
}
