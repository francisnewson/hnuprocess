#include "easy_app.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "HistStore.hh"
#include "TGraphAsymmErrors.h"
#include "THStack.h"
#include "TLegend.h"
#include <vector>


std::pair<std::unique_ptr<TH1>, std::unique_ptr<THStack>> 
extract( TFile& tfin, TFile& tfout, std::string folder  )
{

    std::vector<std::string> folders { "up/pos", "up/neg", "dn/pos", "dn/neg" };

   auto res = make_unique<THStack>();

    std::vector<THStack*> stacks;
    std::vector<TList*> lists;

    std::vector<TH1*> hdatas;

    for( const auto& folder : folders )
    {
        path fp { "signal_muv_plots" };
        fp /= folder;
        fp /= "h_m2m";
        path fstack = fp / "hnu_stack";

        path fdt = fp / "hdata" ;
        auto * hstack = get_object<THStack>( tfin, fstack.string() );
        stacks.push_back( hstack );
        lists.push_back( hstack->GetHists() );

        auto * hdt = get_object<TH1>( tfin, fdt.string() );
        hdatas.push_back( hdt );
    }

    int nChan = lists[0]->GetSize();

    for ( int iChan = 0 ; iChan != nChan ; ++ iChan )
    {
        for ( auto& list : lists )
        {
            TH1D * h = static_cast<TH1D*>( list->At(iChan) );
            res->Add( h );
        }
    }

    cd_p( &tfout, folder );
    res->Write("hnu_stack");

    auto hdata = sum_hists( begin( hdatas ), end( hdatas ) );
    hdata->Write();

    return std::make_pair( std::move(hdata), std::move(res) );
}

std::pair<std::unique_ptr<TH1>, std::unique_ptr<TH1>> get_trigger_efficiency( TFile& tf )
{
    TH1 * hup_all_neg = get_object<TH1>( tf, "p5.data.q1.v4.neg/sig_up_trig_eff/h_all" );
    TH1 * hup_passed_neg = get_object<TH1>( tf, "p5.data.q1.v4.neg/sig_up_trig_eff/h_passed" );

    TH1 * hdn_all_neg = get_object<TH1>( tf, "p5.data.q1.v4.neg/sig_dn_trig_eff/h_all" );
    TH1 * hdn_passed_neg = get_object<TH1>( tf, "p5.data.q1.v4.neg/sig_dn_trig_eff/h_passed" );

    TH1 * hup_all_pos = get_object<TH1>( tf, "p5.data.q1.v4.pos/sig_up_trig_eff/h_all" );
    TH1 * hup_passed_pos = get_object<TH1>( tf, "p5.data.q1.v4.pos/sig_up_trig_eff/h_passed" );

    TH1 * hdn_all_pos = get_object<TH1>( tf, "p5.data.q1.v4.pos/sig_dn_trig_eff/h_all" );
    TH1 * hdn_passed_pos = get_object<TH1>( tf, "p5.data.q1.v4.pos/sig_dn_trig_eff/h_passed" );


    std::vector<TH1*> vall{ hup_all_neg, hdn_all_neg,  hup_all_pos, hdn_all_pos };
    std::vector<TH1*> vpassed{ hup_passed_neg, hdn_passed_neg,  hup_passed_pos, hdn_passed_pos };

    auto hall = sum_hists( begin( vall), end(vall) );
    auto hpassed = sum_hists( begin(vpassed), end(vpassed) );

    std::cout << hall->GetNbinsX() << std::endl;

    return std::make_pair( std::move(hall), std::move(hpassed) );
}

int main()
{


    TFile tfinq1{ "output/halo_signal_sub_q1.root" };
    TFile tfinq11t{ "output/halo_signal_sub_q11t.root" };
    TFile tfout{ "output/halo_signal_combo.root", "RECREATE" };


    extract( tfinq1, tfout, "q1" );
    auto q11t_data_bg = extract( tfinq11t, tfout, "q11t" );

    //DO eff correction
    
    TFile tfeff { "tdata/halo_control/all.halo_control.root" };
    auto all_passed = get_trigger_efficiency( tfeff );
    all_passed.first->Rebin( 25 );
    all_passed.second->Rebin( 25 );

    tfout.cd();
    TGraphAsymmErrors tgase{ all_passed.second.get(), all_passed.first.get() };
    tgase.Write( "eff" );

    auto eff = uclone( all_passed.second );
    eff->Divide( all_passed.first.get() );

    TH1 * hbg = static_cast<TH1*>( q11t_data_bg.second->GetStack()->Last() );
    hbg->Multiply( eff.get()  );
    hbg->SetLineColor( kRed );
    hbg->SetLineWidth( 2 );
    hbg->Write( "q11t_corr" );

    TLegend leg( 0.1, 0.2, 0.4, 0.9);
    leg.AddEntry( hbg, "Corrected BG", "l" );
    leg.AddEntry( q11t_data_bg.first.get(), "Data", "lpe" );

    TList * hists = q11t_data_bg.second->GetHists();
    leg.AddEntry( static_cast<TH1*>( hists->FindObject( "km3_neg" ) ), "K_{#mu3}", "f"  );
    leg.AddEntry( static_cast<TH1*>( hists->FindObject( "km2_neg" ) ), "K_{#mu2}", "f"  );
    leg.AddEntry( static_cast<TH1*>( hists->FindObject( "k2pig_neg" ) ), "K_{2#pi}", "f"  );
    leg.AddEntry( static_cast<TH1*>( hists->FindObject( "k3pi_neg" ) ), "K_{3#pi}", "f"  );
    leg.AddEntry( static_cast<TH1*>( hists->FindObject( "halo_neg" ) ), "Halo", "f"  );
    leg.Write( "leg" );
}
