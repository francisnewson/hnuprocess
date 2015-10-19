#include "easy_app.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "HistStore.hh"
#include "yaml_help.hh"
#include <cassert>
#include "MuvXYP.hh"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TEfficiency.h"

std::unique_ptr<TH3D> extract_3D_hist( TFile& tf, path channel, path hist_name )
{
    path pos_channel = channel;
    pos_channel += std::string(".pos");

    path neg_channel = channel;
    neg_channel += std::string(".neg");

    auto h_result = extract_hist<TH3D>( tf, pos_channel / hist_name );
    auto h_neg_result = extract_hist<TH3D>( tf, neg_channel / hist_name );

    h_result->Add( h_neg_result.get() );
    //h_result->Rebin3D( 2, 2, 1 );
    return h_result;
}

std::unique_ptr<TH1D> hist_variation( TH1& h )
{
    Int_t nBins = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

    auto result = make_unique<TH1D>( "hvar", "variation", 1000, 0.8, 1.2 );

    for ( Int_t gBin = 1; gBin != nBins + 1 ; ++gBin )
    {
        result->Fill( h.GetBinContent( gBin ) );
    }

    return result;
}

int main( int argc, char * argv[] )
{
    TH1::AddDirectory(kFALSE);

    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/extract_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream flaunch( "elaunch.log", std::ofstream::app );
        write_launch ( argc, argv, flaunch );
    }


    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "input,i", po::value<path>(), "input file")
        ( "output,o", po::value<path>(), "output file")
        ( "conditions,c", po::value<path>(), "conditions file")
        ;

    po::options_description desc("Allowed options");
    desc.add( general );

    //**************************************************
    //Parse options
    //**************************************************

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    if ( vm.count( "help" ) )
    {
        std::cerr << desc << std::endl;
        std::cerr << "Exiting because help was requested" << std::endl;
        exit( 1 );
    }

    if ( ! vm.count("input" ) )
    {
        std::cerr << "Input file must be specified!" << std::endl;
        exit( 1 );
    }

    path output_file = "output/muv_3d_eff.root";
    if ( vm.count( "output" ) )
    {
        output_file = vm["output"].as<path>(); 
    }

    //Load ROOT Files
    path input_file = vm["input"].as<path>();
    auto tfin = get_tfile( input_file );
    auto tfout = get_tfile( output_file, "RECREATE");

    bool do_conditions =   vm.count( "conditions" );
    std::unique_ptr<TFile> tfcond;
    if ( do_conditions  )
    {
        path conditions_file = vm["conditions"].as<path>();
        tfcond = get_tfile( conditions_file, "RECREATE" );
    }

    std::string rectangles = 
        R"R(
        shape: rectangles
        points:
        - [ -30,  30,  80,  90 ]
        - [ -50,  50,  70,  80 ]
        - [ -60,  60,  60,  70 ]
        - [ -70,  70,  50,  60 ]
        - [ -80,  80,  40,  50 ]
        - [ -80,  80,  30,  40 ]
        - [ -90, -20,  20,  30 ]
        - [  20,  90,  20,  30 ]
        - [ -90, -30,  10,  20 ]
        - [  30,  90,  10,  20 ]
        - [ -90, -30,   0,  10 ]
        - [  30,  90,   0,  10 ]
        - [ -90, -30, -10,   0 ]
        - [  30,  90, -10,   0 ]
        - [ -90, -30, -20, -10 ]
        - [  30,  90, -20, -10 ]
        - [ -90, -20, -30, -20 ]
        - [  20,  90, -30, -20 ]
        - [ -80,  80, -40, -30 ]
        - [ -80,  80, -50, -40 ]
        - [ -70,  70, -60, -50 ]
        - [ -60,  60, -70, -60 ]
        - [ -50,  50, -80, -70 ]
        - [ -30,  30, -90, -80 ]
        )R" ;


    //**************************************************
    //Selections
    //**************************************************
    YAML::Node muv_xy_cut_node = YAML::Load( rectangles );
    muveff::CutXY muv_xy_cut( muv_xy_cut_node );

    muveff::CutP high_mom_cut{ 30, 60 };
    muveff::CutP clean_mom_cut{ 5, 70 };

    muveff::CompProcess high_mom_xy_cut{ {&muv_xy_cut, &high_mom_cut } };

    //**************************************************
    //Histograms
    //**************************************************

    //Extract raw hists
    path data_channel = "p5.data.q11t.v3";
    const std::unique_ptr<TH3D> h_dt_total 
        = extract_3D_hist( *tfin, data_channel, "muv_eff_raw/h_xyp_total");
    const std::unique_ptr<TH3D> h_dt_passed 
        = extract_3D_hist( *tfin, data_channel, "muv_eff_raw/h_xyp_passed");

    path mc_channel = "p5.km2.v2";
    const std::unique_ptr<TH3D> h_mc_total 
        = extract_3D_hist( *tfin, mc_channel, "muv_eff_raw/h_xyp_total");
    const std::unique_ptr<TH3D> h_mc_passed 
        = extract_3D_hist( *tfin, mc_channel, "muv_eff_raw/h_xyp_passed");

    //RAW HISTS
    //Just do npassed/ntotal for each xy bin


    //Do raw effs in xy
    std::cerr << "Doing raw hists" << std::endl;
    std::unique_ptr<TH2D> xy_eff_dt_raw = muveff::xy_eff( *h_dt_passed, *h_dt_total );
    std::unique_ptr<TH2D> xy_eff_mc_raw = muveff::xy_eff( *h_mc_passed, *h_mc_total );

    cd_p( tfout.get(), "raw" );
    xy_eff_dt_raw->SetTitle( "Raw xy efficiency (data)" );
    xy_eff_dt_raw->Write( "dt_xy_eff" );
    xy_eff_mc_raw->SetTitle( "Raw xy efficiency (MC)" );
    xy_eff_mc_raw->Write( "mc_xy_eff" );

    //CLEAN MOM HISTS
    //Remove low p
    auto h_dt_cp_total = uclone( h_dt_total );
    auto h_dt_cp_passed = uclone( h_dt_passed );
    auto h_mc_cp_total = uclone( h_mc_total );
    auto h_mc_cp_passed = uclone( h_mc_passed );

    std::cerr << "Processing high mom cut" << std::endl;
    for ( auto h : { h_dt_cp_total.get(), h_dt_cp_passed.get(),
            h_mc_cp_total.get(), h_mc_cp_passed.get() } )
    {
        muveff::process_hist( *h, clean_mom_cut );
    }


    //Do clean effs in xy
    std::cerr << "Doing clean hists" << std::endl;
    std::unique_ptr<TH2D> xy_eff_dt_cp = muveff::xy_eff( *h_dt_cp_passed, *h_dt_cp_total );
    std::unique_ptr<TH2D> xy_eff_mc_cp = muveff::xy_eff( *h_mc_cp_passed, *h_mc_cp_total );
    std::unique_ptr<TEfficiency> p_eff_dt_cp = muveff::p_eff( *h_dt_cp_passed, *h_dt_cp_total );
    std::unique_ptr<TEfficiency> p_eff_mc_cp = muveff::p_eff( *h_mc_cp_passed, *h_mc_cp_total );

    cd_p( tfout.get(), "clean" );
    xy_eff_dt_cp->SetTitle( "Clean xy efficiency (data)" );
    xy_eff_dt_cp->Write( "dt_xy_eff" );
    xy_eff_mc_cp->SetTitle( "Clean xy efficiency (MC)" );
    xy_eff_mc_cp->Write( "mc_xy_eff" );

    p_eff_dt_cp->SetTitle( "Clean p efficiency (data)" );
    p_eff_dt_cp->Write( "dt_p_eff" );
    p_eff_mc_cp->SetTitle( "Clean p efficiency (MC)" );
    p_eff_mc_cp->Write( "mc_p_eff" );

    std::cerr << "Done" << std::endl;

    //------------------------------

    //HIGH MOMENTUM HISTS
    //Select high p events then do npassed/nototal for each xy bin
    std::cerr << "\nDoing high p hists" << std::endl;
    auto h_dt_hp_total = uclone( h_dt_total );
    auto h_dt_hp_passed = uclone( h_dt_passed );
    auto h_mc_hp_total = uclone( h_mc_total );
    auto h_mc_hp_passed = uclone( h_mc_passed );

    std::cerr << "Processing high mom cut" << std::endl;
    for ( auto h : { h_dt_hp_total.get(), h_dt_hp_passed.get(),
            h_mc_hp_total.get(), h_mc_hp_passed.get() } )
    {
        muveff::process_hist( *h, high_mom_cut );
    }

    //Do high p effs
    std::cerr << "Extracting effs" << std::endl;
    std::unique_ptr<TH2D> xy_eff_dt_hp = muveff::xy_eff( *h_dt_hp_passed, *h_dt_hp_total );
    std::unique_ptr<TH2D> xy_eff_mc_hp = muveff::xy_eff( *h_mc_hp_passed, *h_mc_hp_total );

    std::cerr << "Saving" << std::endl;
    cd_p( tfout.get(), "hp" );
    xy_eff_dt_hp->Write( "raw high-mom xy-efficiency (data)" );
    xy_eff_dt_hp->Write( "dt_xy_eff" );
    xy_eff_mc_hp->Write( "raw high-mom xy-efficiency (MC)" );
    xy_eff_mc_hp->Write( "mc_xy_eff" );

    std::cerr << "Finished" << std::endl;

    //------------------------------

    //HIGH MOMENTUM XY CUT HISTS
    //Select events with high momentum and in the tight xy selection
    //Compute xy efficiency
    std::cerr << "\nCloning high p hists" << std::endl;
    auto h_dt_hpxy_total = uclone( h_dt_total );
    auto h_dt_hpxy_passed = uclone( h_dt_passed );
    auto h_mc_hpxy_total = uclone( h_mc_total );
    auto h_mc_hpxy_passed = uclone( h_mc_passed );

    std::cerr << "Processing high mom xy cut" << std::endl;
    for ( auto h : { h_dt_hpxy_total.get(), h_dt_hpxy_passed.get(),
            h_mc_hpxy_total.get(), h_mc_hpxy_passed.get() } )
    {
        muveff::process_hist( *h, high_mom_xy_cut );
    }

    //Do high p effs
    std::cerr << "Extracting effs" << std::endl;
    std::unique_ptr<TH2D> xy_eff_dt_hpxy = muveff::xy_eff( *h_dt_hpxy_passed, *h_dt_hpxy_total );
    std::unique_ptr<TH2D> xy_eff_mc_hpxy = muveff::xy_eff( *h_mc_hpxy_passed, *h_mc_hpxy_total );

    std::cerr << "Saving" << std::endl;
    cd_p( tfout.get(), "hpxyp" );
    xy_eff_dt_hpxy->SetTitle( "raw high-mom tight-xy xy-eff (data)" );
    xy_eff_dt_hpxy->Write( "dt_xy_eff" );
    xy_eff_mc_hpxy->SetTitle( "raw high-mom tight-xy xy-eff (MC)" );
    xy_eff_mc_hpxy->Write( "mc_xy_eff" );

    //extract xy correction
    auto xy_eff_hpxy_corr = uclone( xy_eff_dt_hpxy );
    xy_eff_hpxy_corr->Divide( xy_eff_mc_hpxy.get() );
    xy_eff_hpxy_corr->SetTitle( "correction for high-mom tight-xy xy-eff (data/MC)" );
    xy_eff_hpxy_corr->Write( "xy_eff_corr" );

    //------------------------------

    //XY CUT HISTS ( full momentum )
    //Use the measured xy correction on the tight xy selection
    //Now we can measure efficiency vs p for all p

    std::cerr << "\nCloning xy cut hists" << std::endl;
    auto h_dt_xycut_total = uclone( h_dt_total );
    auto h_dt_xycut_passed = uclone( h_dt_passed );
    auto h_mc_xycut_total = uclone( h_mc_total );
    auto h_mc_xycut_passed = uclone( h_mc_passed );
    auto h_mc_xycut_passed_rw_hpxy = uclone( h_mc_passed );

    std::cerr << "Processing  xy cut" << std::endl;
    for ( auto h : { h_dt_xycut_total.get(), h_dt_xycut_passed.get(),
            h_mc_xycut_total.get(), h_mc_xycut_passed.get(), h_mc_xycut_passed_rw_hpxy.get() } )
    {
        muveff::process_hist( *h, muv_xy_cut );
    }

    //Here we extract raw efficiencies or the full-mom tight-xy selection
    //No corrections are applied
    std::cerr << "Extracting effs" << std::endl;
    std::unique_ptr<TEfficiency> p_eff_dt_xycut = muveff::p_eff( *h_dt_xycut_passed, *h_dt_xycut_total );
    std::unique_ptr<TEfficiency> p_eff_mc_xycut = muveff::p_eff( *h_mc_xycut_passed, *h_mc_xycut_total );

    cd_p( tfout.get(), "xycut" );
    p_eff_dt_xycut->SetTitle("raw full-mom tight-xy p-eff (data)");
    p_eff_dt_xycut->Write("dt_p_eff");
    p_eff_mc_xycut->SetTitle("raw full-mom tight-xy p-eff (MC)");
    p_eff_mc_xycut->Write("mc_p_eff");

    std::unique_ptr<TH2D> xy_eff_dt_xycut = muveff::xy_eff( *h_dt_xycut_passed, *h_dt_xycut_total );
    std::unique_ptr<TH2D> xy_eff_mc_xycut = muveff::xy_eff( *h_mc_xycut_passed, *h_mc_xycut_total );

    xy_eff_dt_xycut->SetTitle( "raw full-mom tight-xy xy-eff (data)" );
    xy_eff_dt_xycut->Write( "dt_xy_eff" );
    xy_eff_mc_xycut->SetTitle( "raw full-mom tight-xy xy-eff (MC)" );
    xy_eff_mc_xycut->Write( "mc_xy_eff" );

    //Now we reweight using the weights measured in the high-mom tight-xy selection
    muveff::ReweightXY rew_hpxy{ *xy_eff_hpxy_corr };
    muveff::process_hist( *h_mc_xycut_passed_rw_hpxy, rew_hpxy );

    std::unique_ptr<TH2D> xy_eff_mc_xycut_rewhpxy =
        muveff::xy_eff( *h_mc_xycut_passed_rw_hpxy, *h_mc_xycut_total );
    xy_eff_mc_xycut_rewhpxy->SetTitle( "rew(high-mom tight-xy xy-eff) full-mom xy-eff (MC)" );
    xy_eff_mc_xycut_rewhpxy->Write( "mc_xy_eff_rew_hpxy" );

    std::unique_ptr<TEfficiency> p_eff_mc_xycut_rehpxy 
        = muveff::p_eff( *h_mc_xycut_passed_rw_hpxy, *h_mc_xycut_total );

    p_eff_mc_xycut_rehpxy->SetTitle( "rew(high-mom tight-xy xy-eff) full-mom p-eff (MC)" );
    p_eff_mc_xycut_rehpxy->Write( "mc_p_eff_rew_hpxy" );

    //We can use these reweighted plots to determine the required p-eff correction
    //get p correction
    auto pcorr = muveff::get_ratio( *p_eff_dt_xycut );
    auto hrat_p_eff_mc_xycut_rehpxy = muveff::get_ratio( *p_eff_mc_xycut_rehpxy );
    pcorr->Divide( hrat_p_eff_mc_xycut_rehpxy.get() );
    pcorr->SetTitle( "correction for full-mom tight-xy p-eff" );
    pcorr->Write( "p_eff_corr" );

    //normalize p-correction to be < 1.0 for all p
    pcorr->Scale( 1.0 / pcorr->GetMaximum() );
    pcorr->SetTitle( "normalized correction for full-mom tight-xy p-eff" );
    pcorr->Write( "p_eff_corr_norm" );

    //------------------------------

    //FULL P
    //Redo xy correction using mom correction
    //We can now determine xy eff across whole surface of MUVs
    //because we can correct for p

    std::cerr << "\nDoing full p hists" << std::endl;
    auto h_dt_fullp_total = uclone( h_dt_total );
    auto h_dt_fullp_passed = uclone( h_dt_passed );
    auto h_mc_fullp_total = uclone( h_mc_total );

    auto h_mc_fullp_passed = uclone( h_mc_passed );

    //Do the MC rewweighting 
    muveff::ReweightP rewp{ *pcorr };
    muveff::process_hist( *h_mc_fullp_passed, rewp );

    std::unique_ptr<TH2D> xy_eff_dt_fullp = muveff::xy_eff( *h_dt_fullp_passed, *h_dt_fullp_total );
    std::unique_ptr<TH2D> xy_eff_mc_fullp = muveff::xy_eff( *h_mc_fullp_passed, *h_mc_fullp_total );


    cd_p( tfout.get(), "fullp" );
    xy_eff_dt_fullp->SetTitle( "full-mom full-xy xy-eff (data)" );
    xy_eff_dt_fullp->Write( "dt_xy_eff" );
    xy_eff_mc_fullp->SetTitle( "rew(full-mom tight-xy p-eff) full-mom full-xy xy-eff (MC)" );
    xy_eff_mc_fullp->Write( "mc_xy_eff" );

    //extract xy correction
    auto xy_eff_fullp_corr = uclone( xy_eff_dt_fullp );
    xy_eff_fullp_corr->Divide( xy_eff_mc_fullp.get() );
    xy_eff_fullp_corr->SetTitle( "final xy-eff correction" );
    xy_eff_fullp_corr->Write( "xy_eff_corr" );

    std::cerr << "Finished" << std::endl;

    //The final corrections are saved in the conditions file
    tfcond->cd();
    xy_eff_fullp_corr->Write( "muv_eff_xy" );
    pcorr->Write( "muv_eff_p" );

    //------------------------------

    //Do test hists
    //Munge together various combinations of plots to see if agreement is ok
    std::cerr << "\nDoing test hists" << std::endl;

    auto h_dt_test_total = uclone( h_dt_total );
    auto h_dt_test_passed = uclone( h_dt_passed );
    auto h_mc_test_total = uclone( h_mc_total );
    auto h_mc_test_passed = uclone( h_mc_passed );
    auto h_mc_test_passed_raw = uclone( h_mc_passed );

    //Look at plots in the MUV acceptance xy region
    muveff::MuvAcc muv_acc;
    muveff::ReweightXY rewxy_final{ *xy_eff_fullp_corr };

    muveff::CompProcess full_corr{ {&rewxy_final, &rewp } };

    //extract data
    muveff::process_hist( *h_dt_test_total, muv_acc );
    muveff::process_hist( *h_dt_test_passed, muv_acc );

    //extract mc raw
    muveff::process_hist( *h_mc_test_total, muv_acc );
    muveff::process_hist( *h_mc_test_passed_raw, muv_acc );

    //produce mc cor( apply both xy and p corrections )
    muveff::process_hist( *h_mc_test_passed, muv_acc );
    muveff::process_hist( *h_mc_test_passed, full_corr );

    //Measure efficiencies (xy)
    std::unique_ptr<TH2D> xy_eff_dt_test = muveff::xy_eff( *h_dt_test_passed, *h_dt_test_total );
    std::unique_ptr<TH2D> xy_eff_mc_test = muveff::xy_eff( *h_mc_test_passed, *h_mc_test_total );

    cd_p( tfout.get(), "test" );
    xy_eff_dt_test->SetTitle( "muv_acc full-p xy-eff (data)" );
    xy_eff_dt_test->Write( "dt_xy_eff" );
    xy_eff_mc_test->SetTitle( "rew(final xyp) muv_acc xy-eff (MC)" );
    xy_eff_mc_test->Write( "mc_xy_eff" );

    //check xy ratio
    auto xy_eff_rat = uclone( xy_eff_dt_test );
    xy_eff_rat->Divide( xy_eff_mc_test.get() );
    xy_eff_rat->SetTitle( "rew(final xyp) muv_acc data/MC xy ratio" );
    xy_eff_rat->Write( "xy_eff_test" );

    auto xy_var = hist_variation( *xy_eff_rat );
    xy_var->SetTitle( "rew(final xyp) muv_acc xy ratio var" );
    xy_var->Write( "xy_eff_test_var" );

    //Measure efficiencies (p)
    std::unique_ptr<TEfficiency> p_eff_dt_test = muveff::p_eff( *h_dt_test_passed, *h_dt_test_total );
    std::unique_ptr<TEfficiency> p_eff_mc_test = muveff::p_eff( *h_mc_test_passed, *h_mc_test_total );
    std::unique_ptr<TEfficiency> p_eff_mc_test_raw = muveff::p_eff( *h_mc_test_passed_raw, *h_mc_test_total );

    p_eff_dt_test->SetTitle( "muv_acc full-p p-eff (data)" );
    p_eff_dt_test->Write( "dt_p_eff" );
    p_eff_mc_test->SetTitle( "rew(final xyp) muv_acc full-p p-eff (mc)" );
    p_eff_mc_test->Write( "mc_p_eff" );
    p_eff_mc_test->SetTitle( "raw muv_acc full-p p-eff (mc)" );
    p_eff_mc_test->Write( "mc_p_raw" );

    //check p ratio
    std::unique_ptr<TH1D> p_eff_rat = muveff::get_ratio( *p_eff_dt_test );
    std::unique_ptr<TH1D> p_eff_mc = muveff::get_ratio( *p_eff_mc_test );
    p_eff_rat->Divide( p_eff_mc.get() );
    p_eff_rat->SetTitle( "rew(final xyp) muv_acc data/MC p ratio" );
    p_eff_rat->Write( "p_eff_rat" );

    std::unique_ptr<TH1D> p_eff_rat_raw = muveff::get_ratio( *p_eff_dt_test );
    std::unique_ptr<TH1D> p_eff_mc_raw = muveff::get_ratio( *p_eff_mc_test_raw );
    p_eff_rat_raw->Divide( p_eff_mc_raw.get() );
    p_eff_rat_raw->SetTitle( "raw muv_acc data/MC p ratio" );
    p_eff_rat_raw->Write( "p_eff_rat_raw" );

    auto p_var = hist_variation( *p_eff_rat );
    p_var->SetTitle( "rew(final xyp) muv_acc p ratio var" );
    p_var->Write( "p_eff_test_var" );

    //Look at variation vs r
    std::cerr << "Doing radial plots" << std::endl;
    std::unique_ptr<TEfficiency> r_eff_dt = muveff::r_eff( *h_dt_test_passed, * h_dt_test_total );
    std::unique_ptr<TEfficiency> r_eff_mc_raw = muveff::r_eff( *h_mc_test_passed_raw, * h_mc_test_total );
    std::unique_ptr<TEfficiency> r_eff_mc_corr = muveff::r_eff( *h_mc_test_passed, * h_mc_test_total);

    std::unique_ptr<TH1D> r_rat_raw = muveff::get_ratio( *r_eff_dt );
    std::unique_ptr<TH1D> r_rat_mc_raw = muveff::get_ratio( *r_eff_mc_raw );
    r_rat_raw->Divide( r_rat_mc_raw.get() );
    r_rat_raw->SetTitle( "raw muv_acc data/MC r ratio" );
    r_rat_raw->Write( "r_rat_raw" );

    std::unique_ptr<TH1D> r_rat_corr = muveff::get_ratio( *r_eff_dt );
    std::unique_ptr<TH1D> r_rat_mc_corr = muveff::get_ratio( *r_eff_mc_corr );
    r_rat_corr->Divide( r_rat_mc_corr.get() );
    r_rat_corr->SetTitle( "rew (final xyp) muv_acc data/MC r ratio" );
    r_rat_corr->Write( "r_rat_corr" );

    auto r_var = hist_variation( *r_rat_corr );
    r_var->SetTitle( "rew(final xyp) muv_acc r ratio var" );
    r_var->Write( "r_eff_test_var" );

}
