#include "MuvEff.hh"
#include "Selection.hh"
#include "Km2Reco.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( MuvEff );

    MuvPlots::MuvPlots()
    {
        h_mom_ = hs_.MakeTH1D( "h_mom", "Muon momentum",
                100, 0, 100, "Momentum (GeV/c)" );

        h_m2miss_ = hs_.MakeTH1D( "h_m2miss", "Reconstructed m^{2}_{miss}",
                100, -0.02, 0.02, "m^{2}_{miss} (GeV^{2}/c^{4})" );

        h_m2p_ = hs_.MakeTH2D( "h_m2p", "Momentum vs m^{2}_{miss}",
                100, -0.02, 0.02, "m^{2}_{miss} (GeV^{2}/c^{4})" ,
                100, 0, 100, "Momentum (GeV/c)" );

        h_pz_  = hs_.MakeTH2D( "h_pz", "Z vs Momentum",
                100, 0, 100, "Momentum (GeV/c)",
                120, -2000, 10000, "Z vertex ( cm )");
    }

    void MuvPlots::Fill( Km2RecoEvent& km2re, double weight )
    {
        h_mom_->Fill( km2re.get_muon_mom(), weight );
        h_m2miss_->Fill( km2re.get_m2miss(), weight );
        h_m2p_->Fill( km2re.get_m2miss(), km2re.get_muon_mom(), weight );
        h_pz_->Fill( km2re.get_muon_mom(), km2re.get_zvertex() );
    }

    void MuvPlots::Write()
    {
        hs_.Write();
    }

    //--------------------------------------------------

    MuvEff::MuvEff( const Selection& sel, const Selection& muv_selection,
            const Km2Event& km2_event,
            TFile& tfile, std::string folder)
        :Analysis( sel), muv_selection_( muv_selection),
        km2e_( km2_event), tfile_( tfile), folder_( folder),
        eff_mom_( "eff_mom", "Eff( muon momentum)", 100, 0, 100 )
    {
        eff_mom_.SetDirectory( 0 );
    }

    void MuvEff::process_event()
    {
        Km2RecoEvent km2re = km2e_.get_reco_event();
        double weight = get_weight();

        all_plots_.Fill( km2re, weight );

        bool pass = muv_selection_.check() ;

        if  (pass)
        {
            pass_plots_.Fill( km2re, weight );
        }
        else
        {
            fail_plots_.Fill( km2re, weight );
        }
            eff_mom_.Fill( pass, km2re.get_muon_mom() );
    }

    void MuvEff::end_processing()
    {
        cd_p( &tfile_, folder_ + "/all");
        all_plots_.Write();

        cd_p( &tfile_, folder_ + "/pass");
        pass_plots_.Write();

        cd_p( &tfile_, folder_ + "/fail");
        fail_plots_.Write();

        cd_p( &tfile_, folder_ );
        eff_mom_.Write();
    }

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<MuvEff>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv_sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "muv_selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = 
                get_yaml<std::string>( instruct, "folder" );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            return new MuvEff( *sel, *muv_sel, *km2_event, tfile, folder);
        }


}
