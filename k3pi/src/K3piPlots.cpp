#include "K3piPlots.hh"
#include "K3piReco.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"

namespace fn
{
    REG_DEF_SUB( K3piPlots );

    K3piPlots::K3piPlots( const Selection& sel, const K3piReco & k3pi_reco,
            TFile& tf, std::string folder )
        :Analysis( sel ), k3pi_reco_( k3pi_reco ), tfout_( tf  ), folder_( folder )
    {
        h_m2_ = hs_.MakeTH1D( "h_m2m", "Reconstructed kaon mass",
                1000, 0.20, 0.30 , "m^{2}_{miss}" );

        h_dm2_ = hs_.MakeTH1D( "h_dm2m", "Delta reconstructed kaon mass",
                1000, -0.2, 0.2 , "m^{2}_{miss}" );

        h_pt_ = hs_.MakeTH1D( "h_pt", "Transverse momentum", 
                1000, 0, 5, "p_{T}" );

        h_p_ = hs_.MakeTH1D( "h_p", "Kaon momentum", 
                1500, 0, 200, "p" );

        h_dxdz_ = hs_.MakeTH1D( "h_dxdz", "Kaon dxdz", 
                10000, -4e-4, 4e-4, "dxdz" );

        h_dydz_ = hs_.MakeTH1D( "h_dydz", "Kaon dydz", 
                10000, -4e-4, 4e-4, "dydz" );

        h_chi2_ = hs_.MakeTH1D( "h_chi2_", "Chi2", 
                1000, 0, 100, "chi2" );
    }

    void K3piPlots::process_event()
    {
        const K3piRecoEvent&  k3pi_re = k3pi_reco_.get_k3pi_reco_event();
        double wgt = get_weight();

        h_m2_->Fill( k3pi_re.get_invariant_mass2(), wgt );
        h_dm2_->Fill( k3pi_re.get_invariant_mass2() - na62const::mK2, wgt );
        h_pt_->Fill( k3pi_re.get_pt(), wgt  );
        h_chi2_->Fill( k3pi_re.get_chi2(), wgt  );
        h_p_->Fill( k3pi_re.get_mom(), wgt  );

        TVector3 kaon_3mom = k3pi_re.get_kaon_mom();
        h_dxdz_->Fill( kaon_3mom.X()  / kaon_3mom.Z() );
        h_dydz_->Fill( kaon_3mom.Y()  / kaon_3mom.Z() );
    }

    void K3piPlots::end_processing()
    {
        cd_p( &tfout_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<K3piPlots>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = 
                rf.get_selection( get_yaml<std::string>( instruct, "selection" ) );
            
            const K3piReco * k3pi_reco = get_sub<K3piReco>( instruct, rf );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            return new K3piPlots( *sel, *k3pi_reco, tfile, folder );
        }
}
