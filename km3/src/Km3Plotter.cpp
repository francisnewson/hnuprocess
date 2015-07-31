#include "Km3Plotter.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"
#include "RecoFactory.hh"
namespace fn
{
    Km3Plots::Km3Plots()
    {
        h_m2m_ = hs_.MakeTH1D( "h_m2m", "Km3 m2m",1000,  -0.5, 0.5, "m2m (GeV^2/c^4)" );
        h_ptm_ = hs_.MakeTH1D( "h_ptm_", "Km3 p_T",1000, 0, 5, "ptm (GeV/c)" );
        h_pm_ = hs_.MakeTH1D( "h_pm_", "Km3 p",1000,  0, 100, "pm (GeV/c)" );
        h_dv_ = hs_.MakeTH1D( "h_dv_", "Km3 dv",1000,  -5000, 5000, "dv (cm)" );
        h_z_ = hs_.MakeTH1D( "h_z_", "Km3 z( charged )",1200,  -2000, 10000, "dv (cm)" );

        h_eop_ = hs_.MakeTH1D( "h_eop_", "Km3 E/P",1000,  0, 2, "eop" );

        h_cda_ = hs_.MakeTH1D( "h_cda_", "Km3 CDA",1000,  -5, 5, "CDA (cm)" );

        h_m2_vs_p_m_ = hs_.MakeTH2D( "h_m2_vs_p_m_", "Km3 ( pm vs m2m )", 
                100, -0.5, 0.5, "m2m", 100, 0, 100, "pm" );

        h_m2_vs_pt_m_ = hs_.MakeTH2D( "h_m2_vs_pt_m_", "Km3 ( pt vs m2m )", 
                100, -0.5, 0.5, "m2m", 100, 0, 0.5, "pt" );
    }

    void Km3Plots::Fill( const Km3RecoEvent& re, double wgt )
    {
        h_m2m_->Fill( re.get_m2m_miss(), wgt );
        h_ptm_->Fill( re.get_pt_miss(), wgt );
        h_pm_->Fill( re.get_p_miss(), wgt );
        h_dv_->Fill( re.get_neutral_vertex().Z() - re.get_charged_vertex().Z() , wgt );
        h_z_->Fill(  re.get_charged_vertex().Z() , wgt );
        h_eop_->Fill(  re.get_eop() , wgt );
        h_cda_->Fill(  re.get_cda() , wgt );
        h_m2_vs_p_m_->Fill( re.get_m2m_miss(), re.get_p_miss(), wgt );
        h_m2_vs_pt_m_->Fill( re.get_m2m_miss(), re.get_pt_miss(), wgt );
    }

    void Km3Plots::Write()
    {
        hs_.Write();
    }

    //--------------------------------------------------
    REG_DEF_SUB( Km3Plotter );

    Km3Plotter::Km3Plotter( const Selection& sel, 
            TFile& tfile, std::string folder,
            const Km3Reco& km3_reco )
        :Analysis( sel), tfile_(tfile),folder_( folder ), km3_reco_( km3_reco )
    {
    }

    void Km3Plotter::process_event()
    {
        BOOST_LOG_SEV( get_log(), log_level() )
            << "In Km3Plotter::process_event()";

        if ( km3_reco_.found_km3_event() )
        {
            const Km3RecoEvent& km3re = km3_reco_.get_reco_event();
            plots_.Fill( km3re, get_weight() );
        }
    }

    void Km3Plotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        plots_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km3Plotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km3Reco* km3_reco = get_km3_reco( instruct, rf );

            return new Km3Plotter( *sel, tfile, folder, *km3_reco );
        }
}
