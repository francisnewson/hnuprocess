#include "RatioCanvas.hh"
#include "TH1D.h"
namespace fn
{
    RatioCanvas::RatioCanvas ( TH1& bg, TH1& dat, TH1& rat)
        :result_canvas_("c", "c", 400, 500 )
    {
        result_canvas_.cd();

        pad_upper_.reset( new TPad( "pad_upper", "pad_upper", 0, 0.3, 1, 1.0 ) );
        pad_upper_->SetBottomMargin( 0 );
        pad_upper_->Draw();

        pad_upper_->cd();
        bg.Draw();
        dat.Draw("same");

        result_canvas_.cd();

        pad_lower_.reset( new TPad( "pad_lower", "pad_lower", 0, 0.05, 1, 0.3 ) );
        pad_lower_->SetTopMargin( 0 );
        pad_lower_->Draw();

        pad_lower_->cd();
        rat.Draw("ep");

    }

    void RatioCanvas::Write( std::string name )
    {
        result_canvas_.Write( name.c_str() );
    }
}
