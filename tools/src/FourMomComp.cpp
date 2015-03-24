#include "FourMomComp.hh"
#include <cmath>

namespace fn
{

    FourMomComp::FourMomComp()
    {
        hdp_ = hs_.MakeTH1D( "hdp", ( name_ + " #Delta p").c_str() ,
                1000, -50, 50, "#Delta p", "#events" );

        hdpop_ = hs_.MakeTH1D( "hdpop_", ( name_ + " #Delta p  / p").c_str() ,
                1000, -2, 2, "#Delta p", "#events" );

        hdpop2_ = hs_.MakeTH1D( "hdpop2_", ( name_ + " #Delta p  / p^2").c_str() ,
                1000, -0.2, 0.2, "#Delta p", "#events" );

        hdpvsp_ = hs_.MakeTH2D( "hdpvsp_", ( name_ + " #Delta p vs p").c_str(),
                100, 0, 100, "p",
                1000, -50, 50, "#Delta p" );

        hdpopvsp_ = hs_.MakeTH2D( "hdpopvsp_", ( name_ + " #Delta p/p vs p").c_str(),
                100, 0, 100, "p",
                1000, -5, 5, "#Delta p / p" );

        hdtx_ = hs_.MakeTH1D( "hdtx",
                ( name_ + " #Delta #theta_x").c_str() ,
                1000, -0.05, 0.05, "#Delta #theta_x", "#events" );

        hdty_ = hs_.MakeTH1D( "hdty",
                ( name_ + " #Delta #theta_y").c_str() ,
                1000, -0.05, 0.05, "#Delta #theta_y", "#events" );

        hdm2_ = hs_.MakeTH1D( "hdm2",
                ( name_ + " #Delta m^{2}_{miss}").c_str(),
                1000, -0.1, 0.1, " #Delta m^{2}_{miss}" );
    }

    void FourMomComp::FillM2( const TLorentzVector& a, 
            const TLorentzVector& b ,
            const TLorentzVector& ref, double wgt )
    {
        double m2_a = ( ref - a ).M2();
        double m2_b = ( ref - b ).M2();
        hdm2_->Fill( m2_a - m2_b, wgt );
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

        auto reference_mag = amag; //(previously (amag + bmag) / 2  )

        hdpop2_->Fill( ( amag - bmag ) / std::pow( reference_mag,  2 ), wgt );
        hdpop_->Fill( ( amag - bmag ) / reference_mag, wgt );

        hdpvsp_->Fill( reference_mag  , amag - bmag , wgt );
        hdpopvsp_->Fill( reference_mag , (amag - bmag) / reference_mag , wgt );
    }

    void FourMomComp::Write()
    {
        hs_.Write();
    }

    void FourMomComp::SetColor( Color_t color )
    {
        for ( auto& h : hs_ )
        {
            h->SetLineColor( color );
            h->SetLineWidth( 2 );
        }
    }
}
