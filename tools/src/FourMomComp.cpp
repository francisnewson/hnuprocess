#include "FourMomComp.hh"

namespace fn
{

    FourMomComp::FourMomComp()
    {
        hdp_ = hs_.MakeTH1D( "hdp", ( name_ + " #Delta p").c_str() ,
                1000, -50, 50, "#Delta p", "#events" );

        hdtx_ = hs_.MakeTH1D( "hdtx",
        ( name_ + " #Delta #theta_x").c_str() ,
                1000, -0.05, 0.05, "#Delta #theta_x", "#events" );

        hdty_ = hs_.MakeTH1D( "hdty",
        ( name_ + " #Delta #theta_y").c_str() ,
                1000, -0.05, 0.05, "#Delta #theta_y", "#events" );
    }

    void FourMomComp::Fill( const TLorentzVector& a, const TLorentzVector& b,
    double wgt)
    {
        auto amom = a.Vect();
        auto bmom = b.Vect();

        auto amag = amom.Mag();
        auto bmag = bmom.Mag();

        hdp_->Fill( amag - bmag, wgt );
        hdtx_->Fill( amom.X()/amag -  bmom.X()/bmag, wgt );
        hdty_->Fill( amom.Y()/amag -  bmom.Y()/bmag, wgt );
    }

    void FourMomComp::Write()
    {
        hs_.Write();
    }
}
