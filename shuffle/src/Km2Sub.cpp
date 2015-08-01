#include "Km2Sub.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "fiducial_functions.hh"

using boost::filesystem::path;

namespace fn
{
    Km2Sub::Km2Sub( TFile& tfout, TFile& tflog, std::string config_file )
        :tfout_( tfout ), tflog_( tflog )
    {
        config_node_ = YAML::LoadFile( config_file );

        path input_filename  = get_yaml<std::string>( config_node_["files"], "input" );
        tfin_ = get_tfile(  input_filename , "" );
        rtfw_ = make_unique<RootTFileWrapper>( *tfin_ );
        ce_ = make_unique<ChannelHistExtractor>( *rtfw_ );

        names_ = config_node_["names"].as<std::map<std::string, std::string>>();

        brs_ = YAML::LoadFile( "input/shuffle/branching_ratios.yaml")
            .as<std::map<std::string, double>>();

        fid_weights_ = extract_root_fiducial_weights( config_node_["fid"] );
    }

    void Km2Sub::process_all_plots()
    {
        std::vector<std::string> plots = get_yaml<std::vector<std::string>>( config_node_, "plots" );

        for ( const auto& plot_name:  plots )
        {       
            std::cerr << "Processing " << plot_name << std::endl;
            for ( const auto& pol : std::vector<std::string>{ "pos", "neg" } )
            {
                process_plot( plot_name, pol );
            }
        }

        for ( const auto& pol : std::vector<std::string>{ "pos", "neg" } )
        {
            copy_fids( names_.at( "ib"), pol, names_.at("full"), config_node_["fid"] );
        }
    }

    void Km2Sub::copy_fids( std::string src, std::string pol, std::string dest,
            const YAML::Node& fid_conf )
    {
        std::string pre = get_yaml<std::string>( fid_conf, "pre" );
        std::string post = get_yaml<std::string>( fid_conf, "post" );
        std::string root_file = get_yaml<std::string>( fid_conf, "root_file" );
        auto old_tree_path = boost::filesystem::path{ pre } / (src + "." + pol) / post;
        auto new_tree_path = boost::filesystem::path{ pre } / (dest + "." + pol) / post;

        auto tfile = get_tfile( root_file );
        auto old_tree = get_object<TTree>( *tfile, old_tree_path );

        cd_p( &tfout_, new_tree_path.parent_path() );
        TTree * new_tree = old_tree->CloneTree();
        new_tree->Write();
    }

    void Km2Sub::process_plot( std::string plot_name, std::string pol )
    {
        ce_->set_post_path( plot_name );

        auto ib_sum = get_sum(  "ib", pol );
        auto h_raw = uclone( ib_sum );

        auto intp_sum = get_sum( "intp", pol );
        scale_hist( *intp_sum, "intp", pol );

        auto intm_sum = get_sum( "intm", pol );
        scale_hist( *intm_sum, "intm", pol );

        auto sdp_sum = get_sum( "sdp", pol );
        scale_hist( *sdp_sum, "sdp", pol );

        auto sdm_sum = get_sum( "sdm", pol );
        scale_hist( *sdm_sum, "sdm", pol );

        ib_sum->Add( intp_sum.get() );
        ib_sum->Add( intm_sum.get() );
        ib_sum->Add( sdp_sum.get() );
        ib_sum->Add( sdm_sum.get() );

        path write_path = path{ names_["full"] + "." + pol }  / plot_name;
        cd_p( &tfout_, write_path.parent_path() ) ;
        ib_sum->Write( write_path.filename().string().c_str() );

        cd_p( &tflog_, path{"km2_sub"} / write_path );
        h_raw->Write( "hraw" );
        ib_sum->Write( "hsum" );
        intp_sum->Write( "hintp" );
        intm_sum->Write( "hintm" );
        sdp_sum->Write( "hsdp" );
        sdm_sum->Write( "hsdm" );

        cd_p( &tflog_, path{"km2_sub"} / ( write_path.string() + "_rebin") );
        h_raw->Rebin(25);
        ib_sum->Rebin(25);
        intp_sum->Rebin(25);
        intm_sum->Rebin(25);
        sdp_sum->Rebin(25);
        sdm_sum->Rebin(25);

        h_raw->Write( "hraw" );
        ib_sum->Write( "hsum" );
        intp_sum->Write( "hintp" );
        intm_sum->Write( "hintm" );
        sdp_sum->Write( "hsdp" );
        sdm_sum->Write( "hsdm" );
    }

    std::unique_ptr<TH1> Km2Sub::get_sum( std::string chan, std::string pol )
    {
        std::string full_chan = names_.at( chan ) + "." + pol;
        std::vector<std::string> channels = { full_chan };
        return get_summed_histogram( *ce_, begin( channels ), end( channels ));
    }

    void Km2Sub::scale_hist( TH1& h, std::string chan, std::string pol )
    {
        std::string full_chan = names_.at( chan ) + "." + pol;
        std::string br_name = "km2g" + chan;
        std::string ib_chan = names_.at( "ib" ) + "." + pol;
        double fid_rat = fid_weights_.at( ib_chan  ) / fid_weights_.at( full_chan );
        double scale = fid_rat * brs_.at( br_name ) / brs_.at( "km2");
        h.Scale( scale );
    }
}
