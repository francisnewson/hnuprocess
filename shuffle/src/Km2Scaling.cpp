#include "Km2Scaling.hh"
#include "HistExtractor.hh"
#include "TFile.h"
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <iomanip>

using boost::filesystem::path;

namespace fn
{
    Km2Scaling::Km2Scaling( const YAML::Node& scaling_config , 
            const std::map<std::string, double>& fiducial_weights,
            const std::map<std::string, double>& brs
            )
        :scaling_config_( scaling_config ), 
        fiducial_weights_( fiducial_weights), brs_( brs )
    {}

    void Km2Scaling::compute_scaling()
    {
        using std::vector;
        using std::string;

        //Channel definitions
        auto halo_channels = get_yaml<vector<string>>( scaling_config_, "halo_channels" );
        auto data_channels = get_yaml<vector<string>>( scaling_config_, "data_channels" );
        auto km2_channels = get_yaml<vector<string>>( scaling_config_, "km2_channels" );

        //COMPUTE HALO SCALE
        {
            //Setup halo input file
            const YAML::Node halo_node = scaling_config_["halo"];
            path halo_input_file = get_yaml<std::string>( halo_node, "input_file" );

            TFile tf_halo( halo_input_file.string().c_str() );
            RootTFileWrapper rtfw_halo( tf_halo );
            ChannelHistExtractor ce_halo( rtfw_halo );
            ce_halo.set_post_path( get_yaml<std::string>( halo_node, "plot_path") );

            //Extract halo histograms
            auto hsummed_halo = get_summed_histogram( 
                    ce_halo, begin( halo_channels ), end( halo_channels ) );

            auto hsummed_data = get_summed_histogram( 
                    ce_halo, begin( data_channels ), end( data_channels ) );


            //Do integrals
            double m2_min_halo = get_yaml<double>( halo_node, "min_mass" );
            double m2_max_halo = get_yaml<double>( halo_node, "max_mass" );

            double halo_halo_integral =  integral( *hsummed_halo, m2_min_halo, m2_max_halo );
            double halo_data_integral =  integral( *hsummed_data, m2_min_halo, m2_max_halo );

            halo_scale_ = halo_data_integral / halo_halo_integral;

            halo_scale_error_ = halo_scale_ * 
                ( 1 / std::sqrt( halo_halo_integral ) + 1 / std::sqrt( halo_data_integral ) );
        }

        //COMPUTE KM2 SCALE
        {
            //Setup km2 input file
            const YAML::Node km2_node = scaling_config_["km2"];
            path km2_input_file = get_yaml<std::string>( km2_node, "input_file" );

            TFile tf_km2( km2_input_file.string().c_str() );
            RootTFileWrapper rtfw_km2( tf_km2 );
            ChannelHistExtractor ce_km2( rtfw_km2 );
            ce_km2.set_post_path( get_yaml<std::string>( km2_node, "plot_path") );

            //Extract km2 histograms
            auto hsummed_halo = get_summed_histogram( 
                    ce_km2, begin( halo_channels ), end( halo_channels ) );

            auto hsummed_data = get_summed_histogram( 
                    ce_km2, begin( data_channels ), end( data_channels ) );

            auto hsummed_km2 = get_summed_histogram( 
                    ce_km2, begin( km2_channels ), end( km2_channels ) );

            //Do integrals
            double m2_min_km2 = get_yaml<double>( km2_node, "min_mass" );
            double m2_max_km2 = get_yaml<double>( km2_node, "max_mass" );

            double km2_halo_integral =  integral( *hsummed_halo, m2_min_km2, m2_max_km2 );
            double km2_data_integral =  integral( *hsummed_data, m2_min_km2, m2_max_km2 );
            double km2_km2_integral =  integral( *hsummed_km2, m2_min_km2, m2_max_km2 );

            double subtracted_km2 = km2_data_integral -  halo_scale_ * km2_halo_integral ;
            double subtracted_km2_error = std::sqrt( km2_data_integral + km2_halo_integral );

            km2_scale_ =  subtracted_km2  /  km2_km2_integral;
            km2_scale_error_ = km2_scale_ *
                ( subtracted_km2_error / subtracted_km2 + 1 / sqrt( km2_km2_integral ) );

            //Record km2 fiducial weight and flux
            km2_fid_weight_ = 0;
            for ( auto km2_channel : km2_channels )
            {
                km2_fid_weight_ += fiducial_weights_.at( km2_channel );
            }

            km2_br_ = brs_.at( "km2" );

            std::cerr << "Estimated kaon flux: " << km2_scale_ * km2_fid_weight_ / km2_br_ << std::endl;
        }
    }

    void Km2Scaling::scale_hist( TH1& h, const YAML::Node& instruct ) const
    {
        double scale_factor = 1;
        std::string type = get_yaml<std::string>( instruct, "type" );
        if ( type == "halo" )
        {
            scale_factor = halo_scale_;
        }
        else if ( type == "data" )
        {
            scale_factor = 1; 
        }
        else
        {
            double fid_weight = fiducial_weights_.at( get_yaml<std::string>( instruct, "fid_weight" ) );
            double br = brs_.at( type );
            scale_factor = km2_scale_ *  km2_fid_weight_  / fid_weight * br / km2_br_;

            std::cerr << "Scaling " << type << " km2fid: " <<  km2_fid_weight_ 
                << " km2_scale:" <<  km2_scale_  << " fid:" <<  fid_weight 
                << " br:" <<  br << " km2br:" <<  km2_br_ << "  scale_factor: " << scale_factor << std::endl;
        }
        h.Scale( scale_factor );
    }
}
