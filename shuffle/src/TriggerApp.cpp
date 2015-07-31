#include "TriggerApp.hh"
#include <boost/filesystem/path.hpp>
#include "root_help.hh"
#include "TEfficiency.h"
#include <boost/math/common_factor.hpp>

namespace fn
{
    TriggerApp::TriggerApp( TFile& tf )
        :tf_( tf ){}

    void TriggerApp::set_sels( std::vector<std::string> sels )
    {
        sels_ = sels;
    }

    void TriggerApp::init()
    {
        std::vector<std::unique_ptr<TH1D>> vh_passed;
        std::vector<std::unique_ptr<TH1D>> vh_all;

        for ( auto sel : sels_ )
        {
            boost::filesystem::path folder {sel};
            auto ih_passed = extract_hist<TH1D>(tf_,  folder / "h_passed");
            vh_passed.push_back( std::move(ih_passed ) ) ;
            auto ih_all = extract_hist<TH1D>( tf_, folder / "h_all");
            vh_all.push_back( std::move(ih_all ) ) ;
        }

        h_passed_ = sum_hists( begin( vh_passed ), end( vh_passed ) );
        h_all_ = sum_hists( begin( vh_all ), end( vh_all ) );
    }

    void TriggerApp::rebin( int r )
    {
        h_passed_->Rebin( r );
        h_all_->Rebin( r );
    }

    std::pair<double,double> 
        TriggerApp::get_eff_err( double sigmin, double sigmax ) const 
    {
        double intpass = integral( *h_passed_, sigmin, sigmax );
        double intall = integral( *h_all_, sigmin, sigmax );
        return eff_err( intpass, intall );
    }

    double TriggerApp::get_eff( double sigmin, double sigmax )
    { return get_eff_err( sigmin, sigmax ).first ; }

    double TriggerApp::get_err( double sigmin, double sigmax )
    { return get_eff_err( sigmin, sigmax ).second ; }


    std::pair<double,double> eff_err( double passed, double all ) 
    {
        double upper = TEfficiency::ClopperPearson( all, passed, 0.6827, true );
        double lower = TEfficiency::ClopperPearson( all, passed, 0.6827, false );

        double eff = passed / all;
        return std::make_pair( eff , (upper - lower)/2 );
    }

    bool TriggerApp::correct_hist( TH1& h )
    {

        TH1D * hpass = 0;
        TH1D * hall = 0 ;

        std::unique_ptr<TH1D> h_e_pass;
        std::unique_ptr<TH1D> h_e_all;

        int hnbin = h.GetNbinsX();
        int enbin = h_all_->GetNbinsX();

        int nbin = 0;

        if ( hnbin == enbin )
        {
            hpass = h_passed_.get();
            hall = h_all_.get();
        }
        else
        {
            int common_factor = boost::math::gcd( hnbin, enbin );
            int erebin = enbin / common_factor;
            int hrebin = hnbin / common_factor;

            std::cerr << "Rebinning eff by " << erebin << std::endl;
            std::cerr << "Rebinning h by " << hrebin << std::endl;

            h.Rebin( hrebin );

            h_e_pass = uclone( h_passed_ );
            h_e_pass->Rebin( erebin );

            h_e_all = uclone( h_all_ );
            h_e_all->Rebin( erebin );

            hpass = h_e_pass.get();
            hall = h_e_all.get();

            nbin = hall->GetNbinsX();
        }

        //Sort out limits
        double hmin   = h.GetXaxis()->GetBinLowEdge( 1 );
        double hmax   = h.GetXaxis()->GetBinLowEdge( nbin );

        double emin   = hall->GetXaxis()->GetBinLowEdge( 1 );
        double emax   = hall->GetXaxis()->GetBinLowEdge( nbin );

        if ( (hmin != emin ) || ( hmax != emax ) )
        {
            std::cerr << "Range mismatch. Can't correct histogram" << std::endl;
            return false;
        }

        auto heff = std::unique_ptr<TH1D>( tclone( *hpass ) );
        heff->Divide( hall );
        h.Multiply( heff.get() );
        return true;
    }
}
