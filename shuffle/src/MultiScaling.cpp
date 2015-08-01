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
#include "stl_help.hh"
namespace fn
{
    std::ostream& operator<<( std::ostream& os , const scale_result& sr )
    {
        os << "halo scale: " << sr.halo_scale << " ± " << sr.halo_scale_error 
            << " peak scale: " << sr.peak_scale << " ± " << sr.peak_scale_error;
        return os;
    }

    void ScaleStrategy::update_scaling()
    { 
        sr = compute_scaling();
        //std::cout << sr << "\n" << std::endl;
    }

    M2ScaleStrategy::M2ScaleStrategy(
            const YAML::Node & channel_node,
            const YAML::Node & strat_node)
        :channel_node_( channel_node), strat_node_( strat_node )
    {
        using std::vector;
        using std::string;
        using boost::filesystem::path;

        //Channel definitions
        halo_channels = get_yaml<vector<string>>( channel_node_, "halo_channels" );
        data_channels = get_yaml<vector<string>>( channel_node_, "data_channels" );
        peak_channels  = get_yaml<vector<string>>( channel_node_, "peak_channels" );

        do_halo_ = halo_channels.size() > 0;

        do_peak_ = peak_channels.size() > 0;

        //Setup input file
        input_file = get_yaml<std::string>( strat_node_, "input_file" );

        if ( do_halo_ )
        {
            m2_min_halo = get_yaml<double>( strat_node_, "min_mass_halo" );
            m2_max_halo = get_yaml<double>( strat_node_, "max_mass_halo" );
        }

        if ( do_peak_ )
        {
            m2_min_peak = get_yaml<double>( strat_node_, "min_mass_peak" );
            m2_max_peak = get_yaml<double>( strat_node_, "max_mass_peak" );
        }

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

        auto hsummed_data = get_summed_histogram( 
                ce, begin( data_channels ), end( data_channels ) );

        auto hsummed_peak = get_summed_histogram( 
                ce, begin( peak_channels ), end( peak_channels ) );


        //Do halo division 
        double halo_scale = 1;
        double halo_scale_error = 0;
        double peak_halo_integral = 0;

#if 0
        std::cout 
            << "Halo [" << m2_min_halo << ", " << m2_max_halo << "] "
            << "Peak [" << m2_min_peak << ", " << m2_max_peak << "]\n" ;
#endif

        if ( do_halo_ )
        {
            auto hsummed_halo = get_summed_histogram( 
                    ce, begin( halo_channels ), end( halo_channels ) );

            double halo_halo_integral =  integral( *hsummed_halo, m2_min_halo, m2_max_halo );
            double halo_data_integral =  integral( *hsummed_data, m2_min_halo, m2_max_halo );


            if ( halo_halo_integral < 1 )
            {
                std::cout << "WARNING: Ignoring halo integral" << std::endl;
            }
            else
            {
                halo_scale = halo_data_integral / halo_halo_integral;

                halo_scale_error = halo_scale * 
                    ( 1 / std::sqrt( halo_halo_integral ) + 1 / std::sqrt( halo_data_integral ) );
            }
            peak_halo_integral =  integral( *hsummed_halo, m2_min_peak, m2_max_peak );

#if 0
            std::cout 
                << std::setw(15) << "halo_halo"
                << std::setw(15) << "halo_data\n";

            std::cout 
                << std::setw(15) << halo_halo_integral
                << std::setw(15) << halo_data_integral << "\n";
#endif 
        }

        double peak_scale = 1;
        double peak_scale_error = 0;

        if ( do_peak_ )
        {
            //Do km2 division
            double peak_data_integral =  integral( *hsummed_data, m2_min_peak, m2_max_peak );
            double peak_peak_integral =  integral( *hsummed_peak, m2_min_peak, m2_max_peak );

            double subtracted_peak = peak_data_integral -  halo_scale * peak_halo_integral ;
            double subtracted_peak_error = std::sqrt( peak_data_integral + peak_halo_integral );

#if 0
            std::cout 
                << std::setw(15) << "peak_halo"
                << std::setw(15) << "peak_data"
                << std::setw(15) << "peak_peak"
                << std::setw(15) << "sub_peak\n";

            std::cout 
                << std::setw(15) << peak_halo_integral
                << std::setw(15) << peak_data_integral
                << std::setw(15) << peak_peak_integral 
                << std::setw(15) << subtracted_peak << std::endl;
#endif

            peak_scale =  subtracted_peak  /  peak_peak_integral;
            peak_scale_error = peak_scale *
                ( subtracted_peak_error / subtracted_peak + 1 / sqrt( peak_peak_integral ) );
        }

        return scale_result{ halo_scale, halo_scale_error, peak_scale, peak_scale_error };
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

        peak_name_ = get_yaml<string>( scaling_config, "peakname" );

        const YAML::Node& channel_node_ = scaling_config_["channels"];

        std::string mc_strat_type = get_yaml<string>( scaling_config["mc"], "strategy" );
        std::string halo_strat_type = get_yaml<string>( scaling_config["halo"], "strategy" );

        if ( mc_strat_type == "dummy" )
        {
            mc_strat_.reset( new DummyScaleStrategy() );
        }
        else
        {
            mc_strat_.reset( new M2ScaleStrategy{
                    channel_node_, scaling_config["mc"] } );
        }

        if ( halo_strat_type == "dummy" )
        {
            halo_strat_.reset( new DummyScaleStrategy() );
        }
        else
        {
            halo_strat_.reset( new M2ScaleStrategy{
                    channel_node_, scaling_config["halo"] } );
        }

        //Record km2 fiducial weight and flux
        peak_channels_ = get_yaml<vector<string>>( channel_node_, "peak_channels" );
    }

    void MultiScaling::compute_scaling() 
    {
        //compute scale factors
        //std::cout << "Doing MC scaling\n";
        mc_strat_->update_scaling();


        //std::cout << "Doing halo scaling\n";
        halo_strat_->update_scaling();

        peak_fid_weight_ = 0;
        for ( auto peak_channel : peak_channels_ )
        {
            try
            {
                peak_fid_weight_ += fiducial_weights_.at( peak_channel );
            }
            catch( std::out_of_range& e )
            {
                std::cout << "Missing fiducial weight for : " + peak_channel << std::endl;
                throw;
            }
        }

        try
        {
            peak_br_ = brs_.at( peak_name_ );
        }
        catch( std::out_of_range& e )
        {
            std::cout << "Missing branching ratio for: " + peak_name_ << std::endl;
            throw;
        }
    }

    double MultiScaling::get_halo_scale() const
    {
        return halo_strat_->get_halo_scale();
    }
    double MultiScaling::get_halo_scale_error() const
    {
        return halo_strat_->get_halo_scale_error();
    }

    double MultiScaling::get_peak_scale() const
    {
        return mc_strat_->get_peak_scale();
    }

    double MultiScaling::get_peak_scale_error() const
    {
        return mc_strat_->get_peak_scale_error();
    }

    double MultiScaling::get_fiducial_flux() const
    {
        return mc_strat_->get_peak_scale() * peak_fid_weight_  / peak_br_;
    }

    double MultiScaling::get_fiducial_flux_error() const
    {
        return get_peak_scale_error() / get_peak_scale() * get_fiducial_flux();
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
            auto fid_weight_key =  get_yaml<std::string>( instruct, "fid_weight" );
            double fid_weight = at( fiducial_weights_, fid_weight_key, "Missing fid_weight: " + fid_weight_key );
            double br = brs_.at( type );
            scale_factor = get_fiducial_flux() * br  / fid_weight;
        }
        h.Scale( scale_factor );
    }
}
