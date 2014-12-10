#include "easy_app.hh"
#include "TF1.h"
#include "HistStore.hh"
#include "TRandom3.h"
#include "Counter.hh"
#include <memory>

std::unique_ptr<TF1> quadratic_scattering( std::string name, double peak )
{
    //Function tends to magnitude/x^2
    //Function peaks at peak

    std::unique_ptr<TF1> result{ new TF1
        ( name.c_str(), "1/x**2 * tanh( pow( x*[0], 4 ) )", -5*peak, 5*peak )  };

    //Magic number to get peak in the right place
    double xscale = 1.021;
    double p0 = xscale / peak;

    result->SetParameter( 0, p0 );
    return result;
}

std::unique_ptr<TF1> quartic_scattering( std::string name, double peak )
{
    //Function tends to magnitude/x^2
    //Function peaks at peak

    std::unique_ptr<TF1> result{ new TF1
        ( name.c_str(), "1/x**4 * tanh( pow( x*[0], 6 ) )", -5*peak, 5*peak )  };


    //Magic number to get peak in the right place
    double xscale = 0.965;
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
        ( "nevents,n", po::value<int>(), "Number of events" )
        ( "width,w", po::value<double>(), "Width parameter")
        ( "tail,t", po::value<double>(), "Tail scale")
        ( "gaus,g", po::value<double>(), "Gaus scale")
        ( "sigma,s", po::value<double>(), "Gaussian width");

    po::options_description desc("Allowed options");
    desc.add( general );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    std::cout 
        << "--------------------------------------------------\n"
        << "-             TOY MC SCATTERING                  -\n"
        << "--------------------------------------------------\n"
        << std::endl;

    if ( vm.count( "help" ) )
    {
        std::cout << desc << std::endl;
        return(0);
    }

    int n_events = 10000000;
    if ( vm.count( "nevents" ) )
    {
        n_events = vm["nevents"].as<int>();
    }
    std::cout << "N events: " << n_events << std::endl;

    double sigma = 1.5e-4;
    if ( vm.count("sigma") )
    {
        sigma = vm["sigma"].as<double>();
    }
    std::cout << "Gaussian sigma: " << sigma << std::endl;


    double width_param = 4 * sigma;
    if ( vm.count("width") )
    {
        width_param = vm["width"].as<double>();
    }
    std::cout << "Width param: " << width_param << std::endl;

    double tail = 0.01;
    if ( vm.count("tail") )
    {
        tail = vm["tail"].as<double>();
    }
    std::cout << "Tail rate param: " << tail << std::endl;

    double gaus_scale = 1.00;
    if ( vm.count("gaus") )
    {
        gaus_scale = vm["gaus"].as<double>();
    }

    std::cout << "Gaus rate param: " << gaus_scale << std::endl;

    HistStore hs;

    //Gaussian
    TF1 fgaus("fgaus", "exp(-0.5*((x/[0])**2))", -10*sigma, 10*sigma );
    fgaus.SetNpx( 2000 );
    fgaus.SetParameter(  0, sigma );
    double g_integral = fgaus.Integral( 0, 10*sigma );
    std::cout << "Predicted gaussian integral: " << 0.5 * std::sqrt(2 * 3.1415) * sigma << std::endl;
    std::cout << "Computed gaussian integral: " << g_integral << std::endl;

    //Tail
    auto ftail = quadratic_scattering( "ftail", width_param );
    ftail->SetNpx( 2000 );
    double t_integral = ftail->Integral( 0, 10 * width_param );
    std::cout << "Computed tail integral: " << t_integral << std::endl;

    //rate
    double rate = (tail*t_integral) / (gaus_scale *g_integral);
    std::cout << "Scattering rate: " << rate << std::endl;

    std::vector<double> rates;
    std::vector<TH1D*> hcombos;
    std::vector<TH1D*> htails;

    for ( int i = 0; i != 10; ++i )
    {
        double ratio = 0.2 * i * rate;
        rates.push_back( ratio );
        hcombos.push_back( hs.MakeTH1D( Form( "hcombo_%d", i ), Form( "COMBO, %f", ratio ),
                    1000, -0.01, 0.01, "x" ) );

        htails.push_back( hs.MakeTH1D( Form( "htail_%d", i ), Form( "TAIL, %f", ratio ),
                    1000, -0.01, 0.01, "x" ) );

        htails.back()->SetLineColor( kGreen+2 );
        hcombos.back()->SetLineColor( kRed+2 );
    }

    auto hgaus = hs.MakeTH1D( "hgaus", "Gaussian", 1000, -0.01, 0.01, "x" );
    hgaus->SetLineColor( kBlue+2 );

    logger slg;
    Counter c(  slg, n_events );

    for ( unsigned int i = 0 ; i != n_events ; ++i )
    {
        c.new_event();
        double orig = fgaus.GetRandom();
        hgaus->Fill( orig );

        for (  int i = 0 ; i != 10 ; ++i )
        {
            //std::cout<< "Shift" << std::endl;
            double shift = ftail->GetRandom();
            htails[i]->Fill( shift );

            if ( gRandom->Uniform() < rates[i] )
            {
                //std::cout << "yes" << std::endl;
                hcombos[i]->Fill( orig + shift );
            }
            else
            {
                hcombos[i]->Fill( orig );
            }
        }
    }

    TFile fout( "output/toy_scatter.root", "RECREATE" );
    hs.Write();
}
