#include "SubscriberFactory.hh"
#include "easy_app.hh"


int main( int argc, char * argv[] )
{
    (void) argc;
    (void) argv;

    splash( "input/art/hnu_splash.txt", std::cerr );
    SubscriberFactory::print_subscriber_map( std::cout );
}
