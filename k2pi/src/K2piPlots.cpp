#include "K2piPlots.hh"
#include "k2pi_extract.hh"
#include "Track.hh"

namespace fn
{

    K2piPlots::K2piPlots( const fn::K2piVars * vars, TFile& tf )
        :vars_( vars ), tf_( tf ), found_mc_( false )
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

    }

    void K2piPlots::process_data()
    {
        hm2pip_lkr_->Fill
            ( vars_->data.p4pip_lkr.M2(), vars_->weight );

        lkr_dch_cmp_.Fill
            ( vars_->data.p4pip_lkr, vars_->data.p4pip_dch, vars_->weight );

    }

    void K2piPlots::init_mc()
    {
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
            1000, 1500,  1500, "Delta E1*E2",
            "#entries" );

     hdSepdz_ = mchs_.MakeTH2D( "hdSepdz", "dSep vs dZ", 
            200, -100, 100, "dSep",
            200, -1000, 1000, "dZ");

     hdpvsdz_ = mchs_.MakeTH2D( "hdpvsdz", "dp (pi+) vs dZ", 
            200, -1000, 1000, "dZ",
            200, -50, 50, "dp");
    }

    void K2piPlots::process_mc()
    {
        TVector3 delta1 = delta_cluster( vars_->mc.p4g1, vars_->data.pos1, vars_->mc.vertex );
        TVector3 delta2 = delta_cluster( vars_->mc.p4g2, vars_->data.pos2, vars_->mc.vertex );

        hdxdy_->Fill( delta1.X(), delta1.Y() );
        hdxdy_->Fill( delta2.X(), delta2.Y() );

        hdxdy1_->Fill( delta1.X(), delta1.Y() );
        hdxdy2_->Fill( delta2.X(), delta2.Y() );

        double dZ = vars_->data.neutral_vertex.Z() - vars_->mc.vertex.Z() ;
        hdx1dz_->Fill( delta1.X(), dZ );
        hdy1dz_->Fill( delta1.Y(), dZ );

        double dataEE = vars_->data.E1 * vars_->data.E2;
        double mcEE = vars_->mc.p4g1.E() * vars_->mc.p4g2.E();
        double dEE = dataEE - mcEE;

        hdEEdz_->Fill( dEE, dZ );
        hdEE_->Fill( dEE );
        hdEEvsEE_->Fill( dataEE, dEE );

        TVector3 photon_sep = 
            extrapolate_photon( vars_->mc.p4g1, vars_->data.pos1, vars_->mc.vertex ) - 
            extrapolate_photon( vars_->mc.p4g2, vars_->data.pos2, vars_->mc.vertex );

        TVector3 cluster_sep = vars_->data.pos1 - vars_->data.pos2;
        double delta_sep = cluster_sep.Mag() - photon_sep.Mag();

        hdSepdz_->Fill( delta_sep, dZ );

        double delta_p = vars_->data.p4pip_lkr.P() - vars_->mc.p4pip.P();
        hdpvsdz_->Fill( dZ, delta_p );
    }

    void K2piPlots::end_processing()
    {
        tf_.cd();
        dths_.Write();


        cd_p( &tf_, "mc");
        mchs_.Write();
        tf_.cd();

        cd_p( &tf_, "lkr_dch" );
        lkr_dch_cmp_.Write();
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
