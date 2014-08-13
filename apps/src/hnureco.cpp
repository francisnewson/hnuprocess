#include "easy_app.hh"
#include "stl_help.hh"
#include "RecoFactory.hh"
#include "RecoParser.hh"
#include "Reconstruction.hh"
#include "logging.hh"
#include <boost/filesystem.hpp>
#include <vector>

/*
 *  _                                    
 * | |__  _ __  _   _ _ __ ___  ___ ___  
 * | '_ \| '_ \| | | | '__/ _ \/ __/ _ \
 * | | | | | | | |_| | | |  __/ (_| (_) |
 * |_| |_|_| |_|\__,_|_|  \___|\___\___/ 
 *                                       
 * 
 */

//A global bool which can be
//used to store stop signals
bool&  remote_stop()
{
    static bool rs{false};
    return rs;
}

//Handle signals by setting remote_stop
void sig_handler( int sig )
{ 
    if ( sig )
    remote_stop() = true;
}


void connect_signals()
{
    //Press ctrl-C to save and close
    signal (SIGXCPU, sig_handler);
    signal (SIGINT, sig_handler);
    signal (SIGTERM, sig_handler);
    signal (127, sig_handler);
}

int main( int argc, char * argv[] )
{
    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/hnu_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream flaunch( "launch.log" );
        write_launch ( argc, argv, flaunch );
    }

    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "channel,c", po::value<path>(),  "Define channel")
        ( "mission,m", po::value<path>(),  "Specify mission file")
        ( "auto,a", "Set automic variables from channel and mission")
        ( "log-level", po::value<std::string>(), "Set logging level" )
        ;

    po::options_description files("files");

    files.add_options()
        ( "input-file,i", po::value<path>(), "Specify input file")
        ( "input-filelist,l", po::value<path>(), "Specify list of input files")
        ( "output-file,o", po::value<path>(), "Specify global output file")
        ( "output-prefix,p", po::value<path>(), "Specify output prefix")
        ;

    po::options_description data("data");

    data.add_options()
        ( "number,n", po::value<Long64_t>(), "Number of events")
        ( "file-count,f", po::value<int>(), "Number of files")
        ( "start-run,r", po::value<int>(), "Specify start run")
        ( "end-run,s", po::value<int>(), "Specify end run")
        ( "start-burst,b", po::value<int>(), "Specify start burst")
        ( "end-burst,d", po::value<int>(), "Specify end burst")
        ;

    po::options_description desc("Allowed options");
    desc.add( general ).add( files ).add( data );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    //Process program options--------------------------------------------

    //help
    if ( vm.count( "help" ) )
    {
        std::cerr << desc << "\n";
        std::cerr << "Exiting because help was requested." << "\n";
        return false;
    }

    std::string channel;
    path mission;
    std::string mission_name;

    //Required options
    if ( !  vm.count( "channel" ) )
    {
        std::cerr << "**ERROR** Must specify channel!" << std::endl;
        return false;
    }
    else
    {
        auto channel_path = vm["channel"].as<path>();
        channel = channel_path.stem().string();
    }

    if ( ! vm.count( "mission" ) )
    {
        std::cerr << "**ERROR** Must specify mission file!" << std::endl;
        return false;
    }
    else
    {
        mission = vm["mission"].as<path>();
        mission_name = mission.stem().string();
    }

    /**************************************************
     * LOGGING
     ***************************************************/

    logger slg;
    auto  core = boost::log::core::get();

    severity_level min_sev = startup;

    if ( vm.count( "log-level" ) )
    {
        try
        {
            min_sev = global_severity_map().at(
                    vm["log-level"].as<std::string>() );
        }
        catch( std::out_of_range & e )
        {
            std::cerr << "Unknown log-level: " 
                << vm["log-level"].as<std::string>() << std::endl;
            throw e;
        }
    }

    // Set a global filter so that only error messages are logged
    core->set_filter(boost::log::expressions::is_in_range(
                boost::log::expressions::attr< severity_level >("Severity"), 
                min_sev,  severity_level::maximum) );

    //--------------------------------------------------

    //PREPARE INPUT
    //Create Reconstruction
    Reconstruction reco ( slg );
    reco.set_remote_stop(  remote_stop() );

    /**************************************************
     * Event count
     ***************************************************/

    //required number of events
    if ( vm.count( "number" ) )
    {
        auto number =  vm["number"].as<Long64_t>() ;
        reco.set_required_events( number ) ;
        BOOST_LOG_SEV( slg , startup) << number << " events requested.";
    }
    else
    {
        BOOST_LOG_SEV( slg , startup)
            << "Defaulting to processing all available events." ;
    }

    //--------------------------------------------------

    /**************************************************
     * Filenames
     ***************************************************/

    //load filenames
    std::vector<path> filenames{};
    if ( vm.count( "input-file" ) == 1  )
    {
        //read single filename
        filenames.push_back( vm[ "input-file" ].as<path>() );
    }
    else 
    {
        //look for filelist
        BOOST_LOG_SEV( slg , startup) << "Initialise filelist" ;
        path filelist{};

        BOOST_LOG_SEV( slg , startup) << "Populate filelist" ;

        if ( vm.count( "input-filelist" ) == 1 )
        {
            filelist = vm[ "input-filelist" ].as<path>() ;
        }
        else if ( vm.count( "auto" ) )

        {
            filelist = path{ "runlists"  } / vm [ "channel" ].as<std::string>();
        }
        else
        {
            //complain if not satisfied
            std::cerr << "**ERROR** Must specifiy one input-file."
                " or one input-filelist" << std::endl;

            std::cerr << files << std::endl;
            std::cerr << "Use -h for full help." << std::endl;
            exit( 0 );
        }
        BOOST_LOG_SEV( slg , startup) << "Open filelist";

        boost::filesystem::ifstream ifs{ filelist };
        if ( ! ifs )
        { throw std::runtime_error( "Can't open filelist " + filelist.string() ); }

        BOOST_LOG_SEV( slg , startup) << "Read filelist";
        //load list of files
        std::vector<std::string> temp_filenames
        { std::istream_iterator<std::string>( ifs ),
            std::istream_iterator<std::string>() };

        BOOST_LOG_SEV( slg , startup) << "Convert filelist";
        std::transform( temp_filenames.begin(), temp_filenames.end(),
                std::back_inserter( filenames) , []( std::string& s )
                { return path( s ); } );
    }

    BOOST_LOG_SEV( slg , startup) << "Trim filelist";
    if ( vm.count("file-count") == 1 )
    {
        int file_count = vm["file-count"].as<int>();
        filenames.erase( begin(filenames) + file_count, end( filenames ) );
    }

    BOOST_LOG_SEV( slg , startup) 
        << "Give filelist to reconstruction" ;

    reco.set_filenames( filenames );
    reco.load_chain();

    //Create RecoFactory
    RecoFactory reco_factory( reco );
    reco_factory.set_log( slg );
    reco_factory.set_remote_stop( &remote_stop() );

    //Prefix preparation
    if ( vm.count( "output-prefix" ) )
    {
        reco_factory.set_output_prefix
            ( vm["output-prefix"].as<path>() );
    }
    else if ( vm.count( "auto" ) )
    {
        reco_factory.set_output_prefix
            ( path{"output"} / ( channel + "_"
                                 + mission_name + "_" ) );
    }

    Subscriber::set_log( slg );

    //Create RecoParser
    RecoParser reco_parser( reco_factory, slg );
    BOOST_LOG_SEV( slg , startup) << "Parssing " << mission;
    reco_parser.parse( mission );


    reco.status_report( std::cerr );

    //C-C now activates remote stop
    connect_signals();

    while ( reco.auto_next_event() )
    {
    }

    reco.end_processing();
}
