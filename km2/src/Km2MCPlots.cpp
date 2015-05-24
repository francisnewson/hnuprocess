#include "Km2MCPlots.hh"
#include "Km2Reco.hh"
#include "mc_help.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{

    REG_DEF_SUB( Km2MCPlots );

    Km2MCPlots::Km2MCPlots( 
            const Selection& sel, 
            const Km2Event& raw_km2e, 
            const Km2Event& corr_km2e, 
            const fne::Event * e,
            TFile& tfile,
            std::string folder
            )
        :Analysis( sel ),raw_km2e_( raw_km2e), corr_km2e_( corr_km2e ),e_( e ),
        tfile_( tfile ), folder_( folder ),
        raw_mc_( 0.1), corr_mc_( 0.1), corr_raw_( 0.001, 0.1),
        muon_raw_mc_( 0.1), muon_corr_mc_( 0.1), muon_corr_raw_( 0.001, 0.1)
    {
        h_dz_raw_mc_ = hs_.MakeTH1D( "h_dz_raw_mc", "#Delta z_vertex",
                1000, -1000, 1000, "z(cm)" );

        h_dz_corr_mc_ = hs_.MakeTH1D( "h_dz_corr_mc", "#Delta z_vertex",
                1000, -1000, 1000, "z(cm)" );

        h_dz_corr_raw_  = hs_.MakeTH1D( "h_dz_corr_raw", "#Delta z_vertex",
                1000, -100, 100, "z(cm)" );


        h_dcda_raw_mc_ = hs_.MakeTH1D( "h_dcda_raw_mc", "#Delta cda",
                1000, -10, 10, "z(cm)" );

        h_dcda_corr_mc_ = hs_.MakeTH1D( "h_dcda_corr_mc", "#Delta cda",
                1000, -10, 10, "z(cm)" );

        h_dcda_corr_raw_ = hs_.MakeTH1D( "h_dcda_corr_raw", "#Delta cda",
                1000, -10, 10, "z(cm)" );

        h_cda_raw_= hs_.MakeTH1D( "h_cda_raw", "#Delta cda",
                1000, -10, 10, "z(cm)" );

        h_cda_corr_= hs_.MakeTH1D( "h_cda_corr", "#Delta cda",
                1000, -10, 10, "z(cm)" );

        h_cda_mc_= hs_.MakeTH1D( "h_cda_mc", "#Delta cda",
                1000, -10, 10, "z(cm)" );

    }

    void Km2MCPlots::process_event()
    {

        const Km2RecoEvent& raw_reco = raw_km2e_.get_reco_event();
        auto raw_hnu_4mom = raw_reco.get_p4_miss_kmu();
        auto raw_mu_4mom = raw_reco.get_p4_mu();
        auto kaon_4mom = raw_reco.get_p4_k();
        double raw_z = raw_reco.get_zvertex();
        double raw_cda = raw_reco.get_cda();

        const Km2RecoEvent& corr_reco = corr_km2e_.get_reco_event();
        auto corr_hnu_4mom = corr_reco.get_p4_miss_kmu();
        auto corr_mu_4mom = corr_reco.get_p4_mu();
        double corr_z = corr_reco.get_zvertex();
        double corr_cda = corr_reco.get_cda();

        boost::optional<mc_km2_event> mc_event = get_mc_km2( e_->mc );

        double wgt = get_weight();
        corr_raw_.Fill( corr_hnu_4mom , raw_hnu_4mom, wgt );
        muon_corr_raw_.Fill( corr_mu_4mom, raw_mu_4mom, wgt );
        muon_corr_raw_.FillM2( corr_mu_4mom, raw_mu_4mom, kaon_4mom, wgt );

        h_dz_corr_raw_->Fill( corr_z - raw_z, wgt );

        h_cda_raw_->Fill( raw_cda,  wgt );
        h_cda_corr_->Fill( corr_cda, wgt );
        h_dcda_corr_raw_->Fill( corr_cda - raw_cda, wgt );

        if ( mc_event )
        {
            TLorentzVector p4k = mc_event->kaon->momentum;
            TLorentzVector p4mu = mc_event->muon->momentum;
            TLorentzVector mc_hnu = p4k - p4mu;

            TVector3 mc_vertex = mc_event->kaon->decay_vertex;

            raw_mc_.Fill( raw_hnu_4mom , mc_hnu, wgt );
            corr_mc_.Fill( corr_hnu_4mom , mc_hnu,  wgt );

            muon_raw_mc_.Fill( raw_mu_4mom, p4mu, wgt );
            muon_corr_mc_.Fill( corr_mu_4mom, p4mu, wgt );

            h_dz_raw_mc_->Fill( raw_z - mc_vertex.Z(), wgt );
            h_dz_corr_mc_->Fill( corr_z - mc_vertex.Z(), wgt );
        }
    }

    void Km2MCPlots::end_processing()
    {
        cd_p( &tfile_, folder_ / "hnu_raw_mc" );
        raw_mc_.Write();

        cd_p( &tfile_, folder_ / "hnu_corr_mc" );
        corr_mc_.Write();

        cd_p( &tfile_, folder_ / "hnu_corr_raw" );
        corr_raw_.Write();

        cd_p( &tfile_, folder_ / "mu_raw_mc" );
        muon_raw_mc_.Write();

        cd_p( &tfile_, folder_ / "mu_corr_mc" );
        muon_corr_mc_.Write();

        cd_p( &tfile_, folder_ / "mu_corr_raw" );
        muon_corr_raw_.Write();

        cd_p( &tfile_, folder_ / "extra" );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2MCPlots>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* raw_km2_event = get_km2_event( instruct, rf, "raw_km2event" );
            const Km2Event* corr_km2_event = get_km2_event( instruct, rf, "corr_km2event" );

            const fne::Event * event = rf.get_event_ptr();

            return new Km2MCPlots( *sel, *raw_km2_event, *corr_km2_event,
                    event, tfile, folder );
        }

    boost::optional<mc_km2_event> get_mc_km2( const fne::Mc& mc )
    {
        boost::optional<mc_km2_event> result = boost::none;

        std::vector<const fne::McParticle*> kaons;
        std::vector<const fne::McParticle*> muons;
        std::vector<const fne::McParticle*> photons;

        const TClonesArray& tca =  mc.particles;
        int nparticles = mc.npart;

        for ( int ip = 0; ip != nparticles ; ++ip )
        {
            const fne::McParticle* particle = 
                static_cast<const fne::McParticle*>( tca[ip]  );

            using namespace cmc;

            if ( particle->type == photon_type )
            { photons.push_back(particle ); }

            if ( particle->type == muon_type )
            { muons.push_back(particle); }

            if ( particle->type == charged_kaon_type )
            {  kaons.push_back(particle); }
        }

        int particles_found = kaons.size() + muons.size() + photons.size();

        if ( particles_found != nparticles ){ return result ; }

        if ( (kaons.size() != 1) || (muons.size() != 1) )
        {
            return result;
        }

        result =  mc_km2_event{ kaons.front() , muons.front() };
        return result;
    }

}
