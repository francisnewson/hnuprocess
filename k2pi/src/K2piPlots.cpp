#include "K2piPlots.hh"
#include "k2pi_extract.hh"
#include  "k2pi_functions.hh"
#include "NA62Constants.hh"
#include "Track.hh"
#include "TMath.h"

namespace fn
{

    K2piPlots::K2piPlots( const fn::K2piVars * vars, TFile& tf,
            boost::filesystem::path folder)
        :vars_( vars ), tf_( tf ), folder_( folder ), found_mc_( false )
    {
        //init Data histograms
        init_data();

        //init MC histograms
        init_mc();
        init_resids();
    }

    void K2piPlots::new_event()
    {
        bool mc = vars_->ismc;

        process_data();

        if ( mc )
        {
            found_mc_ = true;
            process_mc();
            process_resids();
        }
    }

    void K2piPlots::init_data()
    {
        hm2pip_lkr_ = dths_.MakeTH1D( "hm2pip_lkr", "Pi+ mass from Lkr",
                1000, -0.3, 0.2, "m^{2}_{miss}", "#events" );

        hpt_dch_uw_ = dths_.MakeTH1D( "hpt_dch_uw",
                "P_{T} measured in DCH - Unweighted",
                1000, 0, 0.5, "P_{T} (GeV)", "#events" );

        hpt_dch_ = dths_.MakeTH1D( "hpt_dch", "P_{T} measured in DCH",
                1000, 0, 0.5, "P_{T} (GeV)", "#events" );

        hz_lkr_dch_ = dths_.MakeTH1D( "hz_lkr_dch", "Z LKr - DCH" ,
                1000, -1000, 1000, "dZ ( cm )" );

        hpt_event_dch_ = dths_.MakeTH1D( "hpt_event_dch", 
                "Event p_{T} - DCH", 
                1000, 0, 1, "p_{T} (GeV/c ) " );

        hz_lkr_ = dths_.MakeTH1D( "hz_lkr", "Z - Lkr" ,
                1200, -2000, 10000,"Z Lkr ( cm )" );

        hrpi_dch_dch_ = dths_.MakeTH1D( "hrpi_dch_dch", "Pi+ radius at DCH [Charged]" ,
                1000, 0, 200, "R( cm )" );

        hrpi_dch_lkr_ = dths_.MakeTH1D( "hrpi_dch_lkr", "Pi+ radius at DCH [Neutral]" ,
                1000, 0, 200, "R( cm )" );

        hetot_ = dths_.MakeTH1D( "hetot", "Total event energy: dch Pi+ + lkr Pi0",
                1000, 65, 85, "E (GeV) " );

        hchi2_ = dths_.MakeTH1D( "hchi2", "Fit #chi^{2}",
                1000, 0, 5, "#chi^{2}" );

        hprob_ = dths_.MakeTH1D( "hprob", "Fit prob",
                1000, 0, 1, "fit prob" );
    }

    void K2piPlots::process_data()
    {
        double wgt = vars_->weight;
        const TVector3& neutral_vertex = vars_->data.neutral_vertex;
        const TVector3& charged_vertex = vars_->data.charged_vertex;
        const TLorentzVector& p4pip_lkr = vars_->data.p4pip_lkr;
        const TLorentzVector& p4pi0_lkr = vars_->data.p4pi0_lkr;
        const TLorentzVector& p4pip_dch = vars_->data.p4pip_dch;
        double pt_dch = vars_->data.pt_dch;
        const TVector3& beam_momentum = vars_->data.beam_momentum;
        double chi2 = vars_->chi2;

        hm2pip_lkr_->Fill ( p4pip_lkr.M2(), wgt);

        hz_lkr_->Fill(neutral_vertex.Z(), wgt);

        lkr_dch_cmp_.Fill ( p4pip_lkr, p4pip_dch, wgt );

        uw_lkr_dch_cmp_.Fill ( p4pip_lkr, p4pip_dch, 1.0 );

        hpt_dch_->Fill( pt_dch , wgt );

        hpt_dch_uw_->Fill( pt_dch , 1.0 );

        hz_lkr_dch_->Fill( neutral_vertex.Z() - charged_vertex.Z(), wgt );

        hetot_->Fill( (p4pip_dch + p4pi0_lkr ).E(), wgt );

        hchi2_->Fill( chi2, wgt );

        hprob_->Fill( TMath::Prob( chi2, 1 ) , wgt );

        //Calculate event p_T
        TVector3 event_p = p4pip_dch.Vect() + p4pi0_lkr.Vect();
        TVector3 event_pt = event_p.Perp( beam_momentum );
        hpt_event_dch_->Fill(  event_pt.Mag(), wgt );

        //Pion track
        Track pion_track_lkr{ neutral_vertex, p4pip_lkr.Vect() };
        TVector3 pion_at_dch_lkr = pion_track_lkr.extrapolate( na62const::zDch1 );
        hrpi_dch_lkr_->Fill( pion_at_dch_lkr.Perp() , wgt );

        Track pion_track_dch{ charged_vertex, p4pip_dch.Vect() };
        TVector3 pion_at_dch_dch = pion_track_dch.extrapolate( na62const::zDch1 );
        hrpi_dch_dch_->Fill( pion_at_dch_dch.Perp() , wgt );
    }

    void K2piPlots::init_mc()
    {

        hdz_neutral_ = mchs_.MakeTH1D( "hdz_neutral_",
                "dZ Neutral vertex - MC",
                1000, -1000, 1000, "dZ (cm )" );

        hdz_charged_ = mchs_.MakeTH1D( "hdz_charged_",
                "dZ Charged vertex - MC",
                1000, -1000, 1000, "dZ (cm )" );

        hdz_tp_ = mchs_.MakeTH1D( "hdz_tp",
                "dZ vertex  ( true pion ) - MC",
                1000, -1000, 1000, "dZ (cm )" );

        hdxdy_ = mchs_.MakeTH2D( "hdxdy", "Cluster - Photon", 
                200, -100, 100, "dX",
                200, -100, 100, "dY");

        hdx1dz_ = mchs_.MakeTH2D( "hdx1dz", "X Cluster1 - Photon1 vs Z", 
                200, -100, 100, "dX",
                200, -1000, 1000, "dZ");

        hdy1dz_ = mchs_.MakeTH2D( "hdy1dz", "Y Cluster1 - Photon1 vs Z", 
                200, -100, 100, "dX",
                200, -1000, 1000, "dZ");

        hdxdy1_ = mchs_.MakeTH2D( "hdxdy1", "Cluster1 - Photon1", 
                200, -100, 100, "dX",
                200, -100, 100, "dY");

        hdxdy2_ = mchs_.MakeTH2D( "hdxdy2", "Cluster2 - Photon2", 
                200, -100, 100, "dX",
                200, -100, 100, "dY");

        hdEEdz_ = mchs_.MakeTH2D( "hdEEdz", "dEE vs dZ", 
                600, -1500, 1500, "dEE",
                200, -1000, 1000, "dZ");

        hdEEvsEE_ = mchs_.MakeTH2D( "hdEEvsEE", "dEE vs EE", 
                400, 0, 2000, "EE",
                400, -1500, 1500, "dEE"
                );

        hdEE_ = mchs_.MakeTH1D( "hdEE", "Delta EE",
                1000, -1500,  1500, "Delta E1*E2",
                "#entries" );

        hdSepdz_ = mchs_.MakeTH2D( "hdSepdz", "dSep vs dZ", 
                200, -100, 100, "dSep",
                200, -1000, 1000, "dZ");

        hdpvsdz_ = mchs_.MakeTH2D( "hdpvsdz", "dp (pi+) vs dZ", 
                200, -1000, 1000, "dZ",
                200, -50, 50, "dp");

        h_event_weight_ = mchs_.MakeTH1D( "h_event_weight_",
                "Event weights",
                1000, 0.8, 2.8, "Weight" );

        h_event_weight_mom_ = mchs_.MakeTH2D( "h_event_weight_mom_",
                "Event weight vs momentim ",
                100, 70, 80, "Momentum (GeV)",
                100, 0.8, 2.8, "Event weight" );
    }

    void K2piPlots::process_mc()
    {
        //Cluster positions
        TVector3 delta1 = delta_cluster(
                vars_->mc.p4g1, vars_->data.pos1, vars_->mc.vertex );
        TVector3 delta2 = delta_cluster( 
                vars_->mc.p4g2, vars_->data.pos2, vars_->mc.vertex );

        hdxdy_->Fill( delta1.X(), delta1.Y(), vars_->weight );
        hdxdy_->Fill( delta2.X(), delta2.Y(), vars_->weight );

        hdxdy1_->Fill( delta1.X(), delta1.Y(), vars_->weight );
        hdxdy2_->Fill( delta2.X(), delta2.Y(), vars_->weight );

        //Z vertex
        double dZ = vars_->data.neutral_vertex.Z() - vars_->mc.vertex.Z() ;
        hdx1dz_->Fill( delta1.X(), dZ, vars_->weight );
        hdy1dz_->Fill( delta1.Y(), dZ , vars_->weight);

        hdz_neutral_->Fill( dZ, vars_->weight );

        double dZ_charged = vars_->data.charged_vertex.Z() - vars_->mc.vertex.Z() ;
        hdz_charged_->Fill( dZ_charged , vars_->weight );


        //Cluster energies
        double dataEE = vars_->data.E1 * vars_->data.E2;
        double mcEE = vars_->mc.p4g1.E() * vars_->mc.p4g2.E();
        double dEE = dataEE - mcEE;

        hdEEdz_->Fill( dEE, dZ, vars_->weight );
        hdEE_->Fill( dEE, vars_->weight );
        hdEEvsEE_->Fill( dataEE, dEE, vars_->weight );

        //Cluster separation
        TVector3 photon_sep = 
            extrapolate_photon( vars_->mc.p4g1, vars_->data.pos1, vars_->mc.vertex ) - 
            extrapolate_photon( vars_->mc.p4g2, vars_->data.pos2, vars_->mc.vertex );

        TVector3 cluster_sep = vars_->data.pos1 - vars_->data.pos2;
        double delta_sep = cluster_sep.Mag() - photon_sep.Mag();

        hdSepdz_->Fill( delta_sep, dZ, vars_->weight );

        //Delta P
        double delta_p = vars_->data.p4pip_lkr.P() - vars_->mc.p4pip.P();
        hdpvsdz_->Fill( dZ, delta_p, vars_->weight );

        h_event_weight_->Fill( vars_->weight ) ;
        h_event_weight_mom_->Fill( 
                vars_->mc.p4k.P() , vars_->weight )  ;

        //Pi+ 4mom comparisions
        lkr_mc_cmp_.Fill( vars_->data.p4pip_lkr  , vars_->mc.p4pip, vars_->weight );

        const TLorentzVector& p4_mc_K = vars_->mc.p4k;
        const TVector3& p3_dt_K = vars_->data.beam_momentum;
        double EK = std::hypot( na62const::mK, p3_dt_K.Mag() );
        TLorentzVector p4_dt_K{ p3_dt_K , EK };

        lkr_tk_mc_cmp_.Fill( p4_mc_K - vars_->data.p4pi0_lkr  , 
                vars_->mc.p4pip, vars_->weight );

        mc_dch_cmp_.Fill(vars_->mc.p4pip, vars_->data.p4pip_dch,  vars_->weight );

        //Z vertex comparisions
        Track kaon_track( vars_->data.beam_point, vars_->data.beam_momentum );
        Track true_charged_track( vars_->mc.vertex, vars_->mc.p4pip.Vect() );
        Vertex charged_vertex_tp = compute_cda( kaon_track, true_charged_track );

        hdz_tp_->Fill( charged_vertex_tp.point.Z() - vars_->mc.vertex.Z(), 
                vars_->weight );
    }

    void K2piPlots::init_resids()
    {
        hr_E1_ = resid_.MakeTH1D( "hr_E1", "E1 Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_E2_ = resid_.MakeTH1D( "hr_E2", "E2 Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_C1_X_ = resid_.MakeTH1D( "hr_C1_X", "C1_X Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_C2_X_ = resid_.MakeTH1D( "hr_C2_X", "C2_X Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_C1_Y_ = resid_.MakeTH1D( "hr_C1_Y", "C1_Y Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_C2_Y_ = resid_.MakeTH1D( "hr_C2_Y", "C2_Y Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_pK_X_ = resid_.MakeTH1D( "hr_pK_X", "pK_X Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_pK_Y_ = resid_.MakeTH1D( "hr_pK_Y", "pK_Y Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_pK_Z_ = resid_.MakeTH1D( "hr_pK_Z", "pK_Z Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_posK_X_ = resid_.MakeTH1D( "hr_posK_X", "posK_X Normalized Residuals",
                1000, -5, 5 , "r" );

        hr_posK_Y_ = resid_.MakeTH1D( "hr_posK_Y", "posK_Y Normalized Residuals",
                1000, -5, 5 , "r" );
    }

    void K2piPlots::process_resids()
    {
        double wgt = vars_->weight;

        Track mc_pip_track( vars_->mc.vertex, vars_->mc.p4pip.Vect() );
        Track mc_g1_track( vars_->mc.vertex, vars_->mc.p4g1.Vect() );
        Track mc_g2_track( vars_->mc.vertex, vars_->mc.p4g2.Vect() );

        //E1
        double dtE1 = vars_->data.E1;
        double mcE1 = vars_->mc.p4g1.E();
        double erE1 = lkr_energy_res( dtE1 );
        hr_E1_->Fill( ( dtE1 - mcE1 ) / erE1 , wgt );

        //E2
        double dtE2 = vars_->data.E2;
        double mcE2 = vars_->mc.p4g2.E();
        double erE2 = lkr_energy_res( dtE2 );
        hr_E2_->Fill( ( dtE2 - mcE2 ) / erE2 , wgt );

        TVector3 dtC1 = vars_->data.pos1;
        TVector3 mcC1 = mc_g1_track.extrapolate( dtC1.Z() );

        //C1_X
        double dtC1X = dtC1.X();
        double mcC1X = mcC1.X();
        double erC1X = lkr_pos_res( dtE1 );
        hr_C1_X_->Fill( ( dtC1X - mcC1X ) / erC1X, wgt );

        //C1_Y
        double dtC1Y = dtC1.Y();
        double mcC1Y = mcC1.Y();
        double erC1Y = lkr_pos_res( dtE1 );
        hr_C1_Y_->Fill( ( dtC1Y - mcC1Y ) / erC1Y, wgt );

        TVector3 dtC2 = vars_->data.pos2;
        TVector3 mcC2 = mc_g2_track.extrapolate( dtC2.Z() );

        //C2_X
        double dtC2X = dtC2.X();
        double mcC2X = mcC2.X();
        double erC2X = lkr_pos_res( dtE2 );
        hr_C2_X_->Fill( ( dtC2X - mcC2X ) / erC2X, wgt );

        //C2_Y
        double dtC2Y = dtC2.Y();
        double mcC2Y = mcC2.Y();
        double erC2Y = lkr_pos_res( dtE2 );
        hr_C2_Y_->Fill( ( dtC2Y - mcC2Y ) / erC2Y, wgt );

        TVector3 mcPK = vars_->mc.p4k.Vect();
        TVector3 dtPK = vars_->data.beam_momentum;

        //PK_X
        double mcPK_X = mcPK.X();
        double dtPK_X = dtPK.X();
        double erPK_X = 1e-3;
        hr_pK_X_->Fill( ( mcPK_X - dtPK_X ) / erPK_X, wgt );

        //PK_Y
        double mcPK_Y = mcPK.Y();
        double dtPK_Y = dtPK.Y();
        double erPK_Y = 1e-3;
        hr_pK_Y_->Fill( ( mcPK_Y - dtPK_Y ) / erPK_Y, wgt );

        //PK_Z
        double mcPK_Z = mcPK.Z();
        double dtPK_Z = dtPK.Z();
        double erPK_Z = 1.5;
        hr_pK_Z_->Fill( ( mcPK_Z - dtPK_Z ) / erPK_Z, wgt );

        Track mcKtrack { vars_->mc.vertex, mcPK };
        TVector3 mcPosK = mcKtrack.extrapolate( 0 );
        TVector3 dtPosK = vars_->data.beam_point;

        //PK_X
        double mcPosK_X = mcPosK.X();
        double dtPosK_X = dtPosK.X();
        double erPosK_X = 0.3;
        hr_posK_X_->Fill( ( mcPosK_X - dtPosK_X ) / erPosK_X, wgt );

        //PosK_Y
        double mcPosK_Y = mcPosK.Y();
        double dtPosK_Y = dtPosK.Y();
        double erPosK_Y = 0.3;
        hr_posK_Y_->Fill( ( mcPosK_Y - dtPosK_Y ) / erPosK_Y, wgt );
    }

    void K2piPlots::end_processing()
    {
        cd_p( &tf_,  folder_.string().c_str() );
        dths_.Write();

        if ( found_mc_ )
        {
            cd_p( &tf_, (folder_ / "mc").string().c_str() );
            mchs_.Write();

            cd_p( &tf_, (folder_ / "mc" / "lkr_mc" ).string().c_str() );
            lkr_mc_cmp_.Write();

            cd_p( &tf_, (folder_ / "mc" / "lkr_tk_mc" ).string().c_str() );
            lkr_tk_mc_cmp_.Write();

            cd_p( &tf_, (folder_ / "mc" / "mc_dch" ).string().c_str() );
            mc_dch_cmp_.Write();

            cd_p( &tf_, ( folder_ / "resids" ).string().c_str() );
            resid_.Write();

            tf_.cd();
        }

        cd_p( &tf_, ( folder_ / "lkr_dch" ).string().c_str() );
        lkr_dch_cmp_.Write();
        tf_.cd();

        cd_p( &tf_, ( folder_ / "uw_lkr_dch" ).string().c_str() );
        uw_lkr_dch_cmp_.Write();
        tf_.cd();
    }

    //--------------------------------------------------

    //auxillary functions

    TVector3 extrapolate_photon( 
            const TLorentzVector& p, const TVector3& c,
            const TVector3& vertex )
    {
        auto ptrack = fn::Track( vertex, p.Vect() );
        auto ppos = ptrack.extrapolate( c.Z() );
        return ppos;
    }

    TVector3 delta_cluster(
            const TLorentzVector& p, const TVector3& c,
            const TVector3& vertex )
    {
        auto ptrack = fn::Track( vertex, p.Vect() );
        auto ppos = ptrack.extrapolate( c.Z() );
        auto delta = c - ppos;
        return delta;
    }

    void fill_dx_dy( TH2D * h, 
            const TLorentzVector& p, const TVector3& c,
            const TVector3& vertex )
    {
        auto delta = delta_cluster( p, c, vertex );
        h->Fill( delta.X(), delta.Y() );
    }
}
