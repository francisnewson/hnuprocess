#include "easy_app.hh"
#include "Event.hh"
#include "root_help.hh"
#include "TTree.h"
#include <boost/filesystem.hpp>

std::string get_original_filename( path p )
{
    TFile tf(p.string().c_str() );
    auto tt = get_object<TTree>( tf, "T" );
    auto info =  static_cast<fne::FileInfo*> ( tt->GetUserInfo()->At(0) );
    return info->filename;
}

int main( int argc, char * argv[] )
{
    po::options_description desc("Options");

    path dir_path;
    desc.add_options()
        ("help,h", "produce help message")
        ("folder,f", po::value(&dir_path), "set folder path") 
        ;

    po::positional_options_description pod;
    pod.add( "folder", 1  );

    po::variables_map vm;
    try {
        po::store( po::command_line_parser( argc, argv).
                options( desc ).positional(pod).run(), vm);
        po::notify( vm );

        if ( vm.count("help") ) {
            std::cout << desc << "\n";
            std::cout << "boost version: " << BOOST_LIB_VERSION << std::endl;
            return EXIT_FAILURE;
        }

    } catch ( const boost::program_options::error& e ) {
        std::cerr << e.what() << std::endl;
    }

    if ( !exists( dir_path ) ) return false;
    boost::filesystem::directory_iterator end_it; // default construction yields past-the-end
    for ( boost::filesystem::directory_iterator path_it( dir_path ); path_it != end_it; ++path_it )
    {
        std::cout << *path_it << " " << get_original_filename( *path_it ) << std::endl;
    }


        std::cout << get_original_filename( "/disk/lustre/na62/fon/hnudata/gopher/scmproot/"
                "p5.km2.intp.v4.neg/run20439.pos.0.scmp.root" );
}
