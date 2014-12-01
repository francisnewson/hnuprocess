#include "Km2Plotter.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"


namespace fn
{
    PZTPlots::PZTPlots()
    {
        for (int i = 0 ; i != 15; ++i )
        {
            hists_.push_back( hs_.MakeTH2D( Form( "hpzt_%d", i ), 
                        Form("PZ for #theta in range %f - %f", i * 1e-3, ( i+1) * 1e-3),
                        100, 0, 100, "P( GeV )",
                        150, -5000, 10000, "Z( cm) ") );
        }
    }

    void PZTPlots::Fill( const Km2RecoEvent& km2re, double wgt  )
    {
        double p = km2re.get_muon_mom();
        double z = km2re.get_zvertex();
        double t = km2re.get_opening_angle();
        int bin =  int( floor( t / (1e-3) ) );
        //std::cerr << t << " " << bin << std::endl;
        if ( bin < hists_.size() )
        {
            hists_[bin]->Fill( p, z, wgt );
        }
    }
    void PZTPlots::Write()
    {
        hs_.Write();
    }

    Km2Plots::Km2Plots()
    {
        //Mass and momentum

        h_pk_ = hs_.MakeTH1D( "h_pk", "Kaon Momentum", 
                1000, 70, 80, "p_{K} GeV" );

        h_m2m_kmu_ = hs_.MakeTH1D( "h_m2m_kmu", "K_{#mu2} missing mass",
                10000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )", "#events" ); 

        h_m2m_kpi_ = hs_.MakeTH1D( "h_m2m_kpi", "K_{2#pi} missing mass",
                10000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )", "#events" ); 

        h_m2m_pimu_ = hs_.MakeTH1D( "h_m2m_pimu", "#pi_{#mu2} missing mass",
                10000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )", "#events" ); 

        h_p_m2m_kmu_ = hs_.MakeTH2D( "h_p_m2m_kmu", "Momentum vs K_{#mu2} missing mass",
                1000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                100, 0, 100, "p (GeV/c)" );

        h_p_m2m_kmu_inner_ = hs_.MakeTH2D( "h_p_m2m_kmu_inner", "Momentum vs K_{#mu2} missing mass",
                1000, -0.01, 0.01, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                100, 0, 100, "p (GeV/c)" );

        h_p_m2m_kpi_ = hs_.MakeTH2D( "h_p_m2m_kpi", "Momentum vs K_{2#pi} missing mass",
                1000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                100, 0, 100, "p (GeV/c)" );

        h_p_m2m_pimu_ = hs_.MakeTH2D( "h_p_m2m_pimu", "Momentum vs #pi_{#mu2} missing mass",
                1000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                100, 0, 100, "p (GeV/c)" );

        h_p_ = hs_.MakeTH1D( "h_p", "Momentum (GeV)",
                100, 0, 100, "p (GeV)" );

        //----------

        //Kinematic shape
        h_cda_ = hs_.MakeTH1D( "h_cda", "CDA (cm)",
                100, 0, 10, "CDA (cm)" );

        h_t_ = hs_.MakeTH1D( "h_t", "Opening angle (rad)",
                250, 0, 25e-3, "#Theta (rad)" );

        h_pt_ = hs_.MakeTH1D( "h_pt", "P_T",
                100, 0, 0.5, "#P_T (GeV)" );

        h_z_ = hs_.MakeTH1D( "h_z", "Z Decay Vertex (cm)",
                120, -2000, 10000, "Z (cm)" );

        //----------

        //Kinematic correlations
        h_p_t_ = hs_.MakeTH2D( "h_p_t", "P vs angle" ,
                100, 0, 100, "P( GeV )",
                250, 0, 25e-3, "t( rad) ");

        h_p_cda_ = hs_.MakeTH2D( "h_p_cda", "P vs cda" ,
                100, 0, 100, "P( GeV )",
                100, 0, 10, "CDA (cm)" );

        h_p_z_ = hs_.MakeTH2D( "h_p_z", "P vs Z vertex" ,
                100, 0, 100, "P( GeV )",
                150, -5000, 10000, "Z( cm) ");

        h_z_t_ = hs_.MakeTH2D( "h_z_t", "Z vs T " ,
                150, -5000, 10000, "Z( cm) ",
                250, 0, 25e-3, "t( rad) ");

        h_z_cda_ = hs_.MakeTH2D( "h_z_cda", "Z vs CDA" ,
                150, -5000, 10000, "Z( cm) ",
                100, 0, 10, "CDA (cm)" );

        h_t_cda_ = hs_.MakeTH2D( "h_t_cda", "T vs CDA" ,
                250, 0, 25e-3, "t( rad) ",
                100, 0, 10, "CDA (cm)" );

        h_m2_t_ = hs_.MakeTH2D( "h_m2_t", "m^{2} vs angle" ,
                1000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                250, 0, 25e-3, "t( rad) ");

        h_m2_z_ = hs_.MakeTH2D( "h_m2_t", "m^{2} vs Z" ,
                1000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                150, -5000, 10000, "Z( cm) ");

        h_m2_cda_ = hs_.MakeTH2D( "h_m2_t", "m^{2} vs CDA" ,
                1000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                100, 0, 10, "CDA (cm)" );

        //----------

        //Slices
        h_xy_coll_ = hs_.MakeTH2D( "h_xy_coll", "XY at Final Collimator",
                100, -200, 200, "X (cm)",
                100, -200, 200, "Y (cm)" );

        h_xy_DCH1_ = hs_.MakeTH2D( "h_xy_DCH1", "XY at DCH1",
                100, -200, 200, "X (cm)",
                100, -200, 200, "Y (cm)" );

        h_xy_DCH4_ = hs_.MakeTH2D( "h_xy_DCH4", "XY at DCH4",
                100, -200, 200, "X (cm)",
                100, -200, 200, "Y (cm)" );

        h_xy_LKr_ = hs_.MakeTH2D( "h_xy_LKr", "XY at LKr",
                100, -200, 200, "X (cm)",
                100, -200, 200, "Y (cm)" );

        h_xy_MUV1_ = hs_.MakeTH2D( "h_xy_MUV1", "XY at MUV1",
                100, -200, 200, "X (cm)",
                100, -200, 200, "Y (cm)" );
    }


    void Km2Plots::Fill( const Km2RecoEvent& km2re, double wgt )
    {
        //Mass and momentum
        h_pk_->Fill( km2re.get_kaon_mom(), wgt );
        h_m2m_kmu_->Fill( km2re.get_m2m_kmu(), wgt );
        h_m2m_kpi_->Fill( km2re.get_m2m_kpi(), wgt );
        h_m2m_pimu_->Fill( km2re.get_m2m_pimu(), wgt );

        h_p_m2m_kmu_->Fill( km2re.get_m2m_kmu(), km2re.get_muon_mom() , wgt );
        h_p_m2m_kmu_inner_->Fill( km2re.get_m2m_kmu(), km2re.get_muon_mom() , wgt );
        h_p_m2m_kpi_->Fill( km2re.get_m2m_kpi(), km2re.get_muon_mom() , wgt );
        h_p_m2m_pimu_->Fill( km2re.get_m2m_pimu(), km2re.get_muon_mom() , wgt );

        h_p_->Fill( km2re.get_muon_mom(), wgt );

        //Kinematic shape
        h_cda_->Fill( km2re.get_cda(), wgt );
        h_t_->Fill( km2re.get_opening_angle() , wgt );
        h_z_->Fill( km2re.get_zvertex(), wgt );

        h_pt_->Fill( km2re.get_pt(), wgt );

        //Kinematic correlations
        h_p_t_->Fill( km2re.get_muon_mom(), km2re.get_opening_angle(), wgt );
        h_p_cda_->Fill( km2re.get_muon_mom(), km2re.get_cda(), wgt );
        h_p_z_->Fill( km2re.get_muon_mom(), km2re.get_zvertex(), wgt );
        h_z_t_->Fill( km2re.get_zvertex(), km2re.get_opening_angle(), wgt );
        h_z_cda_->Fill( km2re.get_zvertex(), km2re.get_cda(), wgt );
        h_t_cda_->Fill( km2re.get_opening_angle(), km2re.get_cda(), wgt );
        h_m2_t_->Fill( km2re.get_m2m_kmu(), km2re.get_opening_angle(), wgt );
        h_m2_z_->Fill( km2re.get_m2m_kmu(), km2re.get_zvertex(), wgt );
        h_m2_cda_->Fill( km2re.get_m2m_kmu(), km2re.get_cda(), wgt );

        //Slices
        const SingleRecoTrack * srt = km2re.get_reco_track();

        TVector3 v_coll = srt->extrapolate_bf( na62const::zFinalCollimator );
        h_xy_coll_->Fill( v_coll.X(), v_coll.Y(), wgt );

        TVector3 v_DCH1 = srt->extrapolate_us( na62const::zDch1 );
        h_xy_DCH1_->Fill( v_DCH1.X(), v_DCH1.Y(), wgt );

        TVector3 v_DCH4 = srt->extrapolate_ds( na62const::zDch4 );
        h_xy_DCH4_->Fill( v_DCH4.X(), v_DCH4.Y(), wgt );

        TVector3 v_LKr = srt->extrapolate_ds( na62const::zLkr );
        h_xy_LKr_->Fill( v_LKr.X(), v_LKr.Y(), wgt );

        TVector3 v_MUV1 = srt->extrapolate_ds( na62const::zMuv1 );
        h_xy_MUV1_->Fill( v_MUV1.X(), v_MUV1.Y(), wgt );
    }

    void Km2Plots::Write()
    {
        hs_.Write();
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Plotter);

    Km2Plotter::Km2Plotter( const Selection& sel, 
            TFile& tfile, std::string folder,
            const Km2Event& km2_event)
        :Analysis( sel), tfile_(tfile),folder_( folder ), km2_event_( km2_event )
    {
    }

    void Km2Plotter::process_event()
    {
        const Km2RecoEvent& km2re = km2_event_.get_reco_event();
        km2_plots_.Fill( km2re, get_weight() );

    }

    void Km2Plotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        km2_plots_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2Plotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            return new Km2Plotter( *sel, tfile, folder, *km2_event);
        }

    //--------------------------------------------------

    REG_DEF_SUB( Km2PZTPlotter);

    Km2PZTPlotter::Km2PZTPlotter( const Selection& sel, 
            TFile& tfile, std::string folder,
            const Km2Event& km2_event)
        :Analysis( sel), tfile_(tfile),folder_( folder ), km2_event_( km2_event )
    {
    }

    void Km2PZTPlotter::process_event()
    {
        const Km2RecoEvent& km2re = km2_event_.get_reco_event();
        pzt_plots_.Fill( km2re, get_weight() );

    }

    void Km2PZTPlotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        pzt_plots_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2PZTPlotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            return new Km2PZTPlotter( *sel, tfile, folder, *km2_event);
        }


}
