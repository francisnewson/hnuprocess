#include "K2piPlotter.hh"
#include "yaml_help.hh"

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
                1000, -5000 , 10000, "Z (cm)", 
                "#events" );

        if (mc_)
        {
            h_dz_neut_mc_ = hs_.MakeTH1D(
                    "h_dz_neut_mc", "dZ Neutral vertex - MC",
                    1000, -5000, 10000, "dZ (cm)",
                "#events" );
        }

    }

    void K2piPlotter::process_event()
    {
        //Data only plots
        const K2piRecoEvent & re = 
            k2pi_reco_.get_reco_event();

        h_m2_pip_lkr_->Fill
            ( re.get_m2pip() , get_weight() );

        h_m2_pi0->Fill
            ( re.get_m2pi0() , get_weight() );

        h_neutral_z_->Fill
            ( re.get_zvertex(), get_weight() );

        //MC Plots
        if (!mc_){ return;}

        h_dz_neut_mc_->Fill(  e_->mc.decay.decay_vertex.Z() );
    }

    void K2piPlotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
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
