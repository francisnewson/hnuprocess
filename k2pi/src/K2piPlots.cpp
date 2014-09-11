#include "K2piPlots.hh"
#include "k2pi_extract.hh"
#include "NA62Constants.hh"
#include "Track.hh"

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
    }

    void K2piPlots::new_event()
    {
        bool mc = vars_->ismc;

        process_data();

        if ( mc )
        {
            found_mc_ = true;
            process_mc();
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

        hm2pip_lkr_->Fill ( p4pip_lkr.M2(), wgt);

        hz_lkr_->Fill(neutral_vertex.Z(), wgt);

        lkr_dch_cmp_.Fill ( p4pip_lkr, p4pip_dch, wgt );

        uw_lkr_dch_cmp_.Fill ( p4pip_lkr, p4pip_dch, 1.0 );

        hpt_dch_->Fill( pt_dch , wgt );

        hpt_dch_uw_->Fill( pt_dch , 1.0 );

        hz_lkr_dch_->Fill( neutral_vertex.Z() - charged_vertex.Z(), wgt );

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
