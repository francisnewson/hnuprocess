#include "TriggerApp.hh"
#include <boost/filesystem/path.hpp>
#include "root_help.hh"
#include "TEfficiency.h"

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

    std::pair<double,double> TriggerApp::get_eff_err( double sigmin, double sigmax )
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
}
