#include "easy_app.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "HistStore.hh"
#include "TGraphAsymmErrors.h"
#include "THStack.h"
#include "TLegend.h"
#include "TriggerApp.hh"
#include <vector>


std::pair<std::unique_ptr<TH1>, std::unique_ptr<OwningStack>> extract( TFile& tfin, std::vector<std::string> paths)
{
    std::vector<std::string> stack_paths;
    std::vector<std::string> data_paths;

    for( auto fp : paths  )
    {
        stack_paths.push_back( ( path{fp} / "hnu_stack" ).string() );
        data_paths.push_back( ( path{fp} / "hdata" ).string() );
    }

    auto stacks = extract_object_list<THStack>( tfin, stack_paths );
    auto owning_stack_sum =  sum_stacks(begin( stacks ), end( stacks ) );

    auto data_hists = extract_hist_list<TH1D>( tfin, data_paths );
    auto data_sum = sum_hists( begin( data_hists), end( data_hists ) );

    return std::make_pair( std::move( data_sum ),  std::move( owning_stack_sum ) );
} 

void merge( TFile& tfin, TFile& tfout, std::string output_folder, std::vector<std::string> paths  )
{
    auto data_stack = extract( tfin, paths );
    cd_p( &tfout, output_folder );
    data_stack.first->Write("hdata");
    data_stack.second->Write("hnu_stack");
}

std::pair<std::unique_ptr<TH1>, std::unique_ptr<TH1>> get_trigger_efficiency( TFile& tf )
{
    std::vector<std::string> sels
    {
        "p5.data.q1.v4.neg/sig_up_trig_eff"
        "p5.data.q1.v4.neg/sig_dn_trig_eff"
        "p5.data.q1.v4.pos/sig_up_trig_eff"
        "p5.data.q1.v4.pos/sig_dn_trig_eff"
    };

    TriggerApp ta{ tf};
    ta.set_sels( sels );
    ta.init();

     auto h_passed = std::unique_ptr<TH1D> ( tclone( ta.get_h_passed()) );
     auto h_all = std::unique_ptr<TH1D> ( tclone(ta.get_h_all()) );

    std::cout << h_all->GetNbinsX() << std::endl;

    return std::make_pair( std::move(h_all), std::move(h_passed) );
}

int main()
{
    TFile tfinnoscat{ "output/km2_noscat.root" };
    TFile tfinscat{ "output/km2_scat.root" };
    TFile tfout{ "output/km2_both_scat.root", "RECREATE" };

    std::vector<std::string> regs { "up/pos", "up/neg", "dn/pos", "dn/neg" };
    std::vector<std::string> paths;

    for( auto reg : regs  )
    {
        path fp = path{"signal_muv" } / reg / "h_m2m";
        paths.push_back( fp.string() );
    }

    merge( tfinnoscat, tfout, "noscat", paths );
    merge( tfinscat, tfout, "scat" , paths );

    return 0;

#if 0
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
#endif
}
