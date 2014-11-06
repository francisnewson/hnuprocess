#include "Km2Plotter.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( Km2Plotter);

    Km2Plotter::Km2Plotter( const Selection& sel, 
            TFile& tfile, std::string folder,
            const Km2Event& km2_event)
        :Analysis( sel), tfile_(tfile),folder_( folder ), km2_event_( km2_event )
    {
        h_m2miss_ = hs_.MakeTH1D( "h_m2miss", "K_{#mu2} missing mass",
                10000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                "#events" ); 

        h_m2pimiss_ = hs_.MakeTH1D( "h_m2pimiss", 
                "K_{#mu2} missing mass ( pion assumption )",
                10000, -0.7, 0.3, "m^{2}_{miss} ( GeV^{2}/ c^{4} )",
                "#events" ); 

        h_m2pivsk_ = hs_.MakeTH2D( "h_m2pivsk_", 
                "Missing mass ( Pi vs K )",
                100, -0.7, 0.3, "Kaon asusmption GeV^{2}",
                100, -0.7, 0.3, "Pion asusmption GeV^{2}" );

        h_pz_ = hs_.MakeTH2D( "h_pz", "P vs Z" ,
                100, 0, 100, "P( GeV )",
                150, -5000, 10000, "Z( cm) ");

        h_pm2pi_ = hs_.MakeTH2D( "h_pm2pi", "M2(pi)  vs p_muon",
                100, 0, 100, "P( GeV )",
                100, -0.7, 0.3, "M2, Pion asusmption GeV^{2}" );

        h_pm2mu_ = hs_.MakeTH2D( "h_pm2mu", "M2(#mu)  vs p_muon",
                100, 0, 100, "P( GeV )",
                100, -0.7, 0.3, "M2, Muon asusmption GeV^{2}" );

        h_cda_ = hs_.MakeTH1D( "h_cda", "CDA (cm)",
                100, 0, 10, "CDA (cm)" );
    }

    void Km2Plotter::process_event()
    {
        const Km2RecoEvent& km2re = km2_event_.get_reco_event();

        double m2_mu = km2re.get_m2miss();
        double m2_pi = km2re.get_m2pimiss();
        double wgt = get_weight();
        double p_mu = km2re.get_muon_mom();
        double zvert = km2re.get_zvertex();
        double cda = km2re.get_cda();

        h_m2miss_->Fill( m2_k, wgt );
        h_m2pimiss_->Fill( m2_pi , wgt );
        h_pz_->Fill( p_mu, zvert, wgt );
        h_m2pivsk_->Fill( m2_k, m2_pi, wgt );
        h_pm2pi_->Fill( p_mu, m2_pi, wgt );
        h_pm2mu_->Fill( p_mu, m2_mu, wgt );
        h_cda_->Fill( cda, wgt );
    }

    void Km2Plotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2Plotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = 
                get_yaml<std::string>( instruct, "folder" );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            return new Km2Plotter( *sel, tfile, folder, *km2_event);
        }

}
