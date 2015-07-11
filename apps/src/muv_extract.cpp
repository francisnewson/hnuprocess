#include "easy_app.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "yaml_help.hh"
#include "TFile.h"
#include "TEfficiency.h"
#include "TH3D.h"
#include "TH2D.h"
#include "AreaCut.hh"
#include "ezETAProgressBar.hh"
#include <cassert>

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

class MuvXYCut
{
    private:
        AreaCut cut_;

    public:
        MuvXYCut( const YAML::Node& params )
        {
            std::string shape = get_yaml<std::string>( params, "shape");

            if ( shape == "rectangles" )
            {
                std::vector<rectangle> recs=
                    get_yaml<std::vector<rectangle>>( params, "points" );

                cut_ = AreaCut{ recs };
            }
        }

        bool check_xy( double x, double y ) const
        {
            return cut_.allowed( { x, y, } ) ;
        }
};

bool muv_acc( double x, double y, double p )
{
    const double min_x_ = -135.0;
    const double max_x_ = +135.0;
    const double min_y_ = -135.0;
    const double max_y_ = +135.0;
    const double min_u_ = -135.0;
    const double max_u_ = +135.0;
    const double min_v_ = -135.0;
    const double max_v_ = +135.0;

    const double cospi4 = std::cos( 3.141592654 / 4.0 );
    const double sinpi4 = std::cos( 3.141592654 / 4.0 );

    double u = x * cospi4 - y * sinpi4;
    double v = x * sinpi4 + y * cospi4;

    bool bad_inner =  ( ( fabs( x ) < 11 ) && ( fabs( y ) < 11 ) );
    bool good_inner = ! bad_inner;

    bool good_outer =  ( true
            && ( x > min_x_) && (x < max_x_ )
            && ( y > min_y_) && (y < max_y_ )
            && ( u > min_u_) && (u < max_u_ )
            && ( v > min_v_) && (v < max_v_ )
            );

    bool good_p  = (p > 5 );

    return good_inner && good_outer && good_p;
}

void filter_muv_acc( TH3D &h )
{
    Int_t nBins = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

    std::cerr << "Filter xyp: \n";
    ez::ezETAProgressBar eta( nBins );

    eta.start();
    for ( Int_t gBin = 1 ; gBin != nBins + 1 ; ++gBin, ++eta )
    {
        Int_t xBin, yBin, pBin;
        h.GetBinXYZ( gBin, xBin, yBin, pBin );

        double x = h.GetXaxis()->GetBinCenter( xBin );
        double y = h.GetYaxis()->GetBinCenter( yBin );
        double p = h.GetZaxis()->GetBinCenter( pBin );

        if ( !muv_acc( x, y, p ))
        {
            h.SetBinContent( gBin, 0 );
        }
    }

}

void filter_hist_xy( TH3D& h, const MuvXYCut& cut ) 
{
    Int_t nBins = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

    std::cerr << "Filter hist: \n";
    ez::ezETAProgressBar eta( nBins );

    eta.start();
    for ( Int_t gBin = 1 ; gBin != nBins + 1 ; ++gBin, ++eta )
    {
        Int_t xBin, yBin, zBin;
        h.GetBinXYZ( gBin, xBin, yBin, zBin );

        double x = h.GetXaxis()->GetBinCenter( xBin );
        double y = h.GetYaxis()->GetBinCenter( yBin );
        double z = h.GetZaxis()->GetBinCenter( zBin );

        if ( !cut.check_xy( x, y ) )
        {
            h.SetBinContent( gBin, 0 );
        }
    }
}

void reweight_hist_xy( TH3D & h, const TH2D& hw )
{
    Int_t nBins = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

    assert( h.GetNbinsX() == hw.GetNbinsX() );
    assert( h.GetNbinsY() == hw.GetNbinsY() );

    std::cerr << "Reweight hist: \n";
    ez::ezETAProgressBar eta( nBins );

    eta.start();
    for ( Int_t gBin = 1 ; gBin != nBins + 1 ; ++gBin, ++eta )
    {
        Int_t xBin, yBin, zBin;
        h.GetBinXYZ( gBin, xBin, yBin, zBin );

        double eff = hw.GetBinContent( xBin, yBin );

        h.SetBinContent( gBin, h.GetBinContent( gBin ) * eff );
    }
}

void reweight_hist_p( TH3D & h, const TH1D& hw )
{
    Int_t nBins = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

    assert( h.GetNbinsZ() == hw.GetNbinsX() );

    std::cerr << "Reweight hist: \n";
    ez::ezETAProgressBar eta( nBins );

    for ( Int_t gBin = 1 ; gBin != nBins + 1 ; ++gBin, ++eta )
    {
        Int_t xBin, yBin, zBin;
        h.GetBinXYZ( gBin, xBin, yBin, zBin );

        double eff = hw.GetBinContent( zBin );
        h.SetBinContent( gBin, h.GetBinContent( gBin ) * eff );
    }
}

void reweight_hist_xyp( TH3D &h, const TH2D& hxy, const TH1D& hp )
{
    Int_t nBins = h.GetNbinsX() * h.GetNbinsY() * h.GetNbinsZ();

    assert( h.GetNbinsX() == hxy.GetNbinsX() );
    assert( h.GetNbinsY() == hxy.GetNbinsY() );
    assert( h.GetNbinsZ() == hp.GetNbinsX() );

    std::cerr << "Filter hist: \n";
    ez::ezETAProgressBar eta( nBins );

    eta.start();
    for ( Int_t gBin = 1 ; gBin != nBins + 1 ; ++gBin, ++eta )
    {
        Int_t xBin, yBin, pBin;
        h.GetBinXYZ( gBin, xBin, yBin, pBin );

        double eff = hxy.GetBinContent( xBin, yBin ) * hp.GetBinContent( pBin );

        h.SetBinContent( gBin, h.GetBinContent( gBin ) * eff );
    }

}

std::unique_ptr<TH1D> get_ratio( TEfficiency& teff )
{
    std::unique_ptr<TH1D> hpassed { static_cast<TH1D*>(tclone( *teff.GetPassedHistogram() ) ) };
    const TH1 * htotal = teff.GetTotalHistogram();
    hpassed->Divide( htotal );
    return hpassed;
}

void write_ratio( TEfficiency& teff, std::string s )
{
    auto hrat = get_ratio( teff );
    hrat->Write( s.c_str() );
}

int main( int argc, char * argv[] )
{

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

    YAML::Node muv_xy_cut_node = YAML::Load( rectangles );
    MuvXYCut muv_xy_cut{ muv_xy_cut_node };


    auto proj_xy_highmom = []( const TH3D& h )
    {
        TAxis * p_axis;
        p_axis = h.GetZaxis();
        p_axis->SetRange( p_axis->FindBin( 30 ), p_axis->FindBin( 60 ) );
        return static_cast<TH2D*>(h.Project3D( "yx" ) );
    };

    auto proj_xy_full = []( const TH3D& h )
    {
        return static_cast<TH2D*>(h.Project3D( "yx" ) );
    };

    auto proj_p = []( const TH3D &h )
    {
        return static_cast<TH1D*>(h.Project3D( "z" ) );
    };

    auto xy_eff_highmom = [ proj_xy_highmom, muv_xy_cut ]( const TH3D& hrawpassed, const TH3D& hrawtotal )
    {
        auto hpassed = std::unique_ptr<TH3D>( tclone( hrawpassed ) );
        auto htotal = std::unique_ptr<TH3D>( tclone( hrawtotal ) );
        filter_hist_xy( *hpassed, muv_xy_cut );
        filter_hist_xy( *htotal, muv_xy_cut );

        TH2D * h_xy_total = proj_xy_highmom( *htotal );
        TH2D * h_xy_passed = proj_xy_highmom( *hpassed );

        TH2D * h_xy_eff = tclone( *h_xy_passed );

        h_xy_total->Sumw2();
        h_xy_eff->Sumw2();

        h_xy_eff->Divide( h_xy_total );

        return h_xy_eff;
    };

    auto xy_eff = [proj_xy_full]( const TH3D& hrawpassed, const TH3D& hrawtotal )
    {
        auto hpassed = std::unique_ptr<TH3D>( tclone( hrawpassed ) );
        auto htotal = std::unique_ptr<TH3D>( tclone( hrawtotal ) );

        TH2D * h_xy_total = proj_xy_full( *htotal );
        TH2D * h_xy_passed = proj_xy_full( *hpassed );
        TH2D * h_xy_eff = tclone( *h_xy_passed );

        h_xy_total->Sumw2();
        h_xy_eff->Sumw2();

        h_xy_eff->Divide( h_xy_total );

        return h_xy_eff;
    };

    auto p_eff = [proj_p]( const  TH3D& hrawpassed, const TH3D& hrawtotal )
    {
        auto hpassed = std::unique_ptr<TH3D>( tclone( hrawpassed ) );
        auto htotal = std::unique_ptr<TH3D>( tclone( hrawtotal ) );

        TH1D * h_p_total = proj_p( *htotal );
        TH1D * h_p_passed = proj_p( *hpassed );

        h_p_total->Sumw2();
        h_p_passed->Sumw2();

        return make_unique<TEfficiency>( *h_p_passed, *h_p_total );
    };

    auto p_eff_xycut = [ proj_p, muv_xy_cut ] ( const TH3D& hrawpassed, const TH3D& hrawtotal )
    {
        auto hpassed = std::unique_ptr<TH3D>( tclone( hrawpassed ) );
        auto htotal = std::unique_ptr<TH3D>( tclone( hrawtotal ) );
        filter_hist_xy( *hpassed, muv_xy_cut );
        filter_hist_xy( *htotal, muv_xy_cut );

        TH1D * h_p_total = proj_p( *htotal );
        TH1D * h_p_passed = proj_p( *hpassed );

        h_p_total->Sumw2();
        h_p_passed->Sumw2();

        return make_unique<TEfficiency>( *h_p_passed, *h_p_total );
    };


    //Extract raw hists
    path data_channel = "p5.data.q11t.v3";
    const std::unique_ptr<TH3D> h_dt_total = extract_3D_hist( *tfin, data_channel, "muv_eff_raw/h_xyp_total");
    const std::unique_ptr<TH3D> h_dt_passed = extract_3D_hist( *tfin, data_channel, "muv_eff_raw/h_xyp_passed");

    path mc_channel = "p5.km2.v2";
    const std::unique_ptr<TH3D> h_mc_total = extract_3D_hist( *tfin, mc_channel, "muv_eff_raw/h_xyp_total");
    const std::unique_ptr<TH3D> h_mc_passed = extract_3D_hist( *tfin, mc_channel, "muv_eff_raw/h_xyp_passed");

    //Get xy effs
    TH2D * xy_eff_dt = xy_eff_highmom( *h_dt_passed, *h_dt_total );
    TH2D * xy_eff_mc = xy_eff_highmom( *h_mc_passed, *h_mc_total );

    tfout->cd();
    xy_eff_dt->Write( "h_high_mom_xy_dt_eff" );
    xy_eff_mc->Write( "h_high_mom_xy_mc_eff" );

    //Rewieght mc xy effs
    auto h_mc_passed_rw = std::unique_ptr<TH3D>( tclone(*h_mc_passed ) );
    reweight_hist_xy( *h_mc_passed_rw, *xy_eff_dt );

    TH2D * xy_eff_mc_rw = xy_eff_highmom( *h_mc_passed_rw, *h_mc_total );
    xy_eff_mc_rw->Write( "h_high_mom_xy_mc_rw_eff" );

    //Do momentum plots
    std::unique_ptr<TEfficiency> h_dt_eff_p_xycut = p_eff_xycut( *h_dt_passed, *h_dt_total );
    h_dt_eff_p_xycut->Write( "dt_eff_p_xycut" );
    write_ratio( *h_dt_eff_p_xycut, "dt_reff_p_xycut" );

    std::unique_ptr<TEfficiency> h_mc_uw_eff_p_xycut = p_eff_xycut( *h_mc_passed, *h_mc_total );
    h_mc_uw_eff_p_xycut->Write( "mc_uw_eff_p_xycut" );
    write_ratio (*h_mc_uw_eff_p_xycut, "mc_uw_reff_p_xycut" );

    std::unique_ptr<TEfficiency> h_mc_rw_eff_p_xycut = p_eff_xycut( *h_mc_passed_rw, *h_mc_total );
    h_mc_rw_eff_p_xycut->Write( "mc_rw_eff_p_xycut" );
    write_ratio( *h_mc_rw_eff_p_xycut,  "mc_rw_eff_p_xycut" );

    //Determine momentum correction to MC
    auto hp_eff_rw_mc = get_ratio( *h_mc_rw_eff_p_xycut );
    auto hp_eff_corr = get_ratio( *h_dt_eff_p_xycut );

    hp_eff_corr->Divide( hp_eff_rw_mc.get() );
    hp_eff_corr->Scale(  1.0 /hp_eff_corr->GetMaximum() );
    hp_eff_corr->Write( "mc_p_correction" );

    if ( do_conditions )
    {
        tfcond->cd();
        hp_eff_corr->Write( "muv_eff_p" );
        tfout->cd();
    }

    //Redo xy correction using mom correction
    auto h_mc_passed_rwp = std::unique_ptr<TH3D>( tclone(*h_mc_passed ) );
    reweight_hist_p( *h_mc_passed_rwp, *hp_eff_corr );

    TH2D * xy_eff_mc_rwp = xy_eff( *h_mc_passed_rwp, *h_mc_total );
    xy_eff_mc_rwp->Write( "xy_eff_mc_rwp" );

    TH2D * xy_eff_dt_full = xy_eff( *h_dt_passed, *h_dt_total );
    xy_eff_dt_full->Write( "xy_eff_dt_full" );

    //Determine xy correction to MC
    TH2D *  xy_eff_corr = tclone( *xy_eff_dt_full );
    xy_eff_corr->Divide( xy_eff_mc_rwp );
    xy_eff_corr->Write( "mc_xy_correction" );

    if ( do_conditions )
    {
        tfcond->cd();
        xy_eff_corr->Write( "muv_eff_xy" );
        tfout->cd();
    }

    //Do final correction
    auto h_mc_passed_xypw = std::unique_ptr<TH3D>( tclone(*h_mc_passed ) );
    reweight_hist_xyp( *h_mc_passed_xypw, *xy_eff_corr, *hp_eff_corr );

    auto h_mc_total_xypw = std::unique_ptr<TH3D>( tclone( *h_mc_total ) );
    auto h_dt_passed_xypw = std::unique_ptr<TH3D>( tclone( *h_dt_passed ) );
    auto h_dt_total_xypw = std::unique_ptr<TH3D>( tclone( *h_dt_total ) );

    filter_muv_acc( *h_mc_passed_xypw );
    filter_muv_acc( *h_mc_total_xypw );
    filter_muv_acc( *h_dt_passed_xypw );
    filter_muv_acc( *h_dt_total_xypw );

    auto dt_xy_eff_muvacc = xy_eff( *h_dt_passed_xypw, *h_dt_total_xypw );
    auto mc_xy_eff_muvacc = xy_eff( *h_mc_passed_xypw, *h_mc_total_xypw );
    dt_xy_eff_muvacc->Write("dt_xy_eff_muvacc" );
    mc_xy_eff_muvacc->Write("mc_xy_eff_muvacc" );
    dt_xy_eff_muvacc->Divide( mc_xy_eff_muvacc );
    dt_xy_eff_muvacc->Write( "rat_xy_eff_muvacc" );

    auto dt_p_eff_muvacc = p_eff( *h_dt_passed_xypw, *h_dt_total_xypw );
    auto mc_p_eff_muvacc = p_eff( *h_mc_passed_xypw, *h_mc_total_xypw );
    dt_p_eff_muvacc->Write("dt_p_eff_muvacc" );
    mc_p_eff_muvacc->Write("mc_p_eff_muvacc" );

    auto dt_p_eff_muvacc_r = get_ratio( *dt_p_eff_muvacc );
    auto mc_p_eff_muvacc_r = get_ratio( *mc_p_eff_muvacc );

    dt_p_eff_muvacc_r->Divide( mc_p_eff_muvacc_r.get() );
    dt_p_eff_muvacc_r->Write( "rat_p_eff_muvacc" );
}
