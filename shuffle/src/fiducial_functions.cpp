#include "fiducial_functions.hh"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include <TROOT.h>
#include "stl_help.hh"
#include <cassert>
#include <iostream>
#include <iomanip>
namespace fn
{

    std::map<std::string,double> extract_fiducial_weights
        ( std::string filename, std::string pre, std::string post, std::string branch )
        {
            TFile tf( filename.c_str() );
            tf.cd( pre.c_str() );
            TDirectory * dir = tf.CurrentDirectory();
            if ( !dir )
            { throw std::runtime_error( "Can't find "  + pre + " in "  + filename ); }

            TIter next_object( dir->GetListOfKeys() ) ;
            TKey *key;

            std::map<std::string, double > result;

            while ( ( key = (TKey*)next_object() ) )
            {

                TClass * cl = gROOT->GetClass( key->GetClassName() );
                if ( ! cl->InheritsFrom( "TDirectory" ) ) continue;

                TTree * tt = 0;
                TDirectory * folder = static_cast<TDirectory*>( key->ReadObj() );
                folder->GetObject( post.c_str(), tt );
                if ( ! tt )
                {
                    throw std::runtime_error( "Can't find " + post );
                }


                double total_weight = 0.0;

                if ( branch == "events" )
                { total_weight = sum_variable<Long64_t>( branch,  tt ); }
                else
                { total_weight = sum_variable<Double_t>( branch,  tt ); }

                result.insert( std::make_pair( std::string( folder->GetName() ), total_weight ) );
            }

            return result;
        }

    std::map<std::string,double> extract_root_fiducial_weights
        ( const YAML::Node& fid_conf  )
        {
            return extract_fiducial_weights( 
                    get_yaml<std::string>( fid_conf,"root_file"), get_yaml<std::string>( fid_conf,"pre")
                    , get_yaml<std::string>(fid_conf,"post"), get_yaml<std::string>( fid_conf, "branch" ) );
        }

    std::map<std::string,double> extract_all_root_fiducial_weights
        ( const YAML::Node& fid_conf )
        {
            const YAML::Node& files = fid_conf["files"];

            std::map<std::string, double> result;

            for ( const auto& file_node : files )
            {
                std::map<std::string, double> this_file = extract_fiducial_weights( 
                        get_yaml<std::string>( file_node, "root_file"), get_yaml<std::string>( file_node, "pre" ),
                        get_yaml<std::string>( file_node, "post" ), get_yaml<std::string>( file_node, "branch" ) );

                for( auto& chan : this_file )
                {
                    std::string chan_name = chan.first;
                    auto existing = result.find( chan_name );
                    if ( existing == result.end() )
                    {
                        result.insert( chan );
                    }
                    else
                    {
                        double a = existing->second;
                        double b = chan.second;

                        if ( ( a!=0 && b !=0 ))
                        { 
                            if ( ( a - b ) / ( a + b ) > 0.001 )
                            {
                                std::cout << "MISMATCH: " << existing->first << ": " 
                                    << a << " vs " <<  b << std::endl;
                            }
                        }
                        else if ( a != b )
                        {
                            std::cout << "MISMATCH: " << existing->first << ": " 
                                << a << " vs " <<  b << std::endl;
                        }
                    }
                }
            }

            return result;
        }


    void print_fid_weights( const std::map<std::string, double>& fid_weights,
            std::ostream& os )
    {
        os << "\nFiducial weights" << std::endl;
        os << std::setw(27) << "Channel" << std::setw(20) << "Weight" << std::endl;
        for ( auto chan : fid_weights )
        {
            os << std::setw(27) << chan.first << std::setw(20) << chan.second << std::endl;
        }
        os << std::string( 50, '-' ) << "\n" ;
    }
}
