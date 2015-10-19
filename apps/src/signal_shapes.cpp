#include "easy_app.hh"
#include "root_help.hh"
#include "stl_help.hh"
#include "HistExtractor.hh"
#include "fiducial_functions.hh"
#include "TFile.h"
#include "Limiter.hh"
#include <cmath>
#include <numeric>
#include <iomanip>
#include <algorithm>
#include <boost/io/ios_state.hpp>
#include <boost/regex.hpp>
#include <boost/circular_buffer.hpp>

//print individual result to screen (human readable)
void print_result( std::ostream& os, const HnuLimParams& hlp, const HnuLimResult& hlr )
{
    {
        boost::io::ios_flags_saver fs( os );
        os.setf(std::ios::fixed, std::ios::floatfield);
        os.precision(4);

        os << "===========================================\n";
        os << hlp.get_chan() << "\n";
        os << "-------------------------------------------\n";
        os << "Kaon flux: " << std::scientific << hlr.kaon_flux << "\n";
        os.unsetf(std::ios_base::floatfield);
        os << "Mean: " << hlp.get_mass() << " MeV/c^2 -> " << hlp.get_mean() << " GeV^2/c^4\n";
        os << "Width: " << hlp.get_width() << "\n";
        os << "Acc: " << hlp.get_full_acceptance() << " ->  " 
            << hlp.get_width_acceptance() << " ± " << hlp.get_width_acceptance_err() << "\n";
        os << "-------------------------------------------\n";
        os << "Trig eff (raw error): " << hlr.trig_eff << " ± " << hlr.raw_trig_err << "\n";
        os << "Sig range: " << hlr.sig_min << " - " << hlr.sig_max << "\n";
        os << "Background: " << hlr.background << " ± " << hlr.background_err << "\n" ;
        os << "sqrt(Background): " << std::sqrt(hlr.background ) << "\n" ;
        os << "Data observed: " << hlr.dt_obs << "\n";
        os << "Acc Sig limit: " << hlr.acc_sig_ul << "\n";
        os << "Orig Sig limit: " << hlr.orig_sig_ul << " " << hlr.rolke_orig_sig_ul << "\n";
        os.setf(std::ios_base::scientific, std::ios_base::floatfield);
        os << "BR limit: " << hlr.ul_br  << " U2 limit: " << hlr.ul_u2 << "\n";
        os << "Dt BR limit: " <<  hlr.dt_ul_br << " Dt U2 limit: " << hlr.dt_ul_u2 <<  "\n";
    }

    //Determine longest errname
    std::vector<std::string> names;
    std::vector<double> lengths;
    std::vector<double> values;
    const auto& err_budget = hlr.error_budget;
    for ( const auto& p : err_budget )
    {
        names.push_back( p.first );
        lengths.push_back( p.first.size() );
        values.push_back( p.second );
    }
    double max_length = *std::max_element( begin( lengths ), end( lengths ) );
    for ( const auto & name : names )
    { os << std::setw( max_length + 2 ) << name ; }
    os << "\n";

    for ( const auto & val : values )
    { os << std::setw( max_length + 2 ) << std::sqrt(val) ; }

    os << "\n";
}

//headings for writing to file (script readable)
void print_headings( std::ostream& os )
{
    os 
        << std::setw(20) << "channel"
        << std::setw(15) << "mass"
        << std::setw(15) << "mean"
        << std::setw(15) << "sqrt(mean)"
        << std::setw(15) << "full_acc"
        << std::setw(15) << "width"
        << std::setw(15) << "final_acc"
        << std::setw(15) << "final_acc_err"
        << std::setw(15) << "trig_eff"
        << std::setw(15) << "raw_trig_err"
        << std::setw(15) << "raw_trig_sqerr"
        << std::setw(15) << "bg"
        << std::setw(15) << "bg_err"
        << std::setw(15) << "dt_obs"
        << std::setw(15) << "ul_acc_sig"
        << std::setw(15) << "ul_orig_sig"

        << std::setw(15) << "ul_mu"
        << std::setw(15) << "ul_br"
        << std::setw(15) << "ul_u2"

        << std::setw(15) << "ses_br"
        << std::setw(15) << "ses_u2"

        << std::setw(15) <<  "comb_sigma"
        << std::setw(15) <<  "dt_pull"

        << std::setw(15) << "high1_ul_mu"
        << std::setw(15) << "high1_ul_br"
        << std::setw(15) << "high1_ul_u2"

        << std::setw(15) << "low1_ul_mu"
        << std::setw(15) << "low1_ul_br"
        << std::setw(15) << "low1_ul_u2"

        << std::setw(15) << "high2_ul_mu"
        << std::setw(15) << "high2_ul_br"
        << std::setw(15) << "high2_ul_u2"

        << std::setw(15) << "low2_ul_mu"
        << std::setw(15) << "low2_ul_br"
        << std::setw(15) << "low2_ul_u2"

        << std::setw(15) << "high3_ul_mu"
        << std::setw(15) << "high3_ul_br"
        << std::setw(15) << "high3_ul_u2"

        << std::setw(15) << "low3_ul_mu"
        << std::setw(15) << "low3_ul_br"
        << std::setw(15) << "low3_ul_u2"

        << std::setw(15) << "dt_ul_mu"
        << std::setw(15) << "dt_ul_br"
        << std::setw(15) << "dt_ul_u2"

        << std::setw(15) << "dt_ll_mu"
        << std::setw(15) << "dt_ll_br"
        << std::setw(15) << "dt_ll_u2"

        << std::setw(15) << "err_trig"
        << std::setw(15) << "err_scat"
        << std::setw(15) << "err_muv"
        << std::setw(15) << "err_flux"
        << std::setw(15) << "err_hscale"
        << std::setw(15) << "err_hval"
        << std::setw(15) << "err_k3pi"
        << std::setw(15) << "err_hshape"
        << std::endl;
}

//writing to file (script readable)
void print_flat_result( std::ostream& os, const HnuLimParams& hlp, const HnuLimResult& hlr )
{
    boost::io::ios_flags_saver fs( os );
    os.setf(std::ios::fixed, std::ios::floatfield);
    os.precision(5);

    os 
        << std::setw(20) << hlp.get_chan()
        << std::setw(15) << hlp.get_mass()
        << std::setw(15) << hlp.get_mean()
        << std::setw(15) << std::sqrt(hlp.get_mean() )
        << std::setw(15) << hlp.get_full_acceptance()
        << std::setw(15) << hlp.get_width()
        << std::setw(15) << hlp.get_width_acceptance()
        << std::setw(15) << hlp.get_width_acceptance_err()
        << std::setw(15) << hlr.trig_eff
        << std::setw(15) << hlr.raw_trig_err
        << std::setw(15) << hlr.raw_trig_sqerr
        << std::setw(15) << hlr.background
        << std::setw(15) << hlr.background_err
        << std::setw(15) << hlr.dt_obs;

    os.setf(std::ios_base::scientific,
            std::ios_base::floatfield);

    os
        << std::setw(15) <<  hlr.acc_sig_ul
        << std::setw(15) <<  hlr.orig_sig_ul

        << std::setw(15) <<  hlr.ul_mu
        << std::setw(15) <<  hlr.ul_br
        << std::setw(15) <<  hlr.ul_u2

        << std::setw(15) <<  hlr.ses_br
        << std::setw(15) <<  hlr.ses_u2

        << std::setw(15) <<  hlr.comb_sigma
        << std::setw(15) <<  hlr.dt_pull

        << std::setw(15) <<  hlr.high1_ul_mu
        << std::setw(15) <<  hlr.high1_ul_br
        << std::setw(15) <<  hlr.high1_ul_u2

        << std::setw(15) <<  hlr.low1_ul_mu
        << std::setw(15) <<  hlr.low1_ul_br
        << std::setw(15) <<  hlr.low1_ul_u2

        << std::setw(15) <<  hlr.high2_ul_mu
        << std::setw(15) <<  hlr.high2_ul_br
        << std::setw(15) <<  hlr.high2_ul_u2

        << std::setw(15) <<  hlr.low2_ul_mu
        << std::setw(15) <<  hlr.low2_ul_br
        << std::setw(15) <<  hlr.low2_ul_u2

        << std::setw(15) <<  hlr.high3_ul_mu
        << std::setw(15) <<  hlr.high3_ul_br
        << std::setw(15) <<  hlr.high3_ul_u2

        << std::setw(15) <<  hlr.low3_ul_mu
        << std::setw(15) <<  hlr.low3_ul_br
        << std::setw(15) <<  hlr.low3_ul_u2

        << std::setw(15) <<  hlr.dt_ul_mu
        << std::setw(15) <<  hlr.dt_ul_br
        << std::setw(15) <<  hlr.dt_ul_u2

        << std::setw(15) <<  hlr.dt_ll_mu
        << std::setw(15) <<  hlr.dt_ll_br
        << std::setw(15) <<  hlr.dt_ll_u2

        << std::setw(15) <<  hlr.error_budget.at("err_trig")
        << std::setw(15) <<  hlr.error_budget.at("err_scat")
        << std::setw(15) <<  hlr.error_budget.at("err_muv")
        << std::setw(15) <<  hlr.error_budget.at("err_flux")
        << std::setw(15) <<  hlr.error_budget.at("err_hscale")
        << std::setw(15) <<  hlr.error_budget.at("err_hval")
        << std::setw(15) <<  hlr.error_budget.at("err_k3pi")
        << std::setw(15) <<  hlr.error_budget.at("err_hshape")
        << std::endl;
}

int main()
{
    //**************************************************
    //Signal shapes
    //**************************************************


    //std::string keystring = "loose_";
    //std::string output = "loose_lim";

    std::string keystring = "";
    std::string output = "tgt_lim";

    auto key = keystring.c_str();
    auto out = output.c_str();


    //Decide which masses to look at
    std::vector<int> masses;
    for( int m = 250 ; m != 380 ; m += 1 )
    { masses.push_back( m ); }

    //Load acceptance plots
    std::string acc_filename = "tdata/km2_acc/all.km2_acc.root" ;
    auto fid_weights = extract_fiducial_weights
        ( acc_filename, "", "sample_burst_count/bursts", "weight"  );

    //std::vector<std::string> acc_regions { "signal_upper_loose_plots", "signal_lower_loose_plots" };
    std::vector<std::string> acc_regions { Form("signal_upper_%splots", key), Form("signal_lower_%splots", key) };

    //set up acceptance files
    TFile tfin_acc {acc_filename.c_str()};
    TFile tfout_acc{ "output/signal_shapes.root", "RECREATE" };

    //**************************************************
    //Background estimates
    //**************************************************

    //Decide which backgrounds to consider
    std::vector<std::string> background_channels
    { "halo", "k3pi", "k2pi", "k3pi0", "km3", "km2" };

    //Load background file
    //std::string bg_filename = "tdata/staging/all.mass_plots.root";
    std::string bg_filename = "tdata/staging/full/all.shuffle_plots.root";
    TFile tfin_bg{ bg_filename.c_str() };
    std::vector<std::string> signal_regions  = acc_regions;

    Limiter lm{ tfin_bg, signal_regions };
    lm.set_bg_channels( background_channels);

    //Trigger corrections
    std::string trig_filename = "tdata/staging/full/all.shuffle_plots.root" ;
    TFile tfin_trig{ trig_filename.c_str() };

    TriggerApp trig( tfin_trig );
    trig.set_num_denom( "htrig_num", "htrig_denom" );
    trig.set_sels( { 
            Form("pos/signal_upper_%splots/h_m2m_kmu", key ),
            Form("neg/signal_upper_%splots/h_m2m_kmu", key ),
            Form("pos/signal_lower_%splots/h_m2m_kmu", key ),
            Form("neg/signal_lower_%splots/h_m2m_kmu", key ),
            //"pos/signal_upper_loose_plots/h_m2m_kmu",
            //"neg/signal_upper_loose_plots/h_m2m_kmu",
            //"pos/signal_lower_loose_plots/h_m2m_kmu",
            //"neg/signal_lower_loose_plots/h_m2m_kmu"
            //"pos/signal_upper_muv/h_m2m_kmu",
            //"neg/signal_upper_muv/h_m2m_kmu",
            //"pos/signal_lower_muv/h_m2m_kmu",
            //"neg/signal_lower_muv/h_m2m_kmu",
            } );
    trig.init();

    lm.set_trigger( trig );

    //Scattering
    TFile tfin_scat { "tdata/staging/km2_scat.root"};
    TFile tfin_noscat { "tdata/staging/km2_noscat.root"};
    ScatterContrib sc{ tfin_noscat, tfin_scat };

    lm.set_scatter_contrib( sc );

    //halo errors
    //TFile tf_halo_log{  "tdata/staging/log/halo_sub_log.q11t.root"  };
    TFile tf_halo_log{  "tdata/staging/full/log_halo_sub.root" };

    HaloErrors he{ tfin_bg, tf_halo_log };
    he.set_halo_info( {
            { Form("pos/signal_upper_%splots/h_m2m_kmu/hnu_stack_hists/halo_pos", key ), "scalefactors/pos_ext_upper", Form("p6.halo.q11t.pos/signal_upper_%splots/h_m2m_kmu", key) },
            { Form("neg/signal_upper_%splots/h_m2m_kmu/hnu_stack_hists/halo_neg", key ), "scalefactors/neg_ext_upper", Form("p6.halo.q11t.neg/signal_upper_%splots/h_m2m_kmu", key) },
            { Form("pos/signal_lower_%splots/h_m2m_kmu/hnu_stack_hists/halo_pos", key ), "scalefactors/pos_ext_lower", Form("p6.halo.q11t.pos/signal_lower_%splots/h_m2m_kmu", key) },
            { Form("neg/signal_lower_%splots/h_m2m_kmu/hnu_stack_hists/halo_neg", key ), "scalefactors/neg_ext_lower", Form("p6.halo.q11t.neg/signal_lower_%splots/h_m2m_kmu", key) }
            } );

#if 0
    he.set_halo_info( {
            { "pos/signal_upper_loose_plots/h_m2m_kmu/hnu_stack_hists/halo_pos", "scalefactors/pos_ext_upper", "p6.halo.q11t.pos/signal_upper_loose_plots/h_m2m_kmu" },
            { "neg/signal_upper_loose_plots/h_m2m_kmu/hnu_stack_hists/halo_neg", "scalefactors/neg_ext_upper", "p6.halo.q11t.neg/signal_upper_loose_plots/h_m2m_kmu" },
            { "pos/signal_lower_loose_plots/h_m2m_kmu/hnu_stack_hists/halo_pos", "scalefactors/pos_ext_lower", "p6.halo.q11t.pos/signal_lower_loose_plots/h_m2m_kmu" },
            { "neg/signal_lower_loose_plots/h_m2m_kmu/hnu_stack_hists/halo_neg", "scalefactors/neg_ext_lower", "p6.halo.q11t.neg/signal_lower_loose_plots/h_m2m_kmu" }
            { "neg/signal_lower_muv/h_m2m_kmu/hnu_stack_hists/halo_neg", "neg_lower", "p6.halo.q11t.neg/signal_lower_muv_plots/h_m2m_kmu" },
            { "pos/signal_lower_muv/h_m2m_kmu/hnu_stack_hists/halo_pos", "pos_lower", "p6.halo.q11t.pos/signal_lower_muv_plots/h_m2m_kmu" } ,
            { "neg/signal_upper_muv/h_m2m_kmu/hnu_stack_hists/halo_neg", "neg_upper", "p6.halo.q11t.neg/signal_upper_muv_plots/h_m2m_kmu" },
            { "pos/signal_upper_muv/h_m2m_kmu/hnu_stack_hists/halo_pos", "pos_upper", "p6.halo.q11t.pos/signal_upper_muv_plots/h_m2m_kmu" } } );
#endif
    he.set_shape_error_factor( 0.150 );

    lm.set_halo_errors( he );

    //Output streams
    std::ofstream ofs( Form("output/%s.txt", out ) );
    print_headings( ofs );
    std::ostream& os = std::cout;

    bool do_default = true;
    bool do_width_scan = false;

    for ( const auto& mass : masses )
    {
        std::cerr << mass << std::endl;

        if ( do_default )
        {
            try
            {
                //Set up signal parameters
                HnuLimParams hlp( tfin_acc, fid_weights, acc_regions, mass, 1.0 );

                //write acceptance plots
                tfout_acc.cd();
                hlp.write();

                //Get limits
                auto limres = lm.get_limit( hlp );

                //Print limit results
                print_result( os, hlp, limres );
                print_flat_result( ofs, hlp, limres );
            }
            catch (std::exception & e)
            {
                std::cerr << "Problem with " << mass << e.what() << std::endl;
                continue;
            }
        }

        if ( do_width_scan )
        {
            try
            {
                double last_lim = 1;
                int grad_buf_length = 3;
                boost::circular_buffer<double> grad_buf( grad_buf_length );
                for ( int i = 0 ; i != grad_buf_length ; ++i )
                { grad_buf.push_back( -1 ); }

                //Set up signal parameters
                for ( int nsig  = 50 ; nsig != 250 ; nsig += 10 )
                {
                    HnuLimParams hlp( tfin_acc, fid_weights, acc_regions, mass, 0.01 * nsig );

                    //Get limits
                    auto limres = lm.get_limit( hlp );
                    double new_lim = limres.ul_u2;
                    double grad = new_lim - last_lim;

                    std::cout 
                        << std::setw(10) << mass 
                        << std::setw(20) << 0.01 * nsig 
                        << std::setw(20) << limres.ul_u2 
                        << std::setw(20) << std::setprecision(10)  << hlp.get_width()
                        << "\n";

                    grad_buf.push_back( grad );

                    if ( std::all_of( begin( grad_buf ), end( grad_buf ), []( double grad ){ return grad > 0 ; } ) )
                    {
                        break;
                    }

                    last_lim = new_lim;
                }
            }
            catch (std::exception & e)
            {
                std::cerr << "Problem with " << mass << e.what() << std::endl;
                continue;
            }
        }
    }
}
