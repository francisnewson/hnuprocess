#include "easy_app.hh"
#include "TF1.h"

std::unique_ptr<TF1> quartic_scattering( std::string name, double peak )
{
    //Function tends to magnitude/x^2
    //Function peaks at peak

    std::unique_ptr<TF1> result{ new TF1
        ( name.c_str(), "1/x**4 * tanh( pow( x*[0], 6 ) )", -15*peak, 15*peak )  };


    //Magic number to get peak in the right place
    double xscale = 0.965;
    double p0 = xscale / peak;

    result->SetParameter( 0, p0 );
    return result;
}

std::unique_ptr<TF1> cubic_scattering( std::string name, double peak )
{
    //Function tends to magnitude/x^2
    //Function peaks at peak

    std::unique_ptr<TF1> result{ new TF1
        ( name.c_str(), "1/x**3 * tanh( pow( x*[0], 5 ) )", -15*peak, 15*peak )  };


    //Magic number to get peak in the right place
    double xscale = 0.983;
    double p0 = xscale / peak;

    result->SetParameter( 0, p0 );
    return result;
}


int main( int argc, char * argv[] )
{
    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "old,o", po::value<double>() , "Old cutoff")
        ( "new,n", po::value<double>() , "New cutoff")
        ( "power,p", po::value<int>() , "Power");

    po::options_description desc("Allowed options");
    desc.add( general );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    if ( vm.count( "help" ) )
    {
        std::cout << desc << std::endl;
        return EXIT_FAILURE;
    }

    if ( !vm.count( "old" ) )
    {
        std::cout << "Must specify old cuttoff" << std::endl;
        return EXIT_FAILURE;
    }

    if ( !vm.count( "new" ) )
    {
        std::cout << "Must specify new cuttoff" << std::endl;
        return EXIT_FAILURE;
    }

    if ( !vm.count( "power" ) )
    {
        std::cout << "Must specify power" << std::endl;
        return EXIT_FAILURE;
    }

    double old_c = vm["old"].as<double>();
    double new_c = vm["new"].as<double>();

    int power = vm["power"].as<int>();

    std::unique_ptr<TF1> old_f;
    std::unique_ptr<TF1> new_f;

    if( power ==4 )
    {
        old_f = quartic_scattering( "fold", old_c );
        new_f = quartic_scattering( "fnew", new_c );
    }
    else if ( power == 3 )
    {
        old_f = cubic_scattering( "fold", old_c );
        new_f = cubic_scattering( "fnew", new_c );
    }

    double int_old = old_f->Integral( 0, 15 * old_c );
    double int_new = new_f->Integral( 0, 15 * new_c );

    std::cout <<  int_new / int_old  << std::endl;

    return EXIT_SUCCESS;
}
