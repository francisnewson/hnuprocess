#include <iostream>
#include "easy_app.hh"
#include "Event.hh"
#include "Reconstruction.hh"
#include "RecoFactory.hh"
#include "RecoParser.hh"
#include "Selection.hh"
#include "logging.hh"
#include "BadBurst.hh"
#include "Announcer.hh"
#include "SingleTrack.hh"
#include "yaml-cpp/yaml.h"

using namespace fn;

#if 0
/*       _             
 *      | |            
 * _ __ | | __ _ _   _ 
 *| '_ \| |/ _` | | | |
 *| |_) | | (_| | |_| |
 *| .__/|_|\__,_|\__, |
 *| |             __/ |
 *|_|            |___/ 
 */
#endif

int main( int argc, char * argv[])
{
    //COMMANDLINE
    //Set up program options

    po::options_description general("general");
    general.add_options()
        ( "help,h", "Display this help message")
        ( "log-level", po::value<std::string>(), "Set debug level")
        ;

    po::options_description files("files");

    po::options_description data("data");
    data.add_options()
        ( "number,n", po::value<Long64_t>(), "Number of events")
        ;

    po::options_description desc("Allowed options");
    desc.add( general ).add( files ).add( data );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    if ( vm.count( "help" ) )
    {
        std::cerr << desc << std::endl;
        exit(0);
    }

    //required number of events
    Long64_t required_events  = 100;

    if ( vm.count( "number" ) )
    {
        required_events =  vm["number"].as<Long64_t>() ;
    }

    //LOGGING
    logger slg;
    auto  core = boost::log::core::get();

    severity_level min_sev = startup;

    if ( vm.count( "log-level" ) )
    {
        min_sev = global_severity_map().at(
                vm["log-level"].as<std::string>() );
    }


    // Set a global filter so that only error messages are logged
    core->set_filter(boost::log::expressions::is_in_range(
                boost::log::expressions::attr< severity_level >("Severity"), 
                min_sev,  severity_level::maximum) );

    //--------------------------------------------------

    //RECONSTRUCTION

    std::vector<boost::filesystem::path> filenames
    { "/afs/cern.ch/user/f/fnewson/work/hnu/gopher/code/reader/output/k2pig.root" };

    //Create Reconstruction
    Reconstruction reco(  slg );
    reco.set_filenames( filenames );
    reco.set_required_events( required_events );
    reco.load_chain();

    //Create RecoFactory
    RecoFactory reco_factory( reco );
    reco_factory.set_log( slg );

    //Create RecoParser
    RecoParser reco_parser( reco_factory, slg );
    reco_parser.parse( "input/reco/test/test.yaml" );

    Subscriber::set_log( slg );


    reco.status_report( std::cerr );

    int n = 0;

    while ( reco.auto_next_event() )
    {
        BOOST_LOG_SEV( slg, debug )
            << "Printing event " << n++;
    }

    reco.end_processing();

}
