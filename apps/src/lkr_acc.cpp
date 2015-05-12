#include "lkraccep_2007.hh"
#include <random>
#include <iostream>
#include "TH2D.h"
#include "TFile.h"

int main()
{

    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-200, 200);

    TFile tfout{"output/lkr.root", "RECREATE" };

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
