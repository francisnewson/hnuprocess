#include "MuonCheck.hh"
#include "tracking_selections.hh"
#include "Km2Reco.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"
#include "TMath.h"

namespace fn
{

    MuonCheckPlots::MuonCheckPlots()
    {
        h_m2m_ = hs_.MakeTH1D( "h_m2m", "Missing m^{2}", 
                10000, -0.7, 0.3, "m^{2}_{miss} GeV^{2}/c^{4}" );

        h_p_ = hs_.MakeTH1D( "h_p", "Momentum (GeV/c)",
                1000, 0, 100, "p (GeV/c)" );

        h_t_ =  hs_.MakeTH1D( "h_t", "Opening angle (rad)", 
                1000, 0, 0.025, "#theta" );

        h_z_ = hs_.MakeTH1D( "h_z", "Z vertex (cm)",
                1200, -2000, 10000, "Z (cm)" );

        h_rmuv_ = hs_.MakeTH1D( "h_rmuv", "Radius at Muv1" , 
                1000, 0, 250, "r (cm)" );

        h_phi_ = hs_.MakeTH1D( "h_phi", "Track phi",
                1000, 0, 2 * TMath::Pi(), "phi" );
    }

    void MuonCheckPlots::Fill( const SingleRecoTrack& srt, 
            const KaonTrack& kt, const Km2RecoEvent& km2re, double wgt )
    {
        h_m2m_->Fill( km2re.get_m2m_kmu(), wgt );
        h_p_->Fill( srt.get_mom() , wgt );
        h_t_->Fill( km2re.get_opening_angle(), wgt );
        h_z_->Fill( km2re.get_zvertex(), wgt );

        TVector3 vmuv1 = srt.extrapolate_ds( na62const::zMuv1 );
        h_rmuv_->Fill( vmuv1.Perp(), wgt );

        h_phi_->Fill( srt.get_3mom().Phi() , wgt );
    }

    //--------------------------------------------------

    void MuonCheckPlots::Write()
    {
        hs_.Write();
    }

    REG_DEF_SUB( MuonCheck );

    MuonCheck::MuonCheck( const Selection& sel,
            const Selection& mr,
            const SingleTrack& st,
            const KaonTrack& kt,
            const Km2Event & km2e,
            TFile& tf, std::string folder )
        :Analysis( sel ),
        mr_( mr ), st_( st ), kt_( kt ), km2e_( km2e),
        tf_( tf ), folder_( folder )
    {
    }

    void MuonCheck::process_event()
    {
        double wgt = get_weight();
        bool found_muon  = mr_.check();

        const auto& srt = st_.get_single_track();
        const auto& km2re = km2e_.get_reco_event();

        all_.Fill( srt, kt_, km2re, wgt );

        if ( found_muon )
        {
            wgt *= mr_.get_weight();
            pass_.Fill( srt, kt_, km2re, wgt );
        }
    }

    void MuonCheck::end_processing()
    {
        boost::filesystem::path write_folder = folder_;

        cd_p( &tf_, write_folder/"pass" );
        pass_.Write();

        cd_p( &tf_, write_folder/"all" );
        all_.Write();
    }

    template<>
        Subscriber * create_subscriber<MuonCheck>
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv_sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "muv_selection" ) );

            const SingleTrack * st =  get_single_track( instruct, rf );

            const Km2Event * km2e = get_km2_event( instruct, rf );

            const KaonTrack * kt = get_kaon_track( instruct, rf );

            TFile & tfile = rf.get_tfile(
                    get_yaml<std::string>( instruct, "tfile"  ));

            std::string folder = get_folder( instruct, rf );

            return new MuonCheck( *sel, *muv_sel, *st, *kt, *km2e, tfile, folder );


        }

    //--------------------------------------------------
}
