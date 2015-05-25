#ifndef FIDUCIAL_FUNCTIONS_HH
#define FIDUCIAL_FUNCTIONS_HH
#include <map>
#include <string>
#include "TTree.h"
#include "yaml_help.hh"
#if 0
/*
 *   __ _     _            _       _      __                  _   _
 *  / _(_) __| |_   _  ___(_) __ _| |    / _|_   _ _ __   ___| |_(_) ___  _ __  ___
 * | |_| |/ _` | | | |/ __| |/ _` | |   | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| | (_| | |_| | (__| | (_| | |   |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_| |_|\__,_|\__,_|\___|_|\__,_|_|___|_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *                                 |_____|
 *
*/
#endif
namespace fn
{
    //function to extract fiducial counts from MC
    std::map<std::string,double> extract_fiducial_weights
        ( std::string filename, std::string pre, std::string post );

    template <typename T>
        double sum_variable( std::string branch, TTree* tt )
        {
            Long64_t nEntries = tt->GetEntries();
            double total_weight = 0.0;
            T  weight = 0;
            tt->SetBranchAddress( branch.c_str(), &weight );
            tt->SetBranchStatus( "*", 0 );
            tt->SetBranchStatus( branch.c_str(), 1 );
            for ( int i = 0 ; i != nEntries ; ++i )
            {
                tt->GetEntry(i);
                total_weight += double(weight);
            }
            return  total_weight;
        }

    std::map<std::string,double> extract_root_fiducial_weights
        ( const YAML::Node& fid_conf  );
}
#endif
