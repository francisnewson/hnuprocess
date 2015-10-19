#include "sample_scaler.hh"
#include "stl_help.hh"
namespace fn
{

    std::unique_ptr<SampleScaler> get_sample_scaler(
            const YAML::Node& method_node,
            const std::map<std::string, double>& fiducial_weights,
            const std::map<std::string, double>& branching_ratios )
    {
        std::string method_name = get_yaml<std::string>( method_node, "name" );
        std::string method_strategy  = get_yaml<std::string>( method_node, "strategy" );


        std::unique_ptr<SampleScaler> scaler ;

        if ( method_strategy == "flexi" ) // the new way (allows stacks)
        {
            scaler = make_unique<FlexiScaling>( method_node );
        }
        else if( method_strategy == "hardcode" )
        {
            scaler = make_unique<HardcodeScaling>( method_node );
        }
        else // the old way
        {
            scaler = make_unique<MultiScaling>
                (method_node, fiducial_weights, branching_ratios) ;
        }

        return scaler;
    }
}
