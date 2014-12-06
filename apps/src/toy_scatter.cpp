#include "easy_app.hh"
#include "TF1.h"
#include "HistStore.hh"
#include "TRandom3.h"
#include "Counter.hh"
#include <memory>

std::unique_ptr<TF1> quadratic_scattering( std::string name, double magnitude, double peak )
{
    //Function tends to magnitude/x^2
    //Function peaks at peak

    std::unique_ptr<TF1> result{ new TF1
        ( name.c_str(), "[0]/x**2 * tanh( pow( x*[1], 4 ) )", -5*peak, 5*peak )  };


    //Magic number to get peak in the right place
    double xscale = 1.021;

    double p0 = magnitude;
    double p1 = xscale / peak;

    result->SetParameter( 0, p0 );
    result->SetParameter( 1, p1 );

    return result;
}

std::unique_ptr<TF1> quartic_scattering( std::string name, double magnitude, double peak )
{
    //Function tends to magnitude/x^2
    //Function peaks at peak

    std::unique_ptr<TF1> result{ new TF1
        ( name.c_str(), "[0]/x**4 * tanh( pow( x*[1], 6 ) )", -5*peak, 5*peak )  };


    //Magic number to get peak in the right place
    double xscale = 0.965;

    double p0 = magnitude;
    double p1 = xscale / peak;

    result->SetParameter( 0, p0 );
    result->SetParameter( 1, p1 );

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
        ( "rate,r", po::value<double>(), "Rate parameter");

    po::options_description desc("Allowed options");
    desc.add( general );

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    int n_events = 10000000;

    if ( vm.count( "nevents" ) )
    {
        n_events = vm["nevents"].as<int>();
    }

    double width_param = 0.006;
    if ( vm.count("width") )
    {
        width_param = vm["width"].as<double>();
    }

    double rate = 0.001;
    if ( vm.count("rate") )
    {
        rate = vm["rate"].as<double>();
    }

    HistStore hs;

    std::cout << "Hello, world!" << std::endl;


    double sigma = 1.5e-4;
    TF1 fgaus("fgaus", "[0]*exp(-0.5*((x-[1])/[2])**2)", -0.005, 0.005 );
    fgaus.SetNpx( 2000 );
    fgaus.SetParameters( 1, 0, sigma );

    //TF1 ftail("ftail", "tanh( pow(x/[0], 6 ) ) * pow(x,-4) ", -0.005, 0.005 );
    TF1 ftail("ftail", "tanh( pow(x/[0], 4 ) ) * pow(x,-2) ", -0.005, 0.005 );
    ftail.SetNpx( 2000 );
    ftail.SetParameter( 0, 3.85*sigma );

    std::vector<double> rates;
    std::vector<TH1D*> hcombos;
    std::vector<TH1D*> htails;

    for ( int i = 0; i != 10; ++i )
    {
        double ratio = 0.2 * i * 0.009;
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

#if 0
    auto htail = hs.MakeTH1D( "htail", "Tail", 1000, -0.01, 0.01, "x" );
    htail->SetLineColor( kGreen+2 );
    auto hcombo = hs.MakeTH1D( "hcombo", "Combo", 1000, -0.01, 0.01, "x" );
    hcombo->SetLineColor( kRed+2 );
#endif

    logger slg;
    Counter c(  slg, n_events );

    for ( unsigned int i = 0 ; i != n_events ; ++i )
    {
        c.new_event();
        //std::cout<< "Gaus" << std::endl;
        double orig = fgaus.GetRandom();
        hgaus->Fill( orig );

        for (  int i = 0 ; i != 10 ; ++i )
        {
            //std::cout<< "Shift" << std::endl;
            double shift = ftail.GetRandom();
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
