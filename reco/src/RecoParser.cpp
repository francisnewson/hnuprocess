#include "RecoParser.hh"
#include <cassert>
#include "Xcept.hh"
#include "stl_help.hh"
#include "yaml_help.hh"
#include <boost/regex.hpp>
#include <iomanip>
namespace fn
{
    //Constructor takes a reference to a RecoFactory which
    //can actually produce subscribers etc
    RecoParser::RecoParser( RecoFactory& rf, logger& log )
        :rf_ (rf ) , log_( log ){

            boost::filesystem::path channel_classes_file
                = "input/reco/channel_classes.yaml";

            BOOST_LOG_SEV( log_, startup)
                << "RECOPARSER: Loading  channel classes from "
                << channel_classes_file;

            channel_classes_ = YAML::LoadFile( channel_classes_file.string() ).as
                <std::map<std::string, std::vector<std::string>>>();
        }

    //Process a YAML file
    void RecoParser::parse( boost::filesystem::path config )
    {
        BOOST_LOG_SEV( log_, startup)
            << "RECOPARSER: Parsing " << config;

        //Load file
        YAML::Node config_node = YAML::LoadFile( config.string() );
        assert(config_node.Type() == YAML::NodeType::Sequence);

        //Loop over items
        for (YAML::const_iterator it=config_node.begin();
                it!=config_node.end();++it)
        {
            try
            {
                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: new subscriber";
                ;

                assert(it->Type() == YAML::NodeType::Map);

                YAML::const_iterator instruct_it  = it->begin();
                assert(it->Type() == YAML::NodeType::Map);

                const YAML::Node& instruct = instruct_it->second;
                std::string cat = instruct_it->first.as<std::string>();

                if ( cat == "exec" )
                {
                    BOOST_LOG_SEV( log_, startup )
                        << "RECOPARSER: Skipping exec item";
                    continue;
                }

                if( cat == "output" )
                {
                    parse_output( instruct );
                    continue;
                }

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: reading name  ...";
                std::string name;
                name = get_yaml<std::string>( instruct, "name" );

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: reading type  ...";
                std::string type;
                type = get_yaml<std::string>( instruct, "type" );

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: constructing subscriber  ...";
                //All subscribers must be registered
                //to receive new_event's

                Subscriber * s = 0;

                //Do we need to ignore subscriber?
                bool ignore_subscriber = 
                    check_ignore_subscriber( instruct, rf_.get_channel());

                if ( ignore_subscriber )
                {
                    BOOST_LOG_SEV( log_, startup)
                        << "RECOPARSER: skipping " << name << " (" << type << ")";

                    YAML::Node skip = YAML::Load( "{name: " + name + ", type: auto_pass}" );
                    s = rf_.create_subscriber( name, "auto_pass", skip );
                }
                else
                {
                    s = rf_.create_subscriber( 
                            name, type, instruct );
                }

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: Subscriber - " << name;

                BOOST_LOG_SEV( log_, debug)
                    << "\n" << instruct ;

                //Selections need to be found by others
                if ( cat == "selection" )
                {
                    BOOST_LOG_SEV( log_, debug)
                        << "RECOPARSER: Selection - " << name;

                    Selection * sel = dynamic_cast<Selection*>(s);
                    if ( !sel )
                    {
                        throw Xcept<BadCast>
                            ( name + "is not a selection" );
                    }

                    rf_.add_selection(name, sel );
                }
            }

            catch( YAML::Exception& e )
            {
                std::cerr << "YAML: " << e.what() << std::endl;
                std::cerr << "Trying to parse: " << *it <<  "\n";
                throw;
            }
            catch( std::runtime_error& e )
            {
                std::cerr << "RUNTIME: " << e.what() << std::endl;
                std::cerr << "Trying to parse: " << *it << "\n";
                throw ;
            }
        }
    }

    void RecoParser::parse_output( const YAML::Node& instruct )
    {
        try
        {

            std::string name;
            std::string type;

            name = instruct["name"].as<std::string>();
            type = instruct["type"].as<std::string>();

            if ( type == "ostream" )
            {

                std::string filename = 
                    instruct["filename"].as<std::string>();

                rf_.define_ostream( name, filename );
            }
            else if ( type =="tfile")
            {
                std::string filename = 
                    instruct["filename"].as<std::string>();

                rf_.define_tfile( name, filename );
            }
            else
            {
                throw std::runtime_error(
                        "Unknown output type: " + type );
            }
        }
        catch( std::runtime_error& e )
        {
            std::cerr << "Trying to parse: " << instruct ;
            throw;
        }
    }

    bool RecoParser::check_ignore_subscriber
        ( const YAML::Node& instruct, std::string channel )
        {
            //Do we have an ignore_class node?
            if ( YAML::Node ignore_class_node = instruct["ignore_classes"] )
            {
                std::vector<std::string> ignore_classes = 
                    ignore_class_node.as<std::vector<std::string>>();

                for ( auto& ignore_class : ignore_classes )
                {
                    BOOST_LOG_SEV( log_, debug )
                        << "Checking ignore class: " << ignore_class << " for "  << channel;

                    std::vector<std::string> ignore_channels = channel_classes_[ignore_class];

                    if ( contains( ignore_channels , channel ) )
                    {
                        return true;
                    }
                }
            }

            return false;
        }

    //--------------------------------------------------

    ExecParser::ExecParser( logger& log )
        :log_( log )
    {}

    void ExecParser::parse( boost::filesystem::path config)
    {
        BOOST_LOG_SEV( log_, startup)
            << "EXECPARSER: Parsing " << config;

        //Load file
        YAML::Node config_node = YAML::LoadFile( config.string() );
        assert(config_node.Type() == YAML::NodeType::Sequence);

        bool found_exec = false;

        //Loop over items
        for (YAML::const_iterator it=config_node.begin();
                it!=config_node.end();++it)
        {
            assert(it->Type() == YAML::NodeType::Map);

            YAML::const_iterator instruct_it  = it->begin();
            assert(it->Type() == YAML::NodeType::Map);

            const YAML::Node& instruct = instruct_it->second;
            std::string cat = instruct_it->first.as<std::string>();

            if ( cat == "exec" )
            {
                BOOST_LOG_SEV( log_, startup )
                    << "EXECPARSER: Found exec item";
                found_exec = true;

                if ( instruct["skipmcruns"] )
                {
                BOOST_LOG_SEV( log_, startup )
                    << "EXECPARSER: Found skipmcruns";
                    mc_skip_ = instruct["skipmcruns"].as<std::vector<int>>();
                }

                if ( instruct["skipdatafiles"] )
                {
                BOOST_LOG_SEV( log_, startup )
                    << "EXECPARSER: Found skipdatafiles";
                    data_skip_ = instruct["skipdatafiles"].as<std::vector<std::string>>();
                }
                if ( instruct["skipdatalist"] )
                {
                BOOST_LOG_SEV( log_, startup )
                    << "EXECPARSER: Found skipdatalist";
                std::string data_list = instruct["skipdatalist"].as<std::string>();

                std::ifstream ifdl( data_list );
                std::copy( std::istream_iterator<std::string>( ifdl ),
                        std::istream_iterator<std::string>(),
                        std::back_inserter( data_skip_ ) );
                }
            }
        }


        if ( ! found_exec )
        {
            BOOST_LOG_SEV( log_, startup )
                << "EXECPARSER: Didn't find exec config";
        }
    }


    void ExecParser::prune_filelist( std::vector<boost::filesystem::path>& filenames )
    {
        filenames.erase( std::remove_if( filenames.begin(), filenames.end(),
                    [this]( boost::filesystem::path p ){ return reject(p);} ), 
            filenames.end() ) ;
    }

    bool ExecParser::reject( boost::filesystem::path filename )
    {
        //should we be matching filenames or run numbers
        std::string  fstem = filename.stem().string();

        if ( fstem.find( "goldcmp" ) != std::string::npos )
        {
            //data
            bool found =  ( std::find( data_skip_.begin(), data_skip_.end(), fstem) != data_skip_.end() );
            return found;
        }
        else
        {
            boost::regex run_regex("run([0-9]+)");
            boost::smatch match;
            bool found_match = boost::regex_search( fstem, match, run_regex );
            int run = std::stoi( std::string( match[1].first, match[1].second ) );
            bool found ( std::find( mc_skip_.begin(), mc_skip_.end(), run) != mc_skip_.end() );
            return found;
        }
    }
}
