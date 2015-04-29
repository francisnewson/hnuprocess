#include "ToyMC.hh"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "HistStore.hh"
#include "stl_help.hh"
#include "NA62Constants.hh"
#include "logging.hh"
#include "Counter.hh"
#include "easy_app.hh"

using namespace toymc;
using namespace fn;
namespace nc = na62const;

int main( int argc , char * argv[] )
{
    fn::logger slg;
    auto  core = boost::log::core::get();
    // Set a global filter so that only error messages are logged
    core->set_filter(boost::log::expressions::is_in_range(
                boost::log::expressions::attr< severity_level >("Severity"), 
                startup,  severity_level::maximum) );

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "number,n", po::value<Long64_t>(),  "Number of events")
        ;

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, general), vm);

    Long64_t n_events = 10000;

    //required number of events
    if ( vm.count( "number" ) )
    {
        n_events =  vm["number"].as<Long64_t>() ;
        BOOST_LOG_SEV( slg , startup) << n_events << " events requested.";
    }

    SmartRNG<std::mt19937> gen;
    ToyMCLibrary mc_lib;

    //-----------------------------BUILD SIM -----------------------------

    //materials
    double dch_budget = 4e-3; //rad lengths
    double dch_depth = 5; //cm (guess)
    double dch_rad_length = dch_depth / dch_budget;

    double drift_dch1_to_dch2 = nc::zDch2 - nc::zDch1;
    double drift_dch2_to_mag = nc::zMagnet - nc::zDch2;
    double drift_mag_to_dch3 = nc::zDch3 - nc::zMagnet;
    double drift_dch3_to_dch4 = nc::zDch4 - nc::zDch3;
    double drift_dch4_to_al = nc::zAl - nc::zDch4;
    double drift_al_to_lkr = nc::zLkr - nc::zAl;
    double drift_lkr_to_hac = nc::zHac - nc::zLkr;
    double drift_hac_to_muv1 = nc::zMuv1 - nc::zHac;
    double drift_muv1_to_muv2 = nc::zMuv2 - nc::zMuv1;

    ToyMC * scatter_dch1_to_dch2 = mc_lib.add_toy( 
            make_unique<ToyMCThickScatter> ( gen, nc::X0_helium, drift_dch1_to_dch2, 10 ) );

    ToyMC *  scatter_in_dch2 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, dch_rad_length, dch_depth, 1 ) );

    ToyMC *  scatter_dch2_to_mag = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_helium, drift_dch2_to_mag, 1 ) );

    ToyMC * bend_in_mag = mc_lib.add_toy(
            make_unique<ToyMCDipoleBend> ( nc::mpn33_kick, +1 ) );

    ToyMC * scatter_mag_to_dch3 = mc_lib.add_toy( 
            make_unique<ToyMCThickScatter> ( gen, nc::X0_helium, drift_mag_to_dch3, 10 ) );

    ToyMC *  scatter_in_dch3 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, dch_rad_length, dch_depth, 1 ) );

    ToyMC * scatter_dch3_to_dch4 = mc_lib.add_toy( 
            make_unique<ToyMCThickScatter> ( gen, nc::X0_helium, drift_dch3_to_dch4, 10 ) );

    ToyMC *  scatter_in_dch4 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, dch_rad_length, dch_depth, 1 ) );

    ToyMC * scatter_to_al_window = mc_lib.add_toy( 
            make_unique<ToyMCThickScatter> ( gen, nc::X0_helium, drift_dch4_to_al - dch_depth, 10 ) );

    ToyMC * scatter_al_window_to_lkr = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_air, drift_al_to_lkr - nc::len_lkr, 1 ) );

    ToyMC * scatter_in_lkr = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_lkr, nc::len_lkr , 10) );

    ToyMC * scatter_lkr_to_hac = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_air, drift_lkr_to_hac - nc::len_hac, 1 ) );

    ToyMC * scatter_in_hac = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_iron, nc::len_hac , 10) );

    ToyMC * scatter_hac_to_muv1 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_air, drift_hac_to_muv1 - nc::len_muv_iron, 1 ) );

    ToyMC * scatter_in_muv1 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_iron, nc::len_muv_iron , 10) );

    ToyMC * scatter_muv1_to_muv2 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_air, drift_muv1_to_muv2 - nc::len_muv_iron, 10) );

    ToyMC * scatter_in_muv2 = mc_lib.add_toy(
            make_unique<ToyMCThickScatter> ( gen, nc::X0_iron, nc::len_muv_iron , 10) );

    ToyMCComposite na62_sim{ 
        { scatter_dch1_to_dch2,
            scatter_in_dch2,
            scatter_dch2_to_mag,
            bend_in_mag,
            scatter_mag_to_dch3,
            scatter_in_dch3, 
            scatter_dch3_to_dch4, 
            scatter_in_dch4,
            scatter_to_al_window,
            scatter_al_window_to_lkr,
            scatter_in_lkr,
            scatter_lkr_to_hac,
            scatter_in_hac,
            scatter_hac_to_muv1,
            scatter_in_muv1
        } };

    ToyMCComposite muv2_sim{ {
        scatter_muv1_to_muv2,
            scatter_in_muv2 
    } };

    ToyMC * propagate_dch1_to_mag = mc_lib.add_toy(
            make_unique<ToyMCPropagate>( nc::zMagnet - nc::zDch1) );

    ToyMC * propagate_mag_to_muv1 = mc_lib.add_toy(
            make_unique<ToyMCPropagate>( nc::zMuv1 - nc::zMagnet) );

    ToyMC * propagate_muv1_to_muv2 = mc_lib.add_toy(
            make_unique<ToyMCPropagate>( nc::zMuv2 - nc::zMuv1) );

    ToyMCComposite extrapolate_to_muv1{
        {
            propagate_dch1_to_mag,
                bend_in_mag,
                propagate_mag_to_muv1
        }};


    //----------------------------INIT OUTPUT -----------------------------//
    HistStore hs;
    auto h_tx = hs.MakeTH1D(  "h_tx", "tx after scattering", 1000, 1, 0, "tx" );
    auto h_x = hs.MakeTH1D(  "h_x", "dx after scattering", 1000, 1, 0, "x" );

    auto h_pr = hs.MakeTH2D(  "h_pr", "p vs r after scattering", 
            100, 0, 100, "r", 100, 0, 100, "p");

    double z = 0;


    std::uniform_real_distribution<double> mom_dist( 5, 65 );
    std::uniform_real_distribution<double> pos_dist( -20, 20 );

    //------------------------------RUN SIM ------------------------------//
    Counter counter( slg, n_events );

    for ( int i = 0 ; i != n_events ; ++ i )
    { 
        counter.new_event();

        double mom =  mom_dist( gen );
        double x = pos_dist( gen );
        double y = pos_dist( gen );

        track_params initial_params{ mom, +1, x, 0, y, 0, nc::zDch1 };
        track_params params_after_muv1 = na62_sim.transfer( initial_params );
        track_params params_after_muv2 = muv2_sim.transfer( params_after_muv1 );

        double real_muv_y = params_after_muv1.y;
        double real_muv_x = params_after_muv2.x;

        double muv_x = real_muv_x - fmod( real_muv_x, nc::muv_half_width );
        double muv_y = real_muv_y - fmod( real_muv_y, nc::muv_half_width );

        track_params extrap_after_muv1 = extrapolate_to_muv1.transfer( initial_params );
        track_params extrap_after_muv2 = propagate_muv1_to_muv2->transfer( extrap_after_muv1 );

        double extrap_x = extrap_after_muv2.x;
        double extrap_y = extrap_after_muv1.y;

        double dx = muv_x - extrap_x;
        double dy = muv_y - extrap_y;

        BOOST_LOG_SEV( slg, severity_level::debug )
            << muv_x << " " << muv_y << " " 
            << x << " "<<  y << " " 
            << dx << " " << dy << " " ;

        h_tx->Fill( params_after_muv2.ty);
        h_x->Fill( dx - x);
        h_pr->Fill( std::hypot( dy, dx ), mom );

        z+= params_after_muv2.z;
    }

    TFile tfout{ "output/test_toy_mc.root", "RECREATE" };
    hs.Write();

    std::cout << "Final Z: " << z / double(n_events) << std::endl;
}
