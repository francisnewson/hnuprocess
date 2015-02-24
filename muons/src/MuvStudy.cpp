#include "MuvStudy.hh"
#include "Event.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "tracking_selections.hh"
#include "mc_help.hh"
#include "stl_help.hh"
#include "TGraph.h"
#include "RecoMuon.hh"
#include "NA62Constants.hh"


namespace fn
{
    REG_DEF_SUB(MuvStudy);

    MuvStudy::MuvStudy( const Selection& sel, const fne::Event * e,
            const SingleTrack& st, TFile& tfile, std::string folder)
        :Analysis( sel ), e_( e ), st_( st ),
        tfile_( tfile ), folder_( folder ),
        early_( "early_p_eff", "Early efficiency", 100, 0, 100),
        mid_( "mid_p_eff", "Early efficiency", 100, 0, 100),
        late_( "late_p_eff", "Early efficiency", 100, 0, 100)

    {
        h_det_nmuon_ = hs_.MakeTH1D(  "h_det_nmuon", "Number of muon objects",
                10, -0.5, 9.5, "Number of muons objects");
        h_mc_nmuon_ = hs_.MakeTH1D(  "h_mc_nmuon", "Number of MC muons",
                10, -0.5, 9.5, "Number of muons");
    }

    void MuvStudy::process_event()
    {
        double wgt = get_weight();
        const auto& reco_track = st_.get_single_track();
        int track_muon_id = reco_track.get_muon_id();
        double momentum = reco_track.get_mom();

        const fne::RecoMuon * track_muon = 0;
        bool found_track_muon = false;
        bool found_good_track_muon = false;

        //check track muon consistency
        if ( track_muon_id != -1 )
        {
            found_track_muon  = true;
            track_muon  = static_cast<fne::RecoMuon*>
                ( e_->detector.muons.At( track_muon_id ) );

            int muon_track_id = track_muon->iTrk;
            assert( muon_track_id == reco_track.get_compact_id() );

            if ( track_muon->status == 1 || track_muon->status == 2 )
            {
                found_good_track_muon = true;
            }
        }

        //data
        unsigned int nmuons = e_->detector.nmuons;
        h_det_nmuon_->Fill(nmuons);

        std::vector<const fne::RecoMuon*> muv12;
        find_muv12( e_->detector , muv12 );



        //mc muon
        const fne::McParticle * mc_muon = get_mc_muon( e_->mc );
        int n_mc_muon = ( mc_muon == 0 ) ? 0 : 1 ;

        if ( n_mc_muon == 1 )
        {

            double z_mu_prod = mc_muon->production_vertex.Z();
            bool early_decay = z_mu_prod < na62const::zDch1 ;
            bool mid_decay = !(early_decay) && z_mu_prod < na62const::zDch3 ;
            bool late_decay = !( early_decay || mid_decay );

            TEfficiency& eff = early_decay ? early_ : mid_decay ? mid_ : late_;
            eff.Fill( found_good_track_muon, mc_muon->momentum.P() );
        }

        h_mc_nmuon_->Fill( n_mc_muon );



        if (nmuons > 1 )
        {
            std::vector<double> xs;
            std::vector<double> ys;
            for ( int imuon = 0 ; imuon != nmuons ; ++imuon )
            {
                const auto * muon = static_cast<const fne::RecoMuon*>(
                        e_->detector.muons.At( imuon ) );

                xs.push_back( muon->x );
                ys.push_back( muon->y );
            }

            auto muon_hits = make_unique<TGraph>( nmuons,  xs.data(), ys.data() );
            muon_hits->SetName( Form( "muon_hits_%d" , e_->header.compact_number ) );
            multi_muons_.push_back( std::move(muon_hits) );

            BOOST_LOG_SEV( get_log(), always_print )
                << "Multi muon " << e_->header.compact_number 
                << " : "  << nmuons << " hits" ;
        }

    }

    void MuvStudy::end_processing()
    {
        cd_p( &tfile_ , folder_ );
        hs_.Write();

        for ( const auto& muon : multi_muons_ )
        {
            muon->Write();
        }

        early_.Write();
        mid_.Write();
        late_.Write();
    }

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<MuvStudy>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const fne::Event * e = rf.get_event_ptr();

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const SingleTrack * st = get_single_track( instruct, rf );

            return new MuvStudy( *sel, e, *st, tfile, folder );
        }

    const fne::McParticle * get_mc_muon( const fne::Mc& mc )
    {
        int n_mc_muon = 0;
        const fne::McParticle * mc_muon = 0;

        for ( int ipart = 0 ; ipart != mc.npart ; ++ipart )
        {
            const auto * part = static_cast<const fne::McParticle*>(
                    mc.particles.At( ipart ) );

            int particle_type = part->type;

            if( particle_type == cmc::muon_type )
            {
                ++n_mc_muon;
                mc_muon = part;
            }
        }

        if ( n_mc_muon == 0 ){ return 0;}
        if ( n_mc_muon == 1 ){ return mc_muon; }
        else throw( std::runtime_error( "More than one muon" ) );

    }

    void find_muv12( const fne::Detector& det,
            std::vector<const fne::RecoMuon*>& res )
    {
        unsigned int nmuons = det.nmuons;
        for ( int imuon = 0 ; imuon != nmuons ; ++imuon )
        {
            const auto * muon = static_cast<const fne::RecoMuon*>(
                    det.muons.At( imuon ) );

            if ( muon->status == 1 or muon->status ==2 )
            {
                res.push_back( muon );
            }
        }
    }
}

