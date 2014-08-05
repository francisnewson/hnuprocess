#include "RecoParser.hh"
#include <cassert>
#include "Xcept.hh"
namespace fn
{


    //Constructor takes a reference to a RecoFactory which
    //can actually produce subscribers etc
    RecoParser::RecoParser( RecoFactory& rf )
        :rf_ (rf ) {}

    //Process a YAML file
    void RecoParser::parse( boost::filesystem::path config )
    {
        //Load file
        YAML::Node config_node = YAML::LoadFile( config.string() );
        assert(config_node.Type() == YAML::NodeType::Sequence);

        //Loop over items
        for (YAML::const_iterator it=config_node.begin();
                it!=config_node.end();++it)
        {
            auto& instruct = * it;

            std::string name =
                instruct["name"].as<std::string>();

            std::string type =
                instruct["type"].as<std::string>();

            //All subscribers must be registered
            //to receive new_event's
            Subscriber * s = rf_.create_subscriber( 
                    name, type, instruct );

            //There are some special categories
            if ( YAML::Node cat = instruct["cat"] )
            {
                //Selections need to be found by others
                if ( cat.as<std::string>() == "selection" )
                {
                    Selection * sel = dynamic_cast<Selection*>(s);
                    if ( !sel )
                    {
                        throw Xcept<BadCast>
                        ( name + "is not a selection" );

                    }
                    rf_.add_selection(name, sel );
                }
            }
        }
    }
}
