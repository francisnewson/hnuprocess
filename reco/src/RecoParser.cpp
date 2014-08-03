#include "RecoParser.hh"
#include <cassert>
namespace fn
{

    RecoParser::RecoParser( RecoFactory& rf )
        :rf_ (rf ) {}

    void RecoParser::parse( boost::filesystem::path config )
    {
        YAML::Node config_node = YAML::LoadFile( config.string() );
        assert(config_node.Type() == YAML::NodeType::Sequence);

        for (YAML::const_iterator it=config_node.begin();
                it!=config_node.end();++it)
        {
            auto& instruct = * it;

            rf_.create_subscriber( 
                    instruct["name"].as<std::string>(),
                    instruct["type"].as<std::string>(),
                    instruct );
        }
    }
}
