#include "RecoFactory.hh"
#include "Reconstruction.hh"
#include "Xcept.hh"
#include "yaml_help.hh"
#include "GlobalStatus.hh"

namespace fn
{
    RecoFactory::RecoFactory( Reconstruction& reco )
        :reco_( reco), log_(0), next_sub_id_(0), global_status_(0){}

    //logging
    void RecoFactory::set_log( logger& log )
    { log_ = &log; }

    logger& RecoFactory::get_log() const
    { 
        if ( ! log_ )
        { 
            throw std::runtime_error
                ( "RecoFactory has no log" );
        }
        return * log_;
    }

    //--------------------------------------------------

    //reco interaction
    Long64_t RecoFactory::get_max_events()
    {
        return reco_.max_events();
    }

    const fne::Event * RecoFactory::get_event_ptr()
    {
        return reco_.get_event_ptr();
    }

    //--------------------------------------------------

    //SUBSCRIBER MANAGEMENT

    //create
    Subscriber * RecoFactory::create_subscriber( std::string name, 
            std::string subscriber_type, YAML::Node instruct )
    {
        auto it = subscribers_.find( name );
        if ( it != subscribers_.end() )
        {
            throw Xcept<DuplicateSubscriberName>( name );
        }

        Subscriber * s = 0;
        try
        {
            s = SubscriberFactory::get_subscriber
                ( subscriber_type , instruct, *this);
        }
        catch ( ... )
        {
            std::cerr << "Trying create " << name << "\n";
            throw;
        }

        s->set_name( name );
        s->set_id( next_sub_id_ ++ );
        reco_.add_event_subscriber( s );

        subscribers_.insert( std::make_pair( name, s ) );

        return s;
    }

    //get
    Subscriber * RecoFactory::get_subscriber ( std::string name )
    {
        auto it = subscribers_.find( name );
        if ( it == subscribers_.end() )
        {
            throw Xcept<UnknownSubscriberName>( name );
        }

        return it->second;
    }

    //--------------------------------------------------

    //SELECTION MANAGEMENT

    //add
    void RecoFactory::add_selection ( std::string name, Selection * s  )
    {
        auto it = selections_.find( name );
        if ( it != selections_.end() )
        {
            throw Xcept<DuplicateSelectionName>( name );
        }

        selections_.insert( std::make_pair( name, s ) );
    }

    //get
    Selection * RecoFactory::get_selection ( std::string name)
    {
        auto it = selections_.find( name );
        if ( it == selections_.end() )
        {
            throw Xcept<UnknownSelectionName>( name );
        }
        return it->second;
    }

    //--------------------------------------------------

    //STREAM MANAGEMENT

    //prefix
    void RecoFactory::set_output_prefix( boost::filesystem::path prefix )
    {
        output_prefix_ = prefix;
    }

    //define
    void RecoFactory::define_ostream( std::string name, std::string p )
    {
        //Pass on std streams as is
        if  ( ( p.find( "cerr" ) != std::string::npos ) ||
                ( p.find( "cout" ) != std::string::npos ) )
        {
            ostreams_.insert( 
                    make_pair( name,  &osl_.get_ostream( p ) ) );

        }
        else
        {
            std::string munged_path = output_prefix_.string() + p;

            BOOST_LOG_SEV( get_log(), startup )
                << name << " actually points to " << munged_path;

            //Add a prefix to physical files
            ostreams_.insert( 
                    make_pair( name,  &osl_.get_ostream(
                            munged_path ) ) );

            ostream_paths_.insert( make_pair( name, munged_path ) );
        }

    }

    //get
    std::ostream& RecoFactory::get_ostream (std::string name )
    {
        try 
        {
            return *ostreams_.at( name );
        }
        catch( std::out_of_range& e )
        {
            std::cerr << "Can't find " << name << std::endl;
            throw std::out_of_range
                (   std::string( e.what() ) + "name: " +  name );
        }
    }

    //--------------------------------------------------

    //TFILE MANAGEMENT

    //define
    void RecoFactory::define_tfile
        ( std::string name,  boost::filesystem::path p)
        {
            auto munged_path =  boost::filesystem::path
            { output_prefix_.string() + p.string() } ;

            tfiles_.insert( make_pair ( name,  &tfl_.get_tfile
                        ( munged_path  ) ) );

            tfile_paths_.insert( make_pair( name, munged_path.string() ) );

            BOOST_LOG_SEV( get_log(), debug )
                << name << " points to " << munged_path << std::endl;
        }

    //get
    TFile& RecoFactory::get_tfile (std::string name )
    {
        try
        {
            return *tfiles_.at( name );
        }
        catch( std::out_of_range& e )
        {
            std::cerr << "Can't find " << name << std::endl;
            throw std::out_of_range
                (   std::string( e.what() ) + "name: " +  name );
        }
    }

    void RecoFactory::print_outputs( std::ostream& os ) const
    {
        os << "OUTPUT STREAMS\n";
        for ( const auto& kv : ostream_paths_ )
        {
            os << std::setw(20 ) << kv.first << ": " << kv.second << "\n";
        }

        os << "TFILES\n";
        for ( const auto& kv : tfile_paths_ )
        {
            os << std::setw(20 ) << kv.first << ": " << kv.second << "\n";
        }
    }

    //--------------------------------------------------

    //CHANNEL MANAGEMENT
    void RecoFactory::set_channel( const std::string& channel)
    {
        channel_ = channel;
    }

    std::string RecoFactory::get_channel() const
    {
        return channel_;
    }

    bool RecoFactory::is_mc() const
    {
        return reco_.is_mc();
    }

    bool RecoFactory::is_halo() const
    {

        boost::filesystem::path channel_classes_file
            = "input/reco/channel_classes.yaml";

        const auto& channel_classes_ = YAML::LoadFile( channel_classes_file.string() ).as
            <std::map<std::string, std::vector<std::string>>>();

        const auto& halo_channels = channel_classes_.at("p6");

        bool result = false;

        for (auto chan : halo_channels )
        {
            if ( chan == channel_ )
            {
                result =  true;
            }
        }

        BOOST_LOG_SEV( get_log(), always_print )
            << "Checking if " << channel_ << " is halo: "
            << std::boolalpha << result;

        return result;
    }

    //--------------------------------------------------

    //STOPPAGE
    bool * RecoFactory::get_remote_stop() const
    {
        return remote_stop_;
    }

    void RecoFactory::set_remote_stop( bool * b )
    {
        remote_stop_ = b;
    }

    std::string get_folder( const YAML::Node& node, const RecoFactory& rf, std::string key )
    {
        std::string raw_folder = get_yaml<std::string>( node, key );
        std::string channel = rf.get_channel();
        return ( channel + "/" + raw_folder );
    }

    //GLOBALSTATUS
    void RecoFactory::set_global_status( const GlobalStatus * const gs )
    {
        global_status_ = gs;
    }

    const GlobalStatus * RecoFactory::get_global_status()
    {
        if ( global_status_ == 0 )
        {
            throw std::runtime_error(
                    "RecoFactory::Request for GlobalStatus but none set" );
        }

        return global_status_;
    }
}
