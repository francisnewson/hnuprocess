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
    return h_result;
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

    //Do raw effs
    std::cerr << "Doing raw hists" << std::endl;
    std::unique_ptr<TH2D> xy_eff_dt_raw = muveff::xy_eff( *h_dt_passed, *h_dt_total );
    std::unique_ptr<TH2D> xy_eff_mc_raw = muveff::xy_eff( *h_mc_passed, *h_mc_total );

    cd_p( tfout.get(), "raw" );
    xy_eff_dt_raw->Write( "dt_xy_eff" );
    xy_eff_mc_raw->Write( "mc_xy_eff" );

    //------------------------------

    //HIGH MOMENTUM HISTS
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
    xy_eff_dt_hp->Write( "dt_xy_eff" );
    xy_eff_mc_hp->Write( "mc_xy_eff" );

    std::cerr << "Finished" << std::endl;

    //------------------------------

    //HIGH MOMENTUM XY CUT HISTS
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
    xy_eff_dt_hpxy->Write( "dt_xy_eff" );
    xy_eff_mc_hpxy->Write( "mc_xy_eff" );

    //extract xy correction
    auto xy_eff_hpxy_corr = uclone( xy_eff_dt_hpxy );
    xy_eff_hpxy_corr->Divide( xy_eff_mc_hpxy.get() );
    xy_eff_hpxy_corr->Write( "xy_eff_corr" );

    //------------------------------

    //XY CUT HISTS ( full momentum )
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

    std::cerr << "Extracting effs" << std::endl;
    std::unique_ptr<TEfficiency> p_eff_dt_xycut = muveff::p_eff( *h_dt_xycut_passed, *h_dt_xycut_total );
    std::unique_ptr<TEfficiency> p_eff_mc_xycut = muveff::p_eff( *h_mc_xycut_passed, *h_mc_xycut_total );

    cd_p( tfout.get(), "xycut" );
    p_eff_dt_xycut->Write("dt_p_eff");
    p_eff_mc_xycut->Write("mc_p_eff");

    std::unique_ptr<TH2D> xy_eff_dt_xycut = muveff::xy_eff( *h_dt_xycut_passed, *h_dt_xycut_total );
    std::unique_ptr<TH2D> xy_eff_mc_xycut = muveff::xy_eff( *h_mc_xycut_passed, *h_mc_xycut_total );

    xy_eff_dt_xycut->Write( "dt_xy_eff" );
    xy_eff_mc_xycut->Write( "mc_xy_eff" );

    //reweighting
    muveff::ReweightXY rew_hpxy{ *xy_eff_hpxy_corr };
    muveff::process_hist( *h_mc_xycut_passed_rw_hpxy, rew_hpxy );

    std::unique_ptr<TH2D> xy_eff_mc_xycut_rewhpxy =
        muveff::xy_eff( *h_mc_xycut_passed_rw_hpxy, *h_mc_xycut_total );
    xy_eff_mc_xycut_rewhpxy->Write( "mc_xy_eff_rew_hpxy" );

    std::unique_ptr<TEfficiency> p_eff_mc_xycut_rehpxy 
        = muveff::p_eff( *h_mc_xycut_passed_rw_hpxy, *h_mc_xycut_total );

    p_eff_mc_xycut_rehpxy->Write( "mc_p_eff_rew_hpxy" );

    //get p correction
    auto pcorr = muveff::get_ratio( *p_eff_dt_xycut );
    auto hrat_p_eff_mc_xycut_rehpxy = muveff::get_ratio( *p_eff_mc_xycut_rehpxy );
    pcorr->Divide( hrat_p_eff_mc_xycut_rehpxy.get() );
    pcorr->Write( "p_eff_corr" );

    //normalize
    pcorr->Scale( 1.0 / pcorr->GetMaximum() );
    pcorr->Write( "p_eff_corr_norm" );

    //------------------------------

    //FULL P
    //Redo xy correction using mom correction

    std::cerr << "\nDoing full p hists" << std::endl;
    auto h_dt_fullp_total = uclone( h_dt_total );
    auto h_dt_fullp_passed = uclone( h_dt_passed );
    auto h_mc_fullp_total = uclone( h_mc_total );

    auto h_mc_fullp_passed = uclone( h_mc_passed );

    muveff::ReweightP rewp{ *pcorr };
    muveff::process_hist( *h_mc_fullp_passed, rewp );

    std::unique_ptr<TH2D> xy_eff_dt_fullp = muveff::xy_eff( *h_dt_fullp_passed, *h_dt_fullp_total );
    std::unique_ptr<TH2D> xy_eff_mc_fullp = muveff::xy_eff( *h_mc_fullp_passed, *h_mc_fullp_total );

    cd_p( tfout.get(), "fullp" );
    xy_eff_dt_fullp->Write( "dt_xy_eff" );
    xy_eff_mc_fullp->Write( "mc_xy_eff" );

    //extract xy correction
    auto xy_eff_fullp_corr = uclone( xy_eff_dt_fullp );
    xy_eff_fullp_corr->Divide( xy_eff_mc_fullp.get() );
    xy_eff_fullp_corr->Write( "xy_eff_corr" );

    std::cerr << "Finished" << std::endl;

    //------------------------------

    //Do test hists
    std::cerr << "\nDoing test hists" << std::endl;

    auto h_dt_test_total = uclone( h_dt_total );
    auto h_dt_test_passed = uclone( h_dt_passed );
    auto h_mc_test_total = uclone( h_mc_total );
    auto h_mc_test_passed = uclone( h_mc_passed );
    auto h_mc_test_passed_raw = uclone( h_mc_passed );

    muveff::MuvAcc muv_acc;
    muveff::ReweightXY rewxy_final{ *xy_eff_fullp_corr };

    muveff::CompProcess full_corr{ {&rewxy_final, &rewp } };

    muveff::process_hist( *h_dt_test_total, muv_acc );
    muveff::process_hist( *h_dt_test_passed, muv_acc );
    muveff::process_hist( *h_mc_test_total, muv_acc );
    muveff::process_hist( *h_mc_test_passed, muv_acc );
    muveff::process_hist( *h_mc_test_passed_raw, muv_acc );
    muveff::process_hist( *h_mc_test_passed, full_corr );

    //XY
    std::unique_ptr<TH2D> xy_eff_dt_test = muveff::xy_eff( *h_dt_test_passed, *h_dt_test_total );
    std::unique_ptr<TH2D> xy_eff_mc_test = muveff::xy_eff( *h_mc_test_passed, *h_mc_test_total );

    cd_p( tfout.get(), "test" );
    xy_eff_dt_test->Write( "dt_xy_eff" );
    xy_eff_mc_test->Write( "mc_xy_eff" );

    //extract xy ratio
    auto xy_eff_rat = uclone( xy_eff_dt_test );
    xy_eff_rat->Divide( xy_eff_mc_test.get() );
    xy_eff_rat->Write( "xy_eff_test" );

    //P
    std::unique_ptr<TEfficiency> p_eff_dt_test = muveff::p_eff( *h_dt_test_passed, *h_dt_test_total );
    std::unique_ptr<TEfficiency> p_eff_mc_test = muveff::p_eff( *h_mc_test_passed, *h_mc_test_total );
    std::unique_ptr<TEfficiency> p_eff_mc_test_raw = muveff::p_eff( *h_mc_test_passed_raw, *h_mc_test_total );

    p_eff_dt_test->Write( "dt_p_eff" );
    p_eff_mc_test->Write( "mc_p_eff" );
    p_eff_mc_test->Write( "mc_p_raw" );

    std::unique_ptr<TH1D> p_eff_rat = muveff::get_ratio( *p_eff_dt_test );
    std::unique_ptr<TH1D> p_eff_mc = muveff::get_ratio( *p_eff_mc_test );
    p_eff_rat->Divide( p_eff_mc.get() );
    p_eff_rat->Write( "p_eff_rat" );

    std::unique_ptr<TH1D> p_eff_rat_raw = muveff::get_ratio( *p_eff_dt_test );
    std::unique_ptr<TH1D> p_eff_mc_raw = muveff::get_ratio( *p_eff_mc_test_raw );
    p_eff_rat_raw->Divide( p_eff_mc_raw.get() );
    p_eff_rat_raw->Write( "p_eff_rat_raw" );

    //r
    std::cerr << "Doing radial plots" << std::endl;
    std::unique_ptr<TEfficiency> r_eff_dt = muveff::r_eff( *h_dt_test_passed, * h_dt_test_total );
    std::unique_ptr<TEfficiency> r_eff_mc_raw = muveff::r_eff( *h_mc_test_passed_raw, * h_mc_test_total );
    std::unique_ptr<TEfficiency> r_eff_mc_corr = muveff::r_eff( *h_mc_test_passed, * h_mc_test_total);

    std::unique_ptr<TH1D> r_rat_raw = muveff::get_ratio( *r_eff_dt );
    std::unique_ptr<TH1D> r_rat_mc_raw = muveff::get_ratio( *r_eff_mc_raw );
    r_rat_raw->Divide( r_rat_mc_raw.get() );
    r_rat_raw->Write( "r_rat_raw" );

    std::unique_ptr<TH1D> r_rat_corr = muveff::get_ratio( *r_eff_dt );
    std::unique_ptr<TH1D> r_rat_mc_corr = muveff::get_ratio( *r_eff_mc_corr );
    r_rat_corr->Divide( r_rat_mc_corr.get() );
    r_rat_corr->Write( "r_rat_corr" );


}
