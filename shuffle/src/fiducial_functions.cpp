#include "fiducial_functions.hh"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include <TROOT.h>
namespace fn
{

    std::map<std::string,double> extract_fiducial_weights
        ( std::string filename, std::string pre, std::string post, std::string branch )
        {
            TFile tf( filename.c_str() );
            tf.cd( pre.c_str() );
            TDirectory * dir = tf.CurrentDirectory();
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
}
