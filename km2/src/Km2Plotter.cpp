#include "Km2Plotter.hh"

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

             h_pz_ = hs_.MakeTH2D( "h_pz", "P vs Z" ,
                     100, 0, 100, "P( GeV )",
                     150, -5000, 10000, "Z( cm) ");
    }

    void Km2Plotter::process_event()
    {
        const Km2RecoEvent& km2re = km2_event_.get_reco_event();
        h_m2miss_->Fill( km2re.get_m2miss(), get_weight() );
        h_pz_->Fill( km2re.get_muon_mom(), km2re.get_zvertex(), get_weight() );
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
                    instruct["selection"].as<std::string>() );

            TFile & tfile = rf.get_tfile( 
                    instruct["tfile"].as<std::string>() );

            std::string folder = instruct["folder"].as<std::string>();

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            return new Km2Plotter( *sel, tfile, folder, *km2_event);
        }

}
