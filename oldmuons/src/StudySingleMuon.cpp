#include "StudySingleMuon.hh"
#include "SingleMuon.hh"
#include "RecoFactory.hh"
#include "SingleTrack.hh"
#include "Km2Reco.hh"
#include "tracking_selections.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"
#include "TFile.h"
#include "RecoMuon.hh"
#include "TEfficiency.h"

namespace fn
{

    REG_DEF_SUB( StudySingleMuon );

    StudySingleMuon::StudySingleMuon
        ( const Selection & sel, const SingleMuon& sm, const SingleTrack& st , 
          const Selection& muv, const Km2Event& km2,
          TFile& tfile, std::string folder )
        :Analysis( sel ), sm_( sm ), st_( st ), muv_( muv ), km2_(km2),
        tfile_( tfile ), folder_(folder )
    {
        h_m2m_ = hs_.MakeTH1D(
                "h_m2m", "Reconstructed missing mass", 
                1000, -0.7, 0.3, "m2m (GeV^2)" );
    }

    void StudySingleMuon::process_event()
    {
        double wgt = get_weight();
        sm_.check_consistency();

        auto & srt = st_.get_single_track();

        const Km2RecoEvent& km2re =  km2_.get_reco_event();

        h_m2m_->Fill(km2re.get_m2m_kmu(), wgt );

        if ( sm_.found_muon() )
        {
            const auto & muon = *sm_.get_muon();

            muon_all_.fill( srt, muon, wgt );

            if ( muv_.check() )
            {
            muon_pass_.fill( srt, muon, wgt );
            }
        }

        all_.fill( srt, wgt );

        if ( muv_.check() )
        {
            pass_.fill( srt, wgt );
            wgtpass_.fill( srt, wgt * muv_.get_weight() );
        }
    }

    void StudySingleMuon::end_processing()
    {
        cd_p( &tfile_, folder_ );
        hs_.Write();

        cd_p( &tfile_, folder_ + "/passed" );
        muon_pass_.write();
        pass_.write();

        cd_p( &tfile_, folder_ + "/all" );
        all_.write();
        muon_all_.write();

        cd_p( &tfile_, folder_ + "/unweighted" );

        auto itall = all_.hs_.begin();
        auto itpass = pass_.hs_.begin() ;

        for ( ; itall != all_.hs_.end() ; ++itall, ++itpass )
        {
            TEfficiency eff( **itpass, **itall );
            eff.Write( (*itall)->GetName() );
        }

        cd_p( &tfile_, folder_ + "/weighted" );

        itall = all_.hs_.begin();
        auto itwgt = wgtpass_.hs_.begin() ;

        for ( ; itall != all_.hs_.end() ; ++itall, ++itwgt )
        {
            TEfficiency eff( **itwgt, **itall );
            eff.Write( (*itall)->GetName() );
        }
    }

    StudySingleMuon::eff_plots::eff_plots()
    {
        hmom_ = hs_.MakeTH1D( "hmom", "Momentum", 
                100, 0, 100, "p ( GeV )" );

        hxy_ = hs_.MakeTH2D( "hxy_", "Track XY at MUV", 
                161 , -161 ,161 , "x ( cm )" ,
                161 , -161 ,161 , "y ( cm )" );

    }

    void StudySingleMuon::eff_plots::fill( const SingleRecoTrack& srt, double wgt)
    {
        double mom = srt.get_mom();

        double track_x = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double track_y = srt.extrapolate_ds( na62const::zMuv1 ).y();

        hmom_->Fill( mom, wgt );
        hxy_->Fill( track_x, track_y , wgt );
    }

    void StudySingleMuon::eff_plots::write()
    {
        hs_.Write();
    }

    StudySingleMuon::muon_plots::muon_plots()
    {
        h_track_muon_sep_ = hs_.MakeTH2D(
                "h_track_muon_sep", "Track muon separation", 
                101, -101, 101, "X (cm)",
                101, -101, 101, "Y (cm)" );

        h_track_muon_dist_ = hs_.MakeTH1D(
                "h_track_muon_dist", "Track muon distance", 
                100, -0.5, 99.5, "d (cm)" );

        h_track_muon_mom_dist_ = hs_.MakeTH2D(
                "h_track_muon_mom_dist", "Track muon distance vs momentum", 
                100, -0.5, 99.5, "distance (cm)",
                100, -0.5, 99.5, "momentum (GeV)" );
    }

    void StudySingleMuon::muon_plots::fill( const SingleRecoTrack& srt,
            const fne::RecoMuon& muon, double wgt )
    {
        double muon_x = muon.x;
        double muon_y = muon.y;

        double track_x = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double track_y = srt.extrapolate_ds( na62const::zMuv1 ).y();

        double dx = muon_x - track_x;
        double dy = muon_y - track_y;
        double ds = std::hypot( dx, dy );

        double mom = srt.get_mom();

        h_track_muon_sep_->Fill( dx, dy, wgt );
        h_track_muon_dist_->Fill( ds, wgt );
        h_track_muon_mom_dist_->Fill( ds, mom, wgt );
    }

    void StudySingleMuon::muon_plots::write()
    {
        hs_.Write();
    }


    template<>
        Subscriber * create_subscriber<StudySingleMuon>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv = rf.get_selection( 
                    get_yaml<std::string>( instruct, "muv" ) );

            const fne::Event * e = rf.get_event_ptr();

            const SingleTrack * st = get_single_track( instruct, rf );
            const SingleMuon * sm = get_single_muon( instruct, rf );

            const Km2Event * km2 = get_km2_event( instruct, rf );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            return new StudySingleMuon( *sel, *sm, *st , *muv, *km2, tfile, folder );
        }
}
