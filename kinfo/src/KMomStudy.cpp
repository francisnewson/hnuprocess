#include "KMomStudy.hh"
#include "Track.hh"
#include "yaml_help.hh"
#include "RecoFactory.hh"

namespace fn
{
    KMomStudy::KMomStudy( const Selection& sel,
            TFile& tf, std::string folder,
            const fne::Event * e, bool mc,
            KaonTrack& kt)
        :Analysis( sel ), tfile_( tf ), 
        folder_( folder ), e_( e ),
        kt_(kt)
    {

        h_px = hs_.MakeTH1D( "h_px", "PX Res",
                100, 2, 1 , "dpx ( GeV )" );

        h_py = hs_.MakeTH1D( "h_py", "PY Res",
                100, 2, 1 , "dpy ( GeV )" );

        h_pz = hs_.MakeTH1D( "h_pz", "PZ Res",
                100, 2, 1 , "dpz ( GeV )" );

        h_dxdz = hs_.MakeTH1D( "h_dxdz", "P DXDZ Res",
                100, 2, 1 , "ddxdz ( GeV )" );

        h_dydz = hs_.MakeTH1D( "h_dydz", "PDYDZ Res",
                100, 2, 1 , "ddydz ( GeV )" );

        h_p = hs_.MakeTH1D( "h_p", "PXRes",
                100, 2, 1 , "dp ( GeV )" );

        h_x = hs_.MakeTH1D( "h_x", "X Res",
                100, 2, 1 , "dx ( cm )" );

        h_y = hs_.MakeTH1D( "h_y", "Y Res",
                100, 2, 1 , "dy ( cm )" );

        h_wgt = hs_.MakeTH1D( "h_wgt", "wgt",
                100, 0.5, 2.5, "wgt" );

        //--------------------------------------------------

        hmc_px = hs_.MakeTH1D( "h_mc_px", "MC pX",
                100, 2, 1, "MC pX GeV" );

        hmc_py = hs_.MakeTH1D( "h_mc_py", "MC pY",
                100, 2, 1, "MC pY GeV" );

        hmc_pz = hs_.MakeTH1D( "h_mc_pz", "MC pZ",
                100, 2, 1, "MC pZ GeV" );

        hmc_dxdz = hs_.MakeTH1D( "h_mc_dxdz", "MC dxdz",
                100, 2, 1, "MC dxdz GeV" );

        hmc_dydz = hs_.MakeTH1D( "h_mc_dydz", "MC dydz",
                100, 2, 1, "MC pZ GeV" );

        hmc_p = hs_.MakeTH1D( "h_mc_p", "MC p",
                100, 2, 1, "MC p GeV" );

        hmc_x = hs_.MakeTH1D( "h_mc_x", "MC X",
                100, 2, 1, "MC X GeV" );

        hmc_y = hs_.MakeTH1D( "h_mc_y", "MC Y",
                100, 2, 1, "MC Y GeV" );
    }

    void KMomStudy::process_event()
    {
        //MC kaon
        const TLorentzVector& mckaon4mom
            = e_->mc.decay.kaon_momentum;

        const TVector3& mcdecay_vertex =
            e_->mc.decay.decay_vertex;

        Track mctrack( mcdecay_vertex, mckaon4mom.Vect() );
        TVector3 mcpoint = mctrack.extrapolate( 0 );
        TVector3 mckaon3mom = mckaon4mom.Vect();
        double mckaonmom = mckaon3mom.Mag();

        //data kaon
        TVector3 dtkaon3mom = kt_.get_kaon_3mom();
        double dtkaonmom = kt_.get_kaon_mom();
        TVector3 dtpoint = kt_.extrapolate_z( 0 );

        double wgt = get_weight();

        TVector3 delta_p = dtkaon3mom - mckaon3mom;

        h_px->Fill( delta_p.X(), wgt );
        h_py->Fill( delta_p.Y(), wgt );
        h_pz->Fill( delta_p.Z(), wgt );

        double dtdxdz = dtkaon3mom.X() / dtkaon3mom.Z();
        double dtdydz = dtkaon3mom.Y() / dtkaon3mom.Z();

        double mcdxdz = mckaon3mom.X() / mckaon3mom.Z();
        double mcdydz = mckaon3mom.Y() / mckaon3mom.Z();

        h_dxdz->Fill( dtdxdz - mcdxdz , wgt );
        h_dydz->Fill( dtdydz - mcdydz , wgt );

        h_p->Fill( dtkaonmom - mckaonmom , wgt );

        TVector3 delta_point = dtpoint - mcpoint;

        h_x->Fill( delta_point.X(), wgt );
        h_y->Fill( delta_point.Y(), wgt );

        h_wgt->Fill(wgt );

        //--------------------------------------------------
        hmc_px->Fill( mckaon3mom.X(), wgt );
        hmc_py->Fill( mckaon3mom.Y(), wgt );
        hmc_pz->Fill( mckaon3mom.Z(), wgt );
        hmc_p->Fill( mckaon3mom.Mag(), wgt );

        hmc_dxdz->Fill( mcdxdz, wgt );
        hmc_dydz->Fill( mcdydz, wgt );

        hmc_x->Fill( mcpoint.X(), wgt );
        hmc_y->Fill( mcpoint.Y(), wgt );
    }

    void KMomStudy::end_processing()
    {
        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    REG_DEF_SUB( KMomStudy );

    template<>
        Subscriber * create_subscriber<KMomStudy>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            std::string folder = get_folder( instruct, rf );

            const fne::Event * event = rf.get_event_ptr();

            const Selection * sel = rf.get_selection
                ( get_yaml<std::string>( instruct, "selection" ) );

            TFile& tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            KaonTrack * kt = get_kaon_track( instruct, rf );

            bool is_mc = rf.is_mc();

            return new KMomStudy( *sel, tfile, folder, event, is_mc, *kt );
        }
}
