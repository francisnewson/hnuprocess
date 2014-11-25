#include "TrackPlotter.hh"
#include <iostream>
#include <iomanip>

namespace fn
{
    TrackPlotter::TrackPlotter( std::string name1, std::string name2)
    {
        h_dp_ = hs_.MakeTH1D( "h_dp", "#Delta P", 
                1000, -10, 10, "#Delta P ( GeV )" );

        h_dpop2_ = hs_.MakeTH1D( "h_dpop2", "#Delta P", 
                1000, -1, 1, "#Delta P ( GeV )" );

        h_dtx_ = hs_.MakeTH1D( "h_tx", "#Delta tx", 
                1000, -0.02, 0.02, "#Delta P ( GeV )" );

        h_dty_ = hs_.MakeTH1D( "h_ty", "#Delta ty", 
                1000, -0.02, 0.02, "#Delta P ( GeV )" );
    }

    void TrackPlotter::Fill( TVector3 point_1, TVector3 mom_1, 
            TVector3 point_2, TVector3 mom_2, double weight )
    {
        double p1 = mom_1.Mag();
        double p2 = mom_2.Mag();

        double tx1 = mom_1.X() / mom_1.Z();
        double ty1 = mom_1.Y() / mom_1.Z();

        double tx2 = mom_2.X() / mom_2.Z();
        double ty2 = mom_2.Y() / mom_2.Z();

        h_dp_->Fill(p1 - p2, weight );
        h_dpop2_->Fill(( p1 - p2) / std::pow(p1, 2 ), weight );
        h_dtx_->Fill( tx1 - tx2,  weight );
        h_dty_->Fill( ty1 - ty2,  weight );

#if 0
        std::cerr  
            << std::setw(10) << p1 
            << std::setw(10) << p2 
            << std::setw(10) << tx1 
            << std::setw(10) << tx2 
            << std::setw(10) << ty1 
            << std::setw(10) << ty2 
            << std::endl;
#endif
    }

    void TrackPlotter::Write()
    {
        hs_.Write();
    }
}
