#include "DchPlotter.hh"
#include "k2pi_fitting.hh"
#include "K2piEventData.hh"
#include "K2piInterface.hh"
#include "root_help.hh"
#include <iostream>
#include <boost/filesystem/path.hpp>
#include "k2pi_reco_functions.hh"
#include "NA62Constants.hh"

using boost::filesystem::path;

namespace fn
{
    DchPlotter::DchPlotter( TFile& tf, std::string folder)
        :tf_( tf ), folder_( folder )
    {
        hm2pip_lkr_ = dths_.MakeTH1D( "hm2pip_lkr", "Pi+ mass from Lkr",
                1000, -0.3, 0.2, "m^{2}_{miss}", "#events" );

        hpt_ = dths_.MakeTH1D( "hpt_", "Pi+ p_{T}",
                1000, 0.0, 0.5,  "p_{T} ( GeV/c )", "#events" );
    }

    void DchPlotter::plot_data
        ( K2piEventData& event_data,
          K2piLkrData& lkr_data, K2piDchData& dch_data,
          double weight, bool mc, K2piMcData * mc_data )
        {
            //Copy data into parameter array
            k2pi_params lkr_param_data;
            K2piLkrInterface lkr_interface( lkr_data);
            copy( lkr_interface, lkr_param_data );

            //Extract 'fit' result
            k2pi_fit data_fit;
            compute_pion_mass( lkr_param_data, data_fit );

            //Kaon 4mom
            TVector3 kaon_3mom {lkr_data.pkx, lkr_data.pky, lkr_data.pkz };
            double kaon_energy = std::hypot( na62const::mK, kaon_3mom.Mag() );
            TLorentzVector kaon_4mom { kaon_3mom, kaon_energy };

            //Lkr 4mom
            TLorentzVector& lkr_pip_4mom = data_fit.pip;
            TLorentzVector& lkr_pi0_4mom = data_fit.pi0;
            hm2pip_lkr_->Fill( lkr_pip_4mom.M2(), weight);

            //Dch 4mom
            K2piDchInterface dch_interface( dch_data);
            TLorentzVector dch_pip_4mom =  dch_interface.get_pip_4mom();

            //Lkr Dch comparison
            lkr_dch_cmp_.Fill( lkr_pip_4mom, dch_pip_4mom, weight );
            lkr_dch_cmp_.FillM2( lkr_pip_4mom, dch_pip_4mom, kaon_4mom, weight );

            uw_lkr_dch_cmp_.Fill( lkr_pip_4mom, dch_pip_4mom, 1.0);
            uw_lkr_dch_cmp_.FillM2( lkr_pip_4mom, dch_pip_4mom, kaon_4mom, 1.0);

            TVector3 kaon_mom( lkr_interface.pK_X(), lkr_interface.pK_Y(), lkr_interface.pK_Z() );
            TVector3 dch_pip_3mom = dch_pip_4mom.Vect();
            hpt_->Fill( dch_pip_3mom.Perp( kaon_mom ), weight );

            if ( !mc )
            { return ; }

            //--------------------MC Only--------------------//

            K2piMcInterface mc_interface( *mc_data );
            const TLorentzVector& mc_pip_4mom = mc_interface.p4pip();
            mc_dch_cmp_.Fill(  mc_pip_4mom, dch_pip_4mom , weight );
            mc_dch_cmp_.FillM2(  mc_pip_4mom, dch_pip_4mom, kaon_4mom, weight );

            lkr_mc_cmp_.Fill(  lkr_pip_4mom, mc_pip_4mom , weight );
            lkr_mc_cmp_.FillM2(  lkr_pip_4mom, mc_pip_4mom , kaon_4mom,  weight );

            const TLorentzVector& mc_k_4mom = mc_interface.p4k();

            lkr_tk_mc_cmp_.Fill( mc_k_4mom - lkr_pi0_4mom , mc_pip_4mom , weight );

            lkr_tk_mc_cmp_.FillM2( mc_k_4mom - lkr_pi0_4mom , mc_pip_4mom , kaon_4mom,  weight );
#if 0
            mc_pip_4mom.Print();
            dch_pip_4mom.Print();
            lkr_pip_4mom.Print();
#endif

        }


    void DchPlotter::write()
    {
        cd_p( &tf_, folder_ );
        dths_.Write();

        lkr_dch_cmp_.SetColor( kBlue+2 );
        cd_p( &tf_, path{ folder_} / "lkr_dch_cmp" );
        lkr_dch_cmp_.Write();

        uw_lkr_dch_cmp_.SetColor( kBlue+2 );
        cd_p( &tf_, path{ folder_} / "uw_lkr_dch_cmp" );
        uw_lkr_dch_cmp_.Write();

        lkr_mc_cmp_.SetColor( kGreen+2 );
        cd_p( &tf_, path{ folder_} / "lkr_mc_cmp" );
        lkr_mc_cmp_.Write();

        lkr_tk_mc_cmp_.SetColor( kSpring+2 );
        cd_p( &tf_, path{ folder_} / "lkr_tk_mc_cmp" );
        lkr_tk_mc_cmp_.Write();

        mc_dch_cmp_.SetColor( kOrange +2 );
        cd_p( &tf_, path{ folder_} / "mc_dch_cmp" );
        mc_dch_cmp_.Write();
    }

    //--------------------------------------------------

    K2piEventPlotter::K2piEventPlotter( TFile& tf, std::string folder)
        :tf_( tf ), folder_( folder )
    {
        heop_ = dths_.MakeTH1D( "heop", "Track Cluster E/p",
                1500, 0.0 , 1.5,  "E/P", "#events" );

        hchi2_ = dths_.MakeTH1D( "hchi2", "Fit Chi2",
                10000, 0.0 , 10,  "Chi2", "#events" );

        hphoton_sep_ = dths_.MakeTH1D( "hphoton_sep", "Photon Cluster Separation",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

        htrack_cluster_sep_ = dths_.MakeTH1D( "htrack_cluster_sep", "Track Cluster - Photon Cluster Separation",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

        hmin_photon_radius_ = dths_.MakeTH1D( "hmin_photon_radius", "Min photon radius at DCH1",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

    }

    void K2piEventPlotter::plot_data( K2piEventData& event_data,
            K2piLkrData& lkr_data, K2piDchData& dch_data, 
            double weight, bool mc, K2piMcData * mc_data  )
    {
        if (event_data.found_track_cluster)
        {
            heop_->Fill( extract_eop( event_data, dch_data, mc) , weight );
        }

        hphoton_sep_->Fill( extract_photon_sep( lkr_data), weight );
        htrack_cluster_sep_->Fill( extract_min_track_cluster_sep( lkr_data, dch_data), weight );
        hmin_photon_radius_->Fill( extract_min_photon_radius( lkr_data), weight );
        hchi2_->Fill( event_data.lkr_fit_chi2, weight );
    }

    void K2piEventPlotter::write()
    {
        cd_p( &tf_, folder_);
        dths_.Write();
    }


    //--------------------------------------------------

    DchAnalysis::DchAnalysis( Selection& sel,  TFile& tf_,
            std::string folder_, K2piEventData& k2pi_data, 
            std::string lkr_data_source, bool is_mc)
        :Analysis( sel ),
        plots_( tf_, folder_ ),event_plots_( tf_, folder_ ),
        k2pi_data_( k2pi_data), is_mc_( is_mc )
    {

        if ( lkr_data_source == "fit" )
        {
            lkr_data_ = &k2pi_data_.fit_lkr;
        }
        else if ( lkr_data_source == "raw" )
        {
            lkr_data_ = &k2pi_data_.raw_lkr;
        }
        else
        {
            throw std::runtime_error
                ( "DchAnalysis: Unknown lkr source " + lkr_data_source );
        }


        for ( int i = 0 ; i != 11 ; ++ i )
        {
            double angle_sigma = 0.005;
            double angle_frequency = 3e-3 * ( (2.0*i) / 10 );

            double mom_sigma = ( i < 6 ) ? 0.2 : 0.5;
            double mom_frequency = 2e-2 * i;

            if ( i > 5 )
            {
                mom_frequency = 2e-2 * ( i - 5 );
            }

            scatterers_.push_back( TrackScatterer( 
                        angle_sigma, angle_frequency,
                        mom_sigma, mom_frequency ) );

            path folder{ folder_};
            folder /= ("scatter" + std::to_string( i ) );

            scatter_plots_.push_back( DchPlotter( tf_, folder.string() ) );
        }
    }

    void DchAnalysis::process_event()
    {
        if( is_mc_ )
        {
            plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, 
                    k2pi_data_.weight, true, &k2pi_data_.mc  );

            event_plots_.plot_data( k2pi_data_, *lkr_data_ , k2pi_data_.raw_dch, 
                    k2pi_data_.weight, true, &k2pi_data_.mc  );

            for ( int i  = 0 ; i != 11 ; ++i )
            {
                K2piDchData mod_dch = k2pi_data_.raw_dch;
                scatterers_[i].scatter_track( k2pi_data_.compact_number, 
                        mod_dch.dxdz, mod_dch.dydz, mod_dch.p );

                scatter_plots_[i].plot_data( k2pi_data_, *lkr_data_, mod_dch, 
                        k2pi_data_.weight, true, &k2pi_data_.mc );
            }
        }
        else
        {
            plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, 
                    k2pi_data_.weight, false, 0 );

            event_plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, 
                    k2pi_data_.weight, false, 0  );
        }
    }

    void DchAnalysis::end_processing()
    {
        plots_.write();
        event_plots_.write();

        for ( auto& plots :  scatter_plots_)
        {
            plots.write();
        }
    }
}
