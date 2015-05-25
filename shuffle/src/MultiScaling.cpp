#include "MultiScaling.hh"
#include "HistExtractor.hh"
#include "TFile.h"
#include "TH1.h"
#include "TKey.h"
#include "TClass.h"
#include <TROOT.h>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <iomanip>
namespace fn
{
    std::ostream& operator<<( std::ostream& os , const scale_result& sr )
    {
        os << "hs: " << sr.halo_scale << " hse: " << sr.halo_scale_error 
            << " km2: " << sr.km2_scale << " km2e: " << sr.km2_scale_error;
        return os;
    }

    void ScaleStrategy::update_scaling()
    { 
        sr = compute_scaling();
        std::cout << sr << std::endl;
    }

    M2ScaleStrategy::M2ScaleStrategy(
            const YAML::Node & channel_node,
            const YAML::Node & strat_node)
        :channel_node_( channel_node), strat_node_( strat_node )
    {
        using std::vector;
        using std::string;
        using boost::filesystem::path;

        //std::cout << "M2ScaleStrategy channel_node: " << channel_node_ << std::endl;
        //std::cout << "M2ScaleStrategy strat_node: " << strat_node_ << std::endl;

        //Channel definitions
        halo_channels = get_yaml<vector<string>>( channel_node_, "halo_channels" );
        data_channels = get_yaml<vector<string>>( channel_node_, "data_channels" );
        km2_channels  = get_yaml<vector<string>>( channel_node_, "km2_channels" );

        //Setup input file
        input_file = get_yaml<std::string>( strat_node_, "input_file" );

        m2_min_halo = get_yaml<double>( strat_node_, "min_mass_halo" );
        m2_max_halo = get_yaml<double>( strat_node_, "max_mass_halo" );

        m2_min_km2 = get_yaml<double>( strat_node_, "min_mass_peak" );
        m2_max_km2 = get_yaml<double>( strat_node_, "max_mass_peak" );

        post_path = get_yaml<std::string>( strat_node_, "plot_path") ;
    }

    scale_result M2ScaleStrategy::compute_scaling()
    {
        using std::vector;
        using std::string;
        using boost::filesystem::path;

        //Setup input file
        TFile tf( input_file.string().c_str() );
        RootTFileWrapper rtfw( tf);
        ChannelHistExtractor ce( rtfw);

        ce.set_post_path( post_path );

        //Extract histograms
        auto hsummed_halo = get_summed_histogram( 
                ce, begin( halo_channels ), end( halo_channels ) );

        auto hsummed_data = get_summed_histogram( 
                ce, begin( data_channels ), end( data_channels ) );

        auto hsummed_km2 = get_summed_histogram( 
                ce, begin( km2_channels ), end( km2_channels ) );

        //Do halo division 
        double halo_halo_integral =  integral( *hsummed_halo, m2_min_halo, m2_max_halo );
        double halo_data_integral =  integral( *hsummed_data, m2_min_halo, m2_max_halo );

        double halo_scale = halo_data_integral / halo_halo_integral;

        double halo_scale_error = halo_scale * 
            ( 1 / std::sqrt( halo_halo_integral ) + 1 / std::sqrt( halo_data_integral ) );

        //Do km2 division
        double km2_halo_integral =  integral( *hsummed_halo, m2_min_km2, m2_max_km2 );
        double km2_data_integral =  integral( *hsummed_data, m2_min_km2, m2_max_km2 );
        double km2_km2_integral =  integral( *hsummed_km2, m2_min_km2, m2_max_km2 );

        double subtracted_km2 = km2_data_integral -  halo_scale * km2_halo_integral ;
        double subtracted_km2_error = std::sqrt( km2_data_integral + km2_halo_integral );

        std::cout << "KM2 SCALING: " << km2_halo_integral << " " << km2_data_integral << " " 
            << km2_km2_integral << std::endl;

        double km2_scale =  subtracted_km2  /  km2_km2_integral;
        double km2_scale_error = km2_scale *
            ( subtracted_km2_error / subtracted_km2 + 1 / sqrt( km2_km2_integral ) );

        return scale_result{ halo_scale, halo_scale_error, km2_scale, km2_scale_error };
    }

    //--------------------------------------------------

    MultiScaling::MultiScaling( 
            const YAML::Node& scaling_config, 
            const std::map<std::string, double>& fiducial_weights,
            const std::map<std::string, double>& brs)
        :scaling_config_( scaling_config), fiducial_weights_( fiducial_weights), brs_( brs )
    {
        using std::vector;
        using std::string;

        const YAML::Node& channel_node_ = scaling_config_["channels"];

        mc_strat_.reset( new M2ScaleStrategy{
                channel_node_, scaling_config["mc"] } );

        halo_strat_.reset( new M2ScaleStrategy{
                channel_node_, scaling_config["halo"] } );

        //Record km2 fiducial weight and flux
        km2_channels_ = get_yaml<vector<string>>( channel_node_, "km2_channels" );
    }

    void MultiScaling::compute_scaling() 
    {
        //compute scale factors
        mc_strat_->update_scaling();
        halo_strat_->update_scaling();

        km2_fid_weight_ = 0;
        for ( auto km2_channel : km2_channels_ )
        {
            km2_fid_weight_ += fiducial_weights_.at( km2_channel );
        }

        km2_br_ = brs_.at( "km2" );
    }

    double MultiScaling::get_halo_scale() const
    {
        return halo_strat_->get_halo_scale();
    }
    double MultiScaling::get_halo_scale_error() const
    {
        return halo_strat_->get_halo_scale_error();
    }

    double MultiScaling::get_km2_scale() const
    {
        return mc_strat_->get_km2_scale();
    }

    double MultiScaling::get_km2_scale_error() const
    {
        return mc_strat_->get_km2_scale_error();
    }

    void MultiScaling::scale_hist( TH1& h, const YAML::Node& instruct ) const
    {
        double scale_factor = 1;
        std::string type = get_yaml<std::string>( instruct, "type" );

        if ( type == "halo" )
        {
            scale_factor = halo_strat_->get_halo_scale();
        }
        else if ( type == "data" )
        {
            scale_factor = 1; 
        }
        else
        {
            double fid_weight = fiducial_weights_.at( get_yaml<std::string>( instruct, "fid_weight" ) );
            double br = brs_.at( type );
            scale_factor = mc_strat_->get_km2_scale() *  km2_fid_weight_  / fid_weight * br / km2_br_;

        }
        h.Scale( scale_factor );
    }
}
