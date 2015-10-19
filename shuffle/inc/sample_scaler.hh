#ifndef SAMPLE_SCALER_HH
#define SAMPLE_SCALER_HH
#include "MultiScaling.hh"
#include "FlexiScaling.hh"
#if 0
/*
 *                            _                        _
 *  ___  __ _ _ __ ___  _ __ | | ___     ___  ___ __ _| | ___ _ __
 * / __|/ _` | '_ ` _ \| '_ \| |/ _ \   / __|/ __/ _` | |/ _ \ '__|
 * \__ \ (_| | | | | | | |_) | |  __/   \__ \ (_| (_| | |  __/ |
 * |___/\__,_|_| |_| |_| .__/|_|\___|___|___/\___\__,_|_|\___|_|
 *                     |_|         |_____|
 *
 */
#endif
namespace fn
{

std::unique_ptr<SampleScaler> get_sample_scaler(
     const YAML::Node& method_node,
     const std::map<std::string, double>& fiducial_weights,
     const std::map<std::string, double>& branching_ratios );


}
#endif
