#include "ToyMC.hh"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "HistStore.hh"

using namespace toymc;
using namespace fn;

int main( int argc , char * argv[] )
{
    SmartRNG<std::mt19937> gen;

    double rad_length = 19.32 ; //cm
    double block_length =  1.39 / 100.0; //cm

    ToyMCScatter single_block{ gen,  rad_length, block_length };

    int n_events = 100000;

    HistStore hs;
    auto h_tx = hs.MakeTH1D(  "h_tx", "tx after scattering", 1000, -0.100, 0.100, "tx" );

    for ( int i = 0 ; i != n_events ; ++ i )
    { 
        track_params initial_params{ 0.207, 0, 0, 0, 0, 0 };

        for ( int i = 0 ; i != 100 ; ++i )
        {
         initial_params =  single_block.transfer( initial_params );
        }
        h_tx->Fill( initial_params.tx );
    }

    TFile tfout{ "output/test_toy_mc.root", "RECREATE" };
    hs.Write();
}
