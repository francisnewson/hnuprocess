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

        hcda_ = dths_.MakeTH1D( "hcda", "DCH CDA",
                1000, 0.0, 10.0, "Cda( cm )", "#events" );

        hz_ = dths_.MakeTH1D( "hz", "DCH z",
                1200, -3000, 9000, "z( cm )", "#events" );

        hdm2_pi0_ = dths_.MakeTH1D( "hdm2_pi0", "DCH pi0 mass difference",
                1000, -0.2, 0.2, "dm^{2}", "#events" );

        hm2_tx_ = dths_.MakeTH2D( "hm2_tx_", "Mass angle correlation" ,
                100, -0.3, 0.2, "m^{2}_{miss}",
                100, -0.01, 0.01, "dtx" );

        hm2_ty_ = dths_.MakeTH2D( "hm2_ty_", "Mass angle correlation" ,
                100, -0.3, 0.2, "m^{2}_{miss}",
                100, -0.01, 0.01, "dty" );
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

            //Other kinematics ( pt, cda )
            TVector3 kaon_mom( lkr_interface.pK_X(), lkr_interface.pK_Y(), lkr_interface.pK_Z() );
            TVector3 dch_pip_3mom = dch_pip_4mom.Vect();
            Vertex vertex = extract_vertex( dch_data, lkr_data );

            hpt_->Fill( dch_pip_3mom.Perp( kaon_mom ), weight );
            hcda_->Fill( vertex.cda , weight );
            hz_->Fill( vertex.point.Z() , weight );

            //m2m
            TVector3 lkr3mom = lkr_pip_4mom.Vect();
            TVector3 dch3mom = dch_pip_4mom.Vect();
            double dtx = lkr3mom.X()/lkr3mom.Mag()  - dch3mom.X()/dch3mom.Mag();
            double dty = lkr3mom.Y()/lkr3mom.Mag()  - dch3mom.Y()/dch3mom.Mag();
            double dm2 = (kaon_4mom - lkr_pip_4mom).M2() - (kaon_4mom - dch_pip_4mom).M2();

            double dm2_pi0 = (kaon_4mom - dch_pip_4mom ).M2() - na62const::mPi0 *na62const::mPi0;
            hdm2_pi0_->Fill( dm2_pi0, weight);

            hm2_tx_->Fill( dm2, dtx, weight );
            hm2_ty_->Fill( dm2, dty, weight );


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
        }


    void DchPlotter::write()
    {
        //std::cout << "Switching to folder " << folder_ << std::endl;
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

        heop_p_ = dths_.MakeTH2D( "heop_p", "Track Cluster E/p vs p",
                100, 0.0 , 100,  "p",
                150, 0.0 , 2.0,  "E/p" );

        hchi2_ = dths_.MakeTH1D( "hchi2", "Fit Chi2",
                10000, 0.0 , 10,  "Chi2", "#events" );

        hphoton_sep_ = dths_.MakeTH1D( "hphoton_sep", "Photon Cluster Separation",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

        htrack_cluster_sep_ = dths_.MakeTH1D( "htrack_cluster_sep", "Track Cluster - Photon Cluster Separation",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

        htrack_track_cluster_sep_ = dths_.MakeTH1D( "htrack_track_cluster_sep", "Track - Track Cluster",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

        htrack_cluster_E_sep_ = dths_.MakeTH2D( "htrack_cluster_E_sep", "Track Cluster E and sep",
                100, 0.0 , 200,  "sep (cm)",
                100, 0.0 , 100,  "E (GeV)" );

        hmin_photon_radius_ = dths_.MakeTH1D( "hmin_photon_radius", "Min photon radius at DCH1",
                1000, 0.0 , 200,  "Sep ( cm )", "#events" );

        hevent_pt_ = dths_.MakeTH1D( "hevent_pt_", "Event pt",
                1000, 0.0 , 1,  "p_T", "#events" );

        hphoton_energy_ = dths_.MakeTH2D( "hphoton_energy", "Photon energies",
                100, 0.0 , 100,  "E1", 100, 0.0 , 100,  "E2" );

        hhigh_photon_energy_ = dths_.MakeTH1D( "hhigh_photon_energy", "High Photon energy",
                1000, 0.0 , 100,  "Energy(GeV)", "#events" );

        hlow_photon_energy_ = dths_.MakeTH1D( "hlow_photon_energy", "High Photon energy",
                1000, 0.0 , 100,  "Energy(GeV)", "#events" );

    }

    void K2piEventPlotter::plot_data( K2piEventData& event_data,
            K2piLkrData& lkr_data, K2piDchData& dch_data, 
            double weight, bool mc, K2piMcData * mc_data  )
    {
        if (event_data.found_track_cluster)
        {
            heop_->Fill( extract_eop( event_data, dch_data, mc) , weight );
            heop_p_->Fill( dch_data.p, extract_eop( event_data, dch_data, mc)
                    , weight );

        double track_track_cluster_sep = extract_track_track_cluster_sep( event_data, dch_data, mc);
            htrack_track_cluster_sep_->Fill( track_track_cluster_sep , weight );
        htrack_cluster_E_sep_->Fill( track_track_cluster_sep, event_data.TCE, weight );
        }

        double photon_sep =  extract_photon_sep( lkr_data);
        double min_track_cluster_sep = extract_min_track_cluster_sep( lkr_data, dch_data);
        double min_photon_radius = extract_min_photon_radius( lkr_data);
        double event_pt = extract_event_pt(  dch_data, lkr_data, mc );

        hphoton_sep_->Fill( photon_sep, weight );
        htrack_cluster_sep_->Fill( min_track_cluster_sep, weight );
        hmin_photon_radius_->Fill(  min_photon_radius, weight );

        hevent_pt_->Fill( event_pt, weight );

        hchi2_->Fill( event_data.lkr_fit_chi2, weight );
        hhigh_photon_energy_->Fill( lkr_data.E1, weight );
        hlow_photon_energy_->Fill( lkr_data.E2, weight );

        hphoton_energy_->Fill( lkr_data.E1, lkr_data.E2, weight );
    }

    void K2piEventPlotter::write()
    {
        cd_p( &tf_, folder_);
        dths_.Write();
    }

    //--------------------------------------------------

    bool DchSelection::check_data( K2piDchData * dch_data, K2piLkrData * lkr_data )
    {
        return checker_( dch_data, lkr_data );
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

        n_scatterers_ = 0;


        for ( int i = 0 ; i != n_scatterers_ ; ++ i )
        {
            double angle_cutoff = 0.00085;
            double angle_frequency = (i == 0) ? 0 : 2 * i * 0.001;
            //double angle_frequency = (i == 0) ? 0 : 1 * 1 * 0.01;

            double mom_cutoff = 0.08;
            //double mom_frequency = 1 * i * 0.001;
            //double mom_frequency = 1 * i * 0.001;
            double mom_frequency = (i == 0) ? 0 : 1 * 4 * 0.001;

            std::cout << "SCATTER " << i << std::endl;

            std::cout << "angle_cutoff: " << angle_cutoff << "\n";
            std::cout << "angle_frequency: " << angle_frequency << "\n";

            std::cout << "mom_cutoff: " << mom_cutoff << "\n";
            std::cout << "mom_frequency: " << mom_frequency << "\n";

            scatterers_.push_back( TrackPowerScatterer( 
                        angle_cutoff, angle_frequency,
                        mom_cutoff, mom_frequency ) );

            path folder{ folder_};
            folder /= ("scatter" + std::to_string( i ) );

            scatter_plots_.push_back( DchPlotter( tf_, folder.string() ) );
        }
    }

    void DchAnalysis::add_dch_selection( DchSelection* dch_selection )
    {
        dch_selections_.push_back( dch_selection );
    }

    bool check_dch_selections( std::vector<DchSelection*> selections, 
            K2piDchData * dch_data, K2piLkrData * lkr_data )
    {
        for ( auto dch_selection : selections)
        {
            if ( ! dch_selection->check_data( dch_data, lkr_data ) )
            {
                return false;
            }
        }
        return true;
    }

    void DchAnalysis::process_event()
    {
        Vertex vertex = extract_vertex( k2pi_data_.raw_dch, *lkr_data_ );
        double cda = vertex.cda;

        bool passed_dch = check_dch_selections( dch_selections_, &k2pi_data_.raw_dch, lkr_data_ );

        double wgt = k2pi_data_.weight * get_weight();

        if( is_mc_ )
        {
            if ( passed_dch )
            {
                plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, 
                        wgt, true, &k2pi_data_.mc  );

                event_plots_.plot_data( k2pi_data_, *lkr_data_ , k2pi_data_.raw_dch, 
                        wgt, true, &k2pi_data_.mc  );
            }

            for ( int i  = 0 ; i != n_scatterers_ ; ++i )
            {
                K2piDchData mod_dch = k2pi_data_.raw_dch;
                scatterers_[i].scatter_track( k2pi_data_.compact_number, 
                        mod_dch.dxdz, mod_dch.dydz, mod_dch.p );

                Vertex vertex = extract_vertex( mod_dch, *lkr_data_ );
                double cda = vertex.cda;

                bool passed_mod_dch = check_dch_selections( dch_selections_, &mod_dch, lkr_data_ );

                if ( cda < 10.0 )
                    if ( passed_mod_dch )
                    {
                        scatter_plots_[i].plot_data( k2pi_data_, *lkr_data_, mod_dch, 
                                wgt, true, &k2pi_data_.mc );
                    }
            }
        }
        else
        {
            //2015-03-18 I think in raw_dch here, raw means no extra kick, 
            //rather than no kinematic fitting.

            if ( passed_dch )
            {
                plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, 
                        wgt, false, 0 );

                event_plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, 
                        wgt, false, 0  );
            }
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
