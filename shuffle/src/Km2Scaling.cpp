#include "Km2Scaling.hh"
#include "HistExtractor.hh"
#include "TFile.h"
#include "TFractionFitter.h"
#include "TKey.h"
#include "TClass.h"
#include <TROOT.h>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <iomanip>

using boost::filesystem::path;


namespace fn
{
    std::map<std::string,double> extract_fiducial_weights
        ( std::string filename, std::string pre, std::string post, std::string branch )
        {
            TFile tf( filename.c_str() );
            tf.cd( pre.c_str() );
            TDirectory * dir = tf.CurrentDirectory();
            TIter next_object( dir->GetListOfKeys() ) ;
            TKey *key;

            std::map<std::string, double > result;

            while ( ( key = (TKey*)next_object() ) )
            {
                TClass * cl = gROOT->GetClass( key->GetClassName() );
                if ( ! cl->InheritsFrom( "TDirectory" ) ) continue;

                TTree * tt = 0;
                TDirectory * folder = static_cast<TDirectory*>( key->ReadObj() );
                folder->GetObject( post.c_str(), tt );

                double total_weight = 0.0;

                if ( branch == "events" )
                { total_weight = sum_variable<Long64_t>( branch,  tt ); }
                else
                { total_weight = sum_variable<Double_t>( branch,  tt ); }

                result.insert( std::make_pair( std::string( folder->GetName() ), total_weight ) );
            }

            return result;
        }

    std::map<std::string,double> extract_root_fiducial_weights
        ( const YAML::Node& fid_conf  )
        {
            return extract_fiducial_weights( 
                    get_yaml<std::string>( fid_conf,"root_file"), get_yaml<std::string>( fid_conf,"pre")
                    , get_yaml<std::string>(fid_conf,"post"), get_yaml<std::string>( fid_conf, "branch" ) );
        }

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

        std::cout << "\n--Starting " << get_yaml<string>( scaling_config_, "name" ) << std::endl;

        string scaling_strategy =  get_yaml<string>( scaling_config_, "strategy" );
        if ( scaling_strategy == "m2" )
        {
            m2_scaling();
        }
        else if ( scaling_strategy == "regions" )
        {
            region_scaling();
        }

        //Record km2 fiducial weight and flux
        auto km2_channels = get_yaml<vector<string>>( scaling_config_, "km2_channels" );
        km2_fid_weight_ = 0;
        for ( auto km2_channel : km2_channels )
        {
            km2_fid_weight_ += fiducial_weights_.at( km2_channel );
        }

        km2_br_ = brs_.at( "km2" );


        //std::cerr << "Estimated kaon flux: " << km2_scale_ * km2_fid_weight_ / km2_br_ << std::endl;
    }

    void Km2Scaling::region_scaling()
    {
        using std::vector;
        using std::string;

        //Channel definitions
        auto halo_channels = get_yaml<vector<string>>( scaling_config_, "halo_channels" );
        auto data_channels = get_yaml<vector<string>>( scaling_config_, "data_channels" );
        auto km2_channels = get_yaml<vector<string>>( scaling_config_, "km2_channels" );

        //Plot path
        path input_file = get_yaml<std::string>( scaling_config_, "input_file" );
        path plot_path = get_yaml<std::string>( scaling_config_, "plot_path" );

        //regions
        const YAML::Node region_node = scaling_config_["region"];

        double region_min = region_node["min"].as<double>();
        double region_max = region_node["max"].as<double>();

        TFile tf( input_file.string().c_str() );
        RootTFileWrapper rtfw( tf );

        ChannelHistExtractor ce( rtfw );
        ce.set_post_path( plot_path);

        auto hsummed_halo = get_summed_histogram( 
                ce, begin( halo_channels ), end( halo_channels ) );

        auto hsummed_data = get_summed_histogram( 
                ce, begin( data_channels ), end( data_channels ) );

        auto hsummed_km2 = get_summed_histogram( 
                ce, begin( km2_channels ), end( km2_channels ) );

        TObjArray mc(2);
        mc.Add( hsummed_halo.get() );
        mc.Add( hsummed_km2.get() );

        TFractionFitter fit( hsummed_data.get() , &mc );
        fit.SetRangeX( hsummed_halo->FindBin( region_min), hsummed_halo->FindBin( region_max ) );
        fit.Fit();

        fit.GetResult( 0, halo_scale_, halo_scale_error_ );
        fit.GetResult( 1,  km2_scale_,  km2_scale_error_ );

        //absolute scale factor
        double data_integral = integral( *hsummed_data, region_min, region_max );
        double halo_integral = integral( *hsummed_halo,region_min, region_max );
        double km2_integral = integral( *hsummed_km2,region_min, region_max );
        double extra_scale = data_integral / ( halo_scale_ * halo_integral + km2_scale_* km2_integral );
        halo_scale_ *= extra_scale;
        km2_scale_ *= extra_scale;
        halo_scale_error_ *= extra_scale;
        km2_scale_error_ *= extra_scale;
    }

    void Km2Scaling::m2_scaling()
    {
        using std::vector;
        using std::string;

        std::cout << "   Calculating scaling: " << get_yaml<string>( scaling_config_, "name" ) << std::endl;

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

            std::cout << std::setw(20) << "Halo entries: " << hsummed_halo->GetEntries() << std::endl;

            //Do integrals
            double m2_min_halo = get_yaml<double>( halo_node, "min_mass" );
            double m2_max_halo = get_yaml<double>( halo_node, "max_mass" );

            double halo_halo_integral =  integral( *hsummed_halo, m2_min_halo, m2_max_halo );
            double halo_data_integral =  integral( *hsummed_data, m2_min_halo, m2_max_halo );

            //std::cout << "Halo integral " << halo_halo_integral << std::endl;
            //std::cout << "Data integral " << halo_data_integral << std::endl;

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

            std::cout << std::setw(25) << "Halo integral: " << km2_halo_integral << std::endl;
            std::cout << std::setw(25) << "Data integral: " << km2_data_integral << std::endl;
            std::cout << std::setw(25) << "Km2  integral: " << km2_km2_integral << std::endl;

            double subtracted_km2 = km2_data_integral -  halo_scale_ * km2_halo_integral ;
            double subtracted_km2_error = std::sqrt( km2_data_integral + km2_halo_integral );

            km2_scale_ =  subtracted_km2  /  km2_km2_integral;
            km2_scale_error_ = km2_scale_ *
                ( subtracted_km2_error / subtracted_km2 + 1 / sqrt( km2_km2_integral ) );

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

#if 0
            std::cerr << "Scaling " << type << "\n"
                << std::setw(15) <<  "km2fid: " <<  km2_fid_weight_ 
                << std::setw(15)  << " km2_scale: " <<  km2_scale_  
                << std::setw(15) << " fid: " <<  fid_weight  << "\n"
                << std::setw(15) << " br: " <<  br 
                << std::setw(15) << " km2br: " <<  km2_br_ 
                << std::setw(15) << "  scale_factor: " << scale_factor << std::endl;
#endif
        }
        h.Scale( scale_factor );
    }
}
