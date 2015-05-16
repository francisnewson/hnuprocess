#include "lkraccep_2007.hh"
#include "ClusterEnergyCorr.hh"
#include <random>
#include <iostream>
#include "TH2D.h"
#include "TFile.h"

using namespace fn;

int main()
{


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-200, 200);

    TFile tfout{"output/lkr.root", "RECREATE" };

    if (false )
    {
        for ( int run = 20410; run != 20531 ; ++run )
        {

            std::cerr << "Run: " << run << "\n";

            TH2D hxy{ Form("hxy_%d", run), Form( "Accepted hits run %d", run ), 400, -200, 200, 400, -200, 200 };

            for ( int hit = 0 ; hit != 10000000; ++hit )
            {
                double x =  dis( gen );
                double y =  dis( gen );

                if ( !LKr_acc( run, x, y, 8 ) )
                {
                    hxy.Fill(x,y);
                }
            }

            hxy.Write();
        }
    }

    std::uniform_real_distribution<> small_dis(0, 50);
    TH2D hhot{ "h_hot", "Hot cell locations",  400, 0, 50, 400, 0, 50 };

    for ( int hit = 0 ; hit != 10000000; ++hit )
    {
        double x =  small_dis( gen );
        double y =  small_dis( gen );
        std::pair<int,int> cpd_cell = get_cpd_cell_index( x, y );

        if ( cpd_cell.first == 134 && ( cpd_cell.second == 56 || cpd_cell.second == 57 ) )
        {
            hhot.Fill(x,y);
        }
    }

    hhot.Write();

}
