#include "logging.hh"
namespace fn
{

    //function to generate map
    std::map<std::string, severity_level> generate_severity_map()
    {

        static std::map<std::string, severity_level> result;

        using std::make_pair;

        result.insert( make_pair( "per_event", per_event) );
        result.insert( make_pair( "debug", debug) );
        result.insert( make_pair( "startup", startup) );
        result.insert( make_pair( "always_print", always_print) );
        result.insert( make_pair( "maximum", maximum) );

        return result;
    }

    //function to hold static map
    std::map<std::string, severity_level>& global_severity_map()
    {
        static std::map<std::string, severity_level>
            sm_ = generate_severity_map();

        return sm_;
    }

}
