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

        hp_ = dths_.MakeTH1D( "hp", "DCH p",
                100, 0, 100, "p( GeV )", "#events" );

        hdm2_pi0_ = dths_.MakeTH1D( "hdm2_pi0", "DCH pi0 mass difference",
                1000, -0.2, 0.2, "dm^{2}", "#events" );

        hm2_tx_ = dths_.MakeTH2D( "hm2_tx_", "Mass angle correlation" ,
                100, -0.3, 0.2, "m^{2}_{miss}",
                100, -0.01, 0.01, "dtx" );

        hm2_ty_ = dths_.MakeTH2D( "hm2_ty_", "Mass angle correlation" ,
                100, -0.3, 0.2, "m^{2}_{miss}",
                100, -0.01, 0.01, "dty" );

        hxy_muv_ = dths_.MakeTH2D( "hxy_muv", "track xy at MUV" ,
                150, -150, 150, "x",
                150, -150, 150, "y" );

        hxy_lkr_ = dths_.MakeTH2D( "hxy_lkr", "track xy at lkr" ,
                150, -150, 150, "x",
                150, -150, 150, "y" );

        hmuv_eff_ = dths_.MakeTH1D( "hmuv_eff", "Muon veto efficiency",
                1000, -0.5, 1.5, "efficiency", "#events" );

        hchi2_vs_z_ = dths_.MakeTH2D( "hchi2vsz", "Fit Chi2 vs Z vertex",
                200, -2000 , 10000,  "z", 
                100, 0.0 , 10,  "Chi2" );

        hchi2_vs_p_ = dths_.MakeTH2D( "hchi2vsp", "Fit Chi2 vs pi+ momentum",
                100, 0,    100,  "p", 
                100, 0.0 , 10,  "Chi2" );
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
            hp_->Fill( dch_pip_3mom.Mag() );

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


            //Downstream track
            Track ds_track{
                TVector3{ dch_interface.ds_x0(), dch_interface.ds_y0(), na62const::z_tracking },
                    TVector3{ dch_interface.ds_dxdz(), dch_interface.ds_dydz(), 1.0 } };

            double track_muv_x = ds_track.extrapolate( na62const::zMuv2).X();
            double track_muv_y = ds_track.extrapolate( na62const::zMuv1).Y();

            TVector3 track_lkr = ds_track.extrapolate( na62const::zLkr );

            hxy_muv_->Fill( track_muv_x, track_muv_y, weight );
            hxy_lkr_->Fill( track_lkr.X(), track_lkr.Y(), weight );

            hchi2_vs_z_->Fill( vertex.point.Z() ,  event_data.lkr_fit_chi2, weight );
            hchi2_vs_p_->Fill( lkr_pip_4mom.P(), event_data.lkr_fit_chi2, weight );

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


        hchi2_zoom_ = dths_.MakeTH1D( "hchi2_zoom", "Fit Chi2",
                10000, 0.0 , 100,  "Chi2", "#events" );

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

        hchi2_vs_E1_ = dths_.MakeTH2D( "hchi2vsE1", "Fit Chi2 vs E1",
                100, 0,    100,  "E1", 
                100, 0.0 , 10,  "Chi2" );

        hchi2_vs_E2_ = dths_.MakeTH2D( "hchi2vsE2", "Fit Chi2 vs E2",
                100, 0,    100,  "E2", 
                100, 0.0 , 10,  "Chi2" );

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
        hchi2_zoom_->Fill( event_data.lkr_fit_chi2, weight );
        hhigh_photon_energy_->Fill( lkr_data.E1, weight );
        hlow_photon_energy_->Fill( lkr_data.E2, weight );

        hphoton_energy_->Fill( lkr_data.E1, lkr_data.E2, weight );

        hchi2_vs_E1_->Fill( lkr_data.E1, event_data.lkr_fit_chi2, weight );
        hchi2_vs_E2_->Fill( lkr_data.E2, event_data.lkr_fit_chi2, weight );
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
            std::string lkr_data_source, bool is_mc, bool do_scatter)
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


        std::vector<scatter_params> scatterings = {};

        if( do_scatter )
        {
            scatterings = { 
                { "b",       0.00085, 0.0100  , 2  , 0.08, 0.0010, 2}, 
                { "r08" ,    0.00085, 0.0040  , 2  , 0.08, 0.0016, 2}, 
                { "s4"  ,    0.00085, 0.004000, 4  , 0.08, 0.001600, 4}, 
                { "s4high",  0.00085, 0.040000, 4  , 0.08, 0.016000, 4}, 
                { "s4low" ,  0.00085, 0.000400, 4  , 0.08, 0.000160, 4}, 
            };
        }

        n_scatterers_ = scatterings.size();

        for ( auto scat :  scatterings )
        {
            std::cout << "SCATTERING: " << std::setw(6) << scat.name;
            std::cout << "angle_cutoff: " << scat.angle_cutoff;
            std::cout << "angle_frequency: " << scat.angle_frequency;

            std::cout << "mom_cutoff: " << scat.mom_cutoff;
            std::cout << "mom_frequency: " << scat.mom_frequency << "\n";

            scatterers_.push_back( TrackPowerScatterer( 
                        scat.angle_cutoff, scat.angle_frequency, scat.angle_power,
                        scat.mom_cutoff, scat.mom_frequency, scat.mom_power ) );

            path folder{ folder_};
            folder /= ("scatter_" + scat.name );

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
                scatterers_[i].scatter_track( k2pi_data_.compact_number, mod_dch.dxdz, mod_dch.dydz, mod_dch.p );

                bool passed_mod_dch = check_dch_selections( dch_selections_, &mod_dch, lkr_data_ );

                if ( passed_mod_dch )
                {
                    scatter_plots_[i].plot_data( k2pi_data_, *lkr_data_, mod_dch, wgt, true, &k2pi_data_.mc );
                }
            }
        }
        else
        {
            //2015-03-18 I think in raw_dch here, raw means no extra kick, 
            //rather than no kinematic fitting.
            if ( passed_dch )
            {
                plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, wgt, false, 0 );
                event_plots_.plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, wgt, false, 0  );

                for ( int i  = 0 ; i != n_scatterers_ ; ++i )
                {
                    scatter_plots_[i].plot_data( k2pi_data_, *lkr_data_, k2pi_data_.raw_dch, wgt, false, 0);
                }
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
