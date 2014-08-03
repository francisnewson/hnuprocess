#include <iostream>
#include "Event.hh"
#include "Reconstruction.hh"
#include "RecoFactory.hh"
#include "Selection.hh"
#include "logging.hh"
#include "Announcer.hh"
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

int main()
{
    std::cerr << "Hello world" << std::endl;

    logger slg;

    auto  core = boost::log::core::get();

    // Set a global filter so that only error messages are logged
    core->set_filter(boost::log::expressions::is_in_range(
                boost::log::expressions::attr< severity_level >("Severity"), 
                severity_level::always_print, severity_level::maximum) );

    std::vector<boost::filesystem::path> filenames
    { "/afs/cern.ch/user/f/fnewson/work/hnu/gopher/code/reader/output/compact.root" };

    Reconstruction reco(  slg );
    reco.set_filenames( filenames );
    reco.set_required_events( 10 );
    reco.load_chain();

    RecoFactory rf( reco );
    rf.set_log( slg );
    YAML::Node instruct;

    rf.create_subscriber( "announcer", "Announcer", instruct );
    rf.create_subscriber( "counter", "Counter", instruct );
    reco.status_report( std::cerr );

    int n = 0;

    while ( reco.auto_next_event() )
    {
        BOOST_LOG_SEV( slg, debug )
            << "Printing event " << n++;
    }
}
