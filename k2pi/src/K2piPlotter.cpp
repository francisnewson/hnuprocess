#include "K2piPlotter.hh"
#include "yaml_help.hh"
#include "TProfile.h"

namespace fn
{
    REG_DEF_SUB( K2piPlotter);

    K2piPlotter::K2piPlotter( const Selection& sel,
            const fne::Event * e,
            TFile& tfile, std::string folder,
            const K2piReco& k2pi_reco, bool mc)
        :Analysis( sel), e_( e),
        tfile_( tfile), folder_( folder),
        k2pi_reco_( k2pi_reco ), mc_( mc )
    {
        h_m2_pip_lkr_ = hs_.MakeTH1D(
                "h_m2_pip_lkr", "Pi+ m^{2}",
                1000, -0.1, 0.2 , "pi+ m^{2} (GeV^{2})",
                "#events");

        h_m2_pi0 = hs_.MakeTH1D(
                "h_m2_pi0", "Pi0 m^{2}",
                1000, -0.1, 0.2 , "pi0 m^{2} (GeV^{2})",
                "#events");

        h_neutral_z_ = hs_.MakeTH1D(
                "h_neutral_z", "Neutral Z Vertex",
                1000, -5000, 10000, "dZ (cm)",
                "#events" );

        if (mc_)
        {
            h_dz_neut_mc_ = hs_.MakeTH1D(
                    "h_dz_neut_mc", "dZ Neutral vertex - MC",
                    1000, -500 , 500, "Z (cm)", 
                    "#events" );

            h_mc_z_ = hs_.MakeTH1D(
                    "h_mc_z_", "MC Z Vertex",
                    1000, -5000 , 10000, "Z (cm)", 
                    "#events" );

            h_dz_neut_mc_vs_z_ = hs_.MakeTH2D(
                    "h_dz_neut_mc_vs_z_", "dZ vs mc Z",
                    1000, -5000, 10000, "mc Z(cm)", 
                    100, -500, 500, "dz (cm)" );
        }
    }

    void K2piPlotter::process_event()
    {
        //Data only plots
        const K2piRecoEvent & re = 
            k2pi_reco_.get_reco_event();

        double neutral_z = re.get_zvertex();

        h_m2_pip_lkr_->Fill
            ( re.get_m2pip() , get_weight() );

        h_m2_pi0->Fill
            ( re.get_m2pi0() , get_weight() );

        h_neutral_z_->Fill
            ( neutral_z, get_weight() );

        //MC Plots
        if (!mc_){ return;}

        double mc_z =    e_->mc.decay.decay_vertex.Z() ;

        h_dz_neut_mc_->Fill( neutral_z - mc_z, get_weight() );
        h_mc_z_->Fill(  mc_z, get_weight() );

        h_dz_neut_mc_vs_z_->Fill
            ( mc_z, neutral_z - mc_z, get_weight() );
    }

    void K2piPlotter::end_processing()
    {
        TProfile * dzProfile =
            h_dz_neut_mc_vs_z_->ProfileX( "h_dz_profile");

        cd_p( &tfile_, folder_ );
        dzProfile->Write();
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<K2piPlotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const fne::Event * event = rf.get_event_ptr();

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_yaml<std::string>( instruct, "folder") ;

            const K2piReco * k2pi_reco = get_k2pi_reco( instruct, rf );

            bool mc = rf.is_mc();

            return new K2piPlotter
                ( *sel, event,  tfile, folder, *k2pi_reco, mc );
        }
}
