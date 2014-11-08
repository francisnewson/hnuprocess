#include "easy_app.hh"
#include <iostream>
#include <iomanip>
#include <iterator>
#include "EChain.hh"
#include "Event.hh"
#include "logging.hh"

void print_info( path filename, std::ostream& os )
{
    logger slg;
    EChain<fne::Event> echain( slg, {filename}, "T", "event" );
    const fne::Event & e = * echain.get_event_pointer();

    echain.load_next_event_header();
    auto start_run = e.header.run;
    auto start_burst = e.header.burst_time;

    echain.set_next_event( echain.get_max_event() - 1 );
    echain.load_next_event_header();
    auto end_run = e.header.run;
    auto end_burst = e.header.burst_time;

    os << filename.string() 
        << std::setw(10) << start_run
        << std::setw(20) << start_burst
        << std::setw(10) << end_run
        << std::setw(20) << end_burst
        << std::setw(20) << echain.get_max_event() 
        << std::endl;
}

int main( int argc, char * argv[] )
{
    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "list,l", po::value<path>(),  "File list")
        ( "output,o", po::value<path>(),  "Ouput file")
        ;

    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, general), vm );

    boost::filesystem::ifstream ifs( vm["list"].as<path>() );

    boost::filesystem::ofstream ofs( vm["output"].as<path>() );

    for ( auto ifile = std::istream_iterator<path>( ifs ) ; 
            ifile != std::istream_iterator<path>(); ++ifile )
    {
        print_info( *ifile, ofs);
    }
}
