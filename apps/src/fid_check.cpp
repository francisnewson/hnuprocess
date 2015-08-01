#include "easy_app.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "fiducial_functions.hh"
#include <set>
#include <sstream>


typedef std::map<std::string, double> fiducial_map;

struct fid_instruct
{
    std::string name;
    std::string filename;
    std::string pre;
    std::string post;
    std::string branch;
};

fid_instruct instruct_from_node( const YAML::Node& instruct )
{
    fid_instruct res;
    res.name =  get_yaml<std::string>( instruct, "name" );
    res.filename =  get_yaml<std::string>( instruct, "filename" );
    res.pre =  get_yaml<std::string>( instruct, "pre" );
    res.post =  get_yaml<std::string>( instruct, "post" );
    res.branch =  get_yaml<std::string>( instruct, "branch" );

    return res;
}

void process_class( const YAML::Node& node )
{
    std::string cat = get_yaml<std::string>(node , "cat" );

    //generate all maps
    std::map<std::string, fiducial_map> fid_maps;
    for( const auto& instruct : node["files"] )
    {

        fid_instruct fi = instruct_from_node( instruct );
        fid_maps.insert( std::make_pair( fi.name,
                    extract_fiducial_weights(  fi.filename, fi.pre, fi.post, fi.branch )  ) );
    }

    //extract all channels
    std::set<std::string> channels;
    for ( auto& fid_map_pair : fid_maps )
    {
        const fiducial_map & fm = fid_map_pair.second;
        for ( const auto& channel : fm )
        {
            std::string datakey = "data";
            if ( cat == "mc" && contains( channel.first , datakey) ) { continue; }
            if ( cat == "data" && !contains( channel.first , datakey ) ) { continue; }
            channels.insert( channel.first );
        }
    }

    std::ostream& os = std::cout;

    //print headings
    os << "   " << std::setw(40) << "-";
    for ( auto& fid_map : fid_maps ) { os << std::setw(30) << fid_map.first; } os << "\n";

    //loop through all channels
    for ( auto chan : channels )
    {
        std::stringstream ss;
        ss << std::setw(40) << chan;

        bool bad_flag = false;
        bool very_bad_flag = false;
        double val = -1.0;

        for ( auto& fid_map : fid_maps )
        {
            auto found = fid_map.second.find( chan );
            if ( found != fid_map.second.end() )
            {
                double this_val =  found->second;

                //check for match
                if ( val < 0 ) {  val = this_val; }
                if (  val != this_val ) {
                    bad_flag = true;
                    if ( fabs( val - this_val ) > 5 )
                    {
                        very_bad_flag = true;
                    }
                }

                ss << std::setw(30) << std::setprecision(8) <<  this_val;
            }
            else
            {
                ss <<  std::setw(30) << "-";
            }
        }

        if ( very_bad_flag ) { os << "\033[31;m***"; }
        else if ( bad_flag ) { os << "\033[33;m---"; }
        else { os <<  "   "; } 

        os << ss.str() ;

        if ( bad_flag || very_bad_flag ) { os << "\033[0;m"; }
        os << "\n";
    }

    os << "\n";

}

int main(int argc , char * argv[] )
{

    if ( argc != 2 )
    {
        std::cout << "Need yaml file!" << std::endl;
        return EXIT_FAILURE;
    }

    std::string yaml_file( argv[1] );

    YAML::Node config = YAML::LoadFile( yaml_file );

    for ( const auto& class_node : config )
    {
        process_class( class_node );
    }

    return EXIT_SUCCESS;
}
