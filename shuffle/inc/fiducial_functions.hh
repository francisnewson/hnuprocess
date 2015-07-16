#ifndef FIDUCIAL_FUNCTIONS_HH
#define FIDUCIAL_FUNCTIONS_HH
#include <map>
#include <string>
#include "TTree.h"
#include "yaml_help.hh"
#include <iosfwd>
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
    template <class T>
        bool tolerance( const T& a, const T& b, const T& tol )
        {
            if ( ( a!=0) && (b !=0 ))
            { 
                return ( fabs( a - b ) / ( fabs(a) + fabs(b) ) <= tol );
            }
            else return ( a == b );
        }

    //function to extract fiducial counts from MC
    std::map<std::string,double> extract_fiducial_weights
        ( std::string filename, std::string pre, std::string post, std::string branch );

    template <typename T>
        double sum_variable( std::string branch, TTree* tt )
        {
            Long64_t nEntries = tt->GetEntries();
            double total_weight = 0.0;
            T  weight = 0;
            Int_t check = tt->SetBranchAddress( branch.c_str(), &weight );
            if ( check != 0 )
            {
                throw std::runtime_error( 
                        std::string("https://root.cern.ch/root/html/"
                            "TTree.html#TTree:CheckBranchAddressType")
                        + std::to_string( check ) );

            }
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

    std::map<std::string,double> extract_all_root_fiducial_weights
        ( const YAML::Node& fid_conf );

    void print_fid_weights( const std::map<std::string, double>& fid_weights,
            std::ostream& os );

    bool check_fiducial_weights
        ( std::vector<std::map<std::string,std::string>> burst_trees );

}
#endif
