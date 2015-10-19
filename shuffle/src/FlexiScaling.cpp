#include "FlexiScaling.hh"
#include  "HistExtractor.hh"
#include  "root_help.hh"
#include  "yaml_help.hh"
#include "fiducial_functions.hh"
#include "TF1.h"
#include <cstddef>
#include <iomanip>

namespace YAML
{
    Node convert<fn::sumdef>::encode ( const fn::sumdef & sd )
    {
        Node node;
        node["filename" ] = sd.filename;
        node["pre"] = sd.pre;
        node["post"] = sd.post;
        node["min" ] = sd.min;
        node["max" ]  = sd.max;

        return node;
    }

    bool convert<fn::sumdef>::decode( const Node& node, fn::sumdef& sd )
    {
        try
        {
            sd.filename = fn::get_yaml<std::string>( node, "file" );
            sd.max = fn::get_yaml<double>( node, "max" );
            sd.min = fn::get_yaml<double>( node, "min" );
            sd.post = fn::get_yaml<std::string>( node, "post" );
            sd.pre = fn::get_yaml<std::string>( node, "pre" );
            return true;
        }
        catch( std::exception& e )
        {
            std::cerr 
                << "Problem trying to extract the following node as sumdef: \n"
                << node << std::endl;
            throw;
        }
    }
}

//--------------------------------------------------

namespace fn
{
    double get_chan_integral( std::string chan, const sumdef& sum )
    {
        auto tf = get_tfile( sum.filename );
        RootTFileWrapper rtfw{ *tf };
        ChannelHistExtractor ce{ rtfw };
        ce.set_post_path( sum.post );
        std::vector<std::string> c{ {chan }};
        auto hsum = get_summed_histogram( ce, c.begin(), c.end() );
        return integral( *hsum, sum.min, sum.max );
    }

    std::unique_ptr<TH1> get_chan_hist( std::string chan, const sumdef& sum )
    {
        auto tf = get_tfile( sum.filename );
        RootTFileWrapper rtfw{ *tf };
        ChannelHistExtractor ce{ rtfw };
        ce.set_post_path( sum.post );
        std::vector<std::string> c{ {chan }};
        auto result =  get_summed_histogram( ce, c.begin(), c.end() );
        std::cout << result->Integral() << std::endl;
        return result;
    }

    //--------------------------------------------------

    MonoHaloScale::MonoHaloScale( const YAML::Node instruct )
    {
        halo_chan_ = get_yaml<std::string>( instruct["halo"], "chan" );
        halo_sumdef_ = get_yaml<sumdef>( instruct["halo"], "sum" );

        data_chan_ = get_yaml<std::string>( instruct["data"], "chan" );
        data_sumdef_ = get_yaml<sumdef>( instruct["data"], "sum" );
    }

    void MonoHaloScale::compute_scaling()
    {
        bool print = false;

        double halo_integral = get_chan_integral( halo_chan_, halo_sumdef_ );
        double data_integral = get_chan_integral( data_chan_, data_sumdef_ );

        halo_scale_ = data_integral / halo_integral;
        halo_scale_error_ = halo_scale_ * 
            ( 1.0 / std::sqrt( halo_integral )  + 1.0 / std::sqrt( data_integral ) );

        if ( print )
        {
            std::cerr << "MonoHaloScale: " << data_integral << " / " << halo_integral
                << " = " << halo_scale_ << " ± " << halo_scale_error_ << std::endl;
        }
    }
    double  MonoHaloScale::get_halo_scale(){ return halo_scale_; }
    double  MonoHaloScale::get_halo_scale_error(){ return halo_scale_error_; }

    //--------------------------------------------------

    MonoHaloFit::MonoHaloFit( const YAML::Node instruct )
    {
        halo_chan_ = get_yaml<std::string>( instruct["halo"], "chan" );
        halo_sumdef_ = get_yaml<sumdef>( instruct["halo"], "sum" );

        data_chan_ = get_yaml<std::string>( instruct["data"], "chan" );
        data_sumdef_ = get_yaml<sumdef>( instruct["data"], "sum" );
    }

    void MonoHaloFit::compute_scaling()
    {
        bool print = false;
        auto h_halo= get_chan_hist( halo_chan_, halo_sumdef_ );
        auto h_data= get_chan_hist( data_chan_, data_sumdef_ );

        h_halo->Rebin(25);
        h_data->Rebin(25);


        double guess_ratio = 
            integral( *h_data, 0.05, 0.11 ) / integral( *h_halo, 0.05, 0.11 );

        std::cerr 
            << integral( *h_halo, 0.05, 0.11 ) << " " 
            << integral( *h_data, 0.05, 0.11 ) << std::endl;


        auto h_ratio = uclone( h_data );
        h_ratio->Sumw2();
        h_halo->Sumw2();
        h_ratio->Divide( h_halo.get() );

        h_ratio->Print();
        {
            TFile tfout( "output/test.root", "UPDATE" );
            h_ratio->Write( "fred" );
        }

        TF1 fit( "myfit", "pol0" , 0.05, 0.11);
        fit.SetParameter( 0, guess_ratio );
        h_ratio->Fit( "myfit" );
        fit.Print();
        halo_scale_ = fit.GetParameter(0);
        halo_scale_error_ = 0;
    }

    double  MonoHaloFit::get_halo_scale(){ return halo_scale_; }
    double  MonoHaloFit::get_halo_scale_error(){ return halo_scale_error_; }

    //--------------------------------------------------

    StackPeakFlux::StackPeakFlux( const YAML::Node& instruct, bool do_halo, FlexHaloScale * halo_scaler )
    {
        do_halo_ = do_halo;
        if ( do_halo_ )
        {
            halo_chan_ = get_yaml<std::string>( instruct["halo"], "chan" );
            halo_sumdef_ = get_yaml<sumdef>( instruct["halo"], "sum" );
            halo_scale_ = halo_scaler;
        }

        data_chan_ = get_yaml<std::string>( instruct["data"], "chan" );
        data_sumdef_ = get_yaml<sumdef>( instruct["data"], "sum" );

        stack_sumdef_ = get_yaml<sumdef>( instruct["stack"], "sum" );
        norm_chan_ = get_yaml<std::string>( instruct["stack"], "normchan" );

        const auto& channel_list_node = instruct["stack"]["channels"];
        for( auto& chan_node : channel_list_node )
        {
            stack_chans_.push_back( stack_chan{
                    get_yaml<std::string>( chan_node, "chan" ),
                    get_yaml<std::string>( chan_node, "type" ),
                    get_yaml<std::string>( chan_node, "fidname" ) } );
        }

        fid_weights_ = extract_root_fiducial_weights ( instruct["fids"] );

        brs_ = YAML::LoadFile( "input/shuffle/branching_ratios.yaml" )
            .as<std::map<std::string, double>>();
    }

    void StackPeakFlux::compute_scaling()
    {
        bool print = true;

        //do data integral
        double data_integral = get_chan_integral( data_chan_, data_sumdef_ );
        double data_integral_sqerr = data_integral;

        //normalize stack to norm_chan
        std::string local_norm_chan = norm_chan_; //http://stackoverflow.com/a/12944272/1527126
        auto norm_def = std::find_if( begin( stack_chans_ ), end( stack_chans_ ),
                [local_norm_chan]( const stack_chan& sc ){ return sc.chan == local_norm_chan; } );

        if ( norm_def  == end( stack_chans_ ) )
        { throw std::runtime_error( "normchan not in stack" ); }

        //double norm_fid = fid_weights_.at( norm_def->fidname );
        double norm_fid = at( fid_weights_, norm_def->fidname, "Missing fid_weight for " + norm_def->fidname );
        double norm_br = brs_.at( norm_def->type );

        //Sum up stack
        double stack_integral = 0 ;
        double stack_integral_sqerr = 0;

        for ( const auto & chan_def : stack_chans_ )
        {
            double chan_integral = get_chan_integral(chan_def.fidname, stack_sumdef_ );
            double chan_fid = fid_weights_.at( chan_def.fidname );
            double chan_br = brs_.at( chan_def.type );

            double scale = chan_br / norm_br  * norm_fid / chan_fid;

            stack_integral += chan_integral * scale;
            stack_integral_sqerr = chan_integral * fn::sq( scale );
        }

        if ( do_halo_ ) //Approach if we have to subtract halo
        {
            halo_scale_->compute_scaling();
            double raw_halo_integral = get_chan_integral( halo_chan_, halo_sumdef_ );
            double halo_integral = raw_halo_integral * halo_scale_->get_halo_scale();

            double halo_integral_sqerr =
                halo_integral * halo_scale_->get_halo_scale() / halo_scale_->get_halo_scale_error()
                + raw_halo_integral * halo_scale_->get_halo_scale();

            double sub = ( data_integral - halo_integral );
            double sub_sqerr = data_integral_sqerr + halo_integral_sqerr;

            if ( print )
            {
                std::cerr << "sub: " << data_integral <<  " - " << halo_integral 
                    << " = " << sub << std::endl;
            }

            double stack_scale = sub / stack_integral ;
            double stack_scale_error = stack_scale *
                ( std::sqrt( sub_sqerr) / sub  + std::sqrt( stack_integral_sqerr) / stack_integral );

            if ( print )
            {
                std::cerr << "stack_scale " << sub <<  " / " << stack_integral 
                    << " = " << stack_scale << std::endl;
            }

            k_flux_ = norm_fid * stack_scale / norm_br;
            k_flux_error_ = norm_fid * stack_scale_error / norm_br;
        }
        else //aproach if we don't
        {
            double stack_scale = ( data_integral  / stack_integral );
            double stack_scale_error = stack_scale * 
                ( std::sqrt( data_integral_sqerr ) / data_integral 
                  + std::sqrt( stack_integral_sqerr) / stack_integral );

            if ( print )
            {
                std::cerr << "stack_scale " << data_integral <<  " / " << stack_integral 
                    << " = " << stack_scale << std::endl;
            }

            k_flux_ = norm_fid * stack_scale / norm_br;
            k_flux_error_ = norm_fid * stack_scale_error / norm_br;
        }
    }

    double  StackPeakFlux::get_fiducial_flux(){ return k_flux_; }
    double  StackPeakFlux::get_fiducial_flux_error(){ return k_flux_error_ ; }

    //--------------------------------------------------

    ComboKaonFlux::ComboKaonFlux( const YAML::Node & instruct )
    {
        if ( instruct["halo"] )
        {
            auto halo_strat = get_yaml<std::string>( instruct["halo"], "strat" );
            if ( halo_strat == "mono" )
            {
                halo_scale_ = make_unique<MonoHaloScale>( instruct["halo"] );
            }
            else if ( halo_strat == "monofit" )
            {
                halo_scale_ = make_unique<MonoHaloFit>( instruct["halo"] );
            }
            else
            {
                throw std::runtime_error( "Unknown halo strategy: "  + halo_strat );
            }

            peak_flux_ = make_unique<StackPeakFlux>
                ( instruct["peak"], true, halo_scale_.get() );
        }
        else
        {
            peak_flux_ = make_unique<StackPeakFlux>
                ( instruct["peak"], false, nullptr);
        }
    }

    void ComboKaonFlux::compute_scaling()
    {
        halo_scale_->compute_scaling();
        peak_flux_->compute_scaling();
    }

    double  ComboKaonFlux::get_fiducial_flux()
    { return peak_flux_->get_fiducial_flux(); }

    double  ComboKaonFlux::get_fiducial_flux_error()
    { return peak_flux_->get_fiducial_flux_error(); }

    double  ComboKaonFlux::get_halo_scale()
    { return halo_scale_->get_halo_scale(); }

    double  ComboKaonFlux::get_halo_scale_error()
    { return halo_scale_->get_halo_scale_error(); }

    //--------------------------------------------------

    FlexiScaling::FlexiScaling( const YAML::Node& instruct )
    {
        auto halo_strat = get_yaml<std::string>( instruct["halo"], "strat" );
        if ( halo_strat == "mono" )
        {
            halo_scale_ = make_unique<MonoHaloScale>( instruct["halo"] );
        }
        else if ( halo_strat == "monofit" )
        {
            halo_scale_ = make_unique<MonoHaloFit>( instruct["halo"] );
        }
        else
        {
            throw std::runtime_error( "Unknown halo strategy: "  + halo_strat );
        }
        //halo_scale_ = make_unique<MonoHaloScale>( instruct["halo"] );
        kaon_flux_ = make_unique<ComboKaonFlux>( instruct["mc"] );

        fid_weights_ = extract_root_fiducial_weights ( instruct["fids"] );

        brs_ = YAML::LoadFile( "input/shuffle/branching_ratios.yaml" )
            .as<std::map<std::string, double>>();
    }

    void FlexiScaling::compute_scaling()
    {
        kaon_flux_->compute_scaling();
        halo_scale_->compute_scaling();
    }

    double FlexiScaling::get_halo_scale() const 
    { return halo_scale_->get_halo_scale(); }

    double FlexiScaling::get_halo_scale_error() const 
    { return halo_scale_->get_halo_scale_error(); }

    double FlexiScaling::get_fiducial_flux() const
    { return kaon_flux_->get_fiducial_flux(); }

    double FlexiScaling::get_fiducial_flux_error() const
    { return kaon_flux_->get_fiducial_flux_error(); }

    double FlexiScaling::scale_hist( TH1& h, const YAML::Node& instruct ) const
    {
        bool print = false;

        double scale_factor = 1.0;
        std::string type = get_yaml<std::string>( instruct, "type"  );

        if ( type == "halo" )
        {
            scale_factor = halo_scale_->get_halo_scale();
            if (print )
            {std::cout << std::setw(8) << type << " " << std::setw(12) << scale_factor << std::endl;}
        }
        else if ( type == "data" )
        {
            scale_factor = 1.0;
            if ( print )
            {std::cout << std::setw(8) << type << " " << std::setw(12) << scale_factor << std::endl;}
        }
        else
        {
            std::string fid_weight_key;
            try {
                fid_weight_key = get_yaml<std::string>( instruct, "fid_weight" );
            }
            catch( std::exception& e )
            {
                std::cerr << "Can't extract fid_weight_key from\n" << instruct << std::endl;
            }

            double fid_weight = at( fid_weights_, fid_weight_key, "Missing fid weight " + fid_weight_key );
            double br = brs_.at( type );
            scale_factor = get_fiducial_flux() * br / fid_weight;

            if ( print )
            {
                std::cout 
                    << std::setw(8) << type << " " 
                    << std::setw(12) <<  get_fiducial_flux()  << " * " 
                    << std::setw(10) << br << " / "
                    << std::setw(12) << fid_weight << " = " 
                    << std::setw(12) << scale_factor << std::endl;
            }
        }
        h.Scale( scale_factor );

        return scale_factor;
    }

    //--------------------------------------------------

    HardcodeScaling::HardcodeScaling( const YAML::Node& instruct )
    {
        fiducial_flux_ = get_yaml<double>( instruct, "fiducial_flux" );
        fiducial_flux_err_ = get_yaml<double>( instruct, "fiducial_flux_err" );

        halo_scale_ = get_yaml<double>( instruct, "halo_scale" );
        halo_scale_err_ = get_yaml<double>( instruct, "halo_scale_err" );

        fid_weights_ = extract_root_fiducial_weights ( instruct["fids"] );
        brs_ = YAML::LoadFile( "input/shuffle/branching_ratios.yaml" )
            .as<std::map<std::string, double>>();
    }

    double HardcodeScaling::scale_hist( TH1& h, const YAML::Node& instruct ) const 
    {
        double scale_factor = 1.0;
        std::string type = get_yaml<std::string>( instruct, "type"  );

        if ( type == "halo" )
        {
            scale_factor = halo_scale_; 
        }
        else if ( type == "data" )
        {
            scale_factor = 1.0;
        }
        else
        {
            std::string fid_weight_key;
            try {
                fid_weight_key = get_yaml<std::string>( instruct, "fid_weight" );
            }
            catch( std::exception& e )
            {
                std::cerr << "Can't extract fid_weight_key from\n" << instruct << std::endl;
            }

            double fid_weight = at( fid_weights_, fid_weight_key, "Missing fid weight " + fid_weight_key );
            double br = brs_.at( type );
            scale_factor = get_fiducial_flux() * br / fid_weight;
        }

        h.Scale( scale_factor );
        return scale_factor;
    }

    void HardcodeScaling::compute_scaling(){}

    double HardcodeScaling::get_halo_scale() const{ return halo_scale_; }
    double HardcodeScaling::get_halo_scale_error() const{ return halo_scale_err_; }

    double HardcodeScaling::get_fiducial_flux() const { return fiducial_flux_; }
    double HardcodeScaling::get_fiducial_flux_error() const { return fiducial_flux_err_; }
}
