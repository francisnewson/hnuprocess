#include "easy_app.hh"
#include "KaonTrack.hh"
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

int main( int argc, char * argv[] )
{
    bfs::path runlist_dir("runlists/polroot/");

    YAML::Node config_node = YAML::Load(
            "{ name: kaon_track,"
            " type: KaonTrack,"
            " kaon_type: auto,"
            " beam_mapping: input/reco/beam/cross_talk_mapping.yaml }");

    bfs::directory_iterator end_iter;
    for( bfs::directory_iterator dir_iter(runlist_dir) ;
            dir_iter != end_iter; ++dir_iter)
    {
        auto file_path = dir_iter->path();
        if (is_regular_file( file_path ) )
        {
            auto channel = file_path.stem().string();
            std::cerr.setstate(std::ios_base::failbit); //hack http://stackoverflow.com/a/30185095/1527126
            auto result = auto_kaon_track( config_node, channel );
            std::cerr.clear();

            std::cout << std::setw(30) << channel 
                << std::setw(20) << result["kaon_type"].as<std::string>()  ;

            if ( result["kaon_type"].as<std::string>()  == "weightedK" )
            {
                std::cout << " " << result["pos_pol_file"].as<std::string>()
                    << " "  << result["neg_pol_file"].as<std::string>();
            } 

            std::cout << std::endl;

        }
    }
}
