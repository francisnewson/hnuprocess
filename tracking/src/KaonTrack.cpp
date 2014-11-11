#include "KaonTrack.hh"
#include "yaml_help.hh"
#include "tracking_functions.hh"
#include "NA62Constants.hh"
#include "Xcept.hh"
#include <istream>
#include <iomanip>
#include <boost/filesystem.hpp>

namespace fn
{
    Track get_kp_track( const fne::Conditions c )
    {
        return Track { c.pkxoffp, c.pkyoffp, 0, 
            c.pkdxdzp, c.pkdydzp, 1 };
    }

    Track get_km_track( const fne::Conditions c )
    {
        return Track { c.pkxoffm, c.pkyoffm, 0, 
            c.pkdxdzm, c.pkdydzm, 1 };
    }
    //--------------------------------------------------

    YAML::Node auto_kaon_track( YAML::Node& instruct, RecoFactory& rf )
    {
        std::string channel = rf.get_channel();

        YAML::Node config_node =
            YAML::LoadFile( get_yaml<std::string>(instruct, "beam_mapping" ) ) ;

       //std::cerr << "Loading channel lists ... " << std::endl;
        const YAML::Node& channel_lists = config_node["channel_lists"];

       //std::cerr << "Loading property files ... " << std::endl;
        const YAML::Node& property_files = config_node["property_files"];

        assert( channel_lists.Type() == YAML::NodeType::Sequence );

        bool found_period = false;
        std::string period;

       //std::cerr << "Reading channel lists ... " << std::endl;
        //loop over periods
        for ( YAML::const_iterator it = channel_lists.begin();
                it != channel_lists.end() ; ++ it )
        {

            //get list of channels for this period
            const YAML::Node& channel_list = *it;
            std::vector<std::string> channels = 
                channel_list["channels"].as<std::vector<std::string>>();

            //Is our channel in this list?
            auto found_channel = std::find( 
                    channels.begin(), channels.end(), channel );

            if ( found_channel != channels.end() )
            {
                found_period = true;
                period = channel_list["period"].as<std::string>();
                break;
            }
        }

        YAML::Node result;
        result["name"] = instruct["name"].as<std::string>();
        result["type"] = instruct["type"].as<std::string>();

        if ( found_period )
        {
            result["kaon_type"] = "weightedK";
            result["pos_pol_file"] = property_files[period]["pos"].as<std::string>();
            result["neg_pol_file"] = property_files[period]["neg"].as<std::string>();
        }
        else
        {
            result["kaon_type"] = "rawkp";
        }

        return result;
    }


    //--------------------------------------------------

    REG_DEF_SUB( KaonTrack );

    template<>
        Subscriber * create_subscriber<KaonTrack>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();
            bool is_mc = rf.is_mc();

            std::string kaon_type =  
                get_yaml<std::string>( instruct, "kaon_type" );

            logger & slg = rf.get_log();

            BOOST_LOG_SEV( slg, startup )
                << "Creating KaonTrack: " << kaon_type;

            if ( kaon_type == "auto")
            {
                YAML::Node computed_instruct = 
                    auto_kaon_track( instruct, rf );

                BOOST_LOG_SEV( rf.get_log(), startup )
                    << "KaonTrack auto ==> " << 
                    get_yaml<std::string>( computed_instruct, "kaon_type" );

                return create_subscriber<KaonTrack>
                    ( computed_instruct , rf );
            }


            if ( kaon_type == "rawkp" )
            {
                return new RawKPTrack( event, is_mc );
            }
            else if ( kaon_type == "rawkm" )
            {
                return new RawKMTrack( event, is_mc );
            }
            else if ( kaon_type == "weightedK" )
            {
                return new WeightedKTrack( event, is_mc, instruct );
            }
            else
            {
                throw Xcept<UnknownKaonType>( kaon_type );
            }
        }

    KaonTrack * get_kaon_track( YAML::Node& instruct, RecoFactory& rf )
    {
        KaonTrack * kt = 0;

        try
        {
            YAML::Node ykt = instruct["inputs"]["kt"];
            if ( !ykt )
            { throw Xcept<MissingNode>( "kt" ); }

            kt = dynamic_cast<KaonTrack*>
                ( rf.get_subscriber( ykt.as<std::string>() ) );

            if ( !ykt )
            {throw Xcept<BadCast>( "KT" ); }
        }
        catch( ... )
        {
            std::cerr << "Trying to get kaon track (" __FILE__")\n";
            throw;
        }

        return kt;
    }

    //--------------------------------------------------

    void KaonTrack::new_event() {}

    TVector3 KaonTrack::get_kaon_point() const
    {
        return get_kaon_track().get_point();
    }

    TVector3 KaonTrack::get_kaon_3mom() const
    {
        return get_kaon_mom() * (  get_kaon_track().get_direction().Unit() );
    }

    TVector3 KaonTrack::extrapolate_z( double z ) const
    {
        return get_kaon_track().extrapolate( z );
    }

    TLorentzVector KaonTrack::get_kaon_4mom() const
    {
        return { get_kaon_3mom(),
            std::hypot( get_kaon_mom(), na62const::mK) };
    }

    //--------------------------------------------------

    void CachedKaonTrack::new_event()
    {
        kt_ = load_kaon_track();
        kmom_ = load_kaon_mom();
    }

    const Track& CachedKaonTrack::get_kaon_track() const
    {
        return kt_;
    }

    double CachedKaonTrack::get_kaon_mom() const
    {
        return kmom_;
    }

    //--------------------------------------------------

    RawKPTrack::RawKPTrack ( const fne::Event * e , bool mc )
        :e_(e), mc_( mc ){}

    Track RawKPTrack::load_kaon_track() const
    {
        return get_kp_track( e_->conditions );
    }

    double RawKPTrack::load_kaon_mom() const
    {
        return e_->conditions.pkp;
    }

    //--------------------------------------------------

    RawKMTrack::RawKMTrack ( const fne::Event * e , bool mc )
        :e_(e), mc_( mc ){}

    Track RawKMTrack::load_kaon_track() const
    {
        return get_km_track( e_->conditions );
    }

    double RawKMTrack::load_kaon_mom() const
    {
        return e_->conditions.pkm;
    }

    //--------------------------------------------------

    std::istream& operator >> ( std::istream& is , kaon_properties& kp )
    {
        return is >> kp.dxdz >> kp.dydz >> kp.xoff >> kp.yoff >> kp.pmag ;
    }

    std::ostream& operator << ( std::ostream& os , const kaon_properties& kp )
    {
        return os
            << std::setw(10) << kp.dxdz << std::setw(10) << kp.dydz 
            << std::setw(10) << kp.xoff << std::setw(10) << kp.yoff
            << std::setw(10) << kp.pmag ;
    }

    WeightedKTrack::WeightedKTrack ( const fne::Event * e, bool mc, 
            const YAML::Node& instruct )
        :e_( e  ), mc_( mc ), pf_( "input/reco/conditions/run_polarity.dat")
    {

        //Load positive polarity data
        auto pos_pol_file = get_yaml<std::string>( instruct, "pos_pol_file");
        if ( ! boost::filesystem::exists( pos_pol_file ) )
        {
            throw Xcept<MissingFile>( pos_pol_file );
        }
        std::ifstream ifpos( pos_pol_file );
        pos_pol_kaon_sampler_.set_max_key( 10000 );
        pos_pol_kaon_sampler_.load_new_data( ifpos );

        //Load negative polarity data
        auto neg_pol_file = get_yaml<std::string>( instruct, "neg_pol_file");
        if ( ! boost::filesystem::exists( neg_pol_file ) )
        {
            throw Xcept<MissingFile>( neg_pol_file );
        }
        std::ifstream ifneg( neg_pol_file );
        neg_pol_kaon_sampler_.set_max_key( 10000 );
        neg_pol_kaon_sampler_.load_new_data( ifneg );
    }

    void WeightedKTrack::new_event() 
    {
        int polarity = pf_.get_polarity( e_->header.run );
        int sample_key = e_->header.time_stamp % 10000;

        const kaon_properties& kp =  ( polarity > 0 ) 
            ? pos_pol_kaon_sampler_.get_value( sample_key)
            : neg_pol_kaon_sampler_.get_value( sample_key );

        kt_ = Track( kp.xoff, kp.yoff, 0, 
                kp.dydz, kp.dydz, 1 );

        kmom_ = kp.pmag;
    }


    const Track& WeightedKTrack::get_kaon_track() const
    {
        return kt_;
    }

    double WeightedKTrack::get_kaon_mom() const
    {
        return kmom_;
    }

}
