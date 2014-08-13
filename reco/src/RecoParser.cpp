#include "RecoParser.hh"
#include <cassert>
#include "Xcept.hh"
namespace fn
{
    //Constructor takes a reference to a RecoFactory which
    //can actually produce subscribers etc
    RecoParser::RecoParser( RecoFactory& rf, logger& log )
        :rf_ (rf ) , log_( log ){}

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

                if( cat == "output" )
                {
                    parse_output( instruct );
                    continue;
                }

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: reading name  ...";
                std::string name;
                name = instruct["name"].as<std::string>();

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: reading type  ...";
                std::string type;
                type = instruct["type"].as<std::string>();

                BOOST_LOG_SEV( log_, debug)
                    << "RECOPARSER: constructing subscriber  ...";
                //All subscribers must be registered
                //to receive new_event's

                Subscriber * s = 0;
                s = rf_.create_subscriber( 
                        name, type, instruct );

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

            catch( std::runtime_error& e )
            {
                std::cerr << "Trying to parse: " << *it << "\n";
                throw ;
            }
            catch( YAML::Exception& e )
            {
                std::cerr << "Trying to parse: " << *it <<  "\n";
                throw;
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
}
