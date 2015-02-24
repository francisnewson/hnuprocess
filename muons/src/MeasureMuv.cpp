#include "MeasureMuv.hh"
#include "NA62Constants.hh"
#include "TFile.h"
#include "TEfficiency.h"
#include "SingleTrack.hh"
#include "MuonVeto.hh"
#include "tracking_selections.hh"
#include "yaml_help.hh"
namespace fn
{
    REG_DEF_SUB( MeasureMuv);

    MeasureMuv::MeasureMuv( const Selection& base, const Selection& muv, 
            const SingleTrack& st, const MuonVeto& mv,
            TFile& tfile, std::string folder
            )
        :Analysis( base ), muv_( muv ), st_( st ), mv_( mv ),
        tfile_( tfile), folder_( folder)
    {
        h_xy_passed_ = hs_.MakeTH2D( "h_xy_passed", "XY at MUV Z ( Passed )",
                150, -150, 150, "X (cm)",
                150, -150, 150, "Y (cm)");

        h_xy_total_ = hs_.MakeTH2D( "h_xy_total", "XY at MUV Z ( Total )",
                150, -150, 150, "X (cm)",
                150, -150, 150, "Y (cm)");

        h_r_passed_ = hs_.MakeTH1D( "h_r_passed", "R at MUV Z ( Passed )",
                250, 0, 250, "Radius (cm)" ); 

        h_r_total_ = hs_.MakeTH1D( "h_r_total", "R at MUV Z ( Total )",
                250, 0, 250, "Radius (cm)" ); 

        h_p_passed_ = hs_.MakeTH1D( "h_p_passed", "P ( Passed )",
                100, 0, 100, "Momentum (GeV)" ); 

        h_p_total_ = hs_.MakeTH1D( "h_p_total", "P ( Total )",
                100, 0, 100, "Momentum (GeV)" ); 

        h_p_r_ = hs_.MakeTH2D( "h_p_r", "Momentum and radius" ,
                100, 0, 100, "Momentum (GeV)",
                250, 0, 250, "Radius (cm)" );

        h_track_muon_separation_ = hs_.MakeTH2D( "h_track_muon_separation",
                "Track-Muon separation",
                99, -198, 198, "X (cm)",
                99, -198, 198, "Y (cm)");

        h_track_muon_separation_passed_ = hs_.MakeTH2D( "h_track_muon_separation_passed",
                "Track-Muon separation (passed)",
                99, -198, 198, "X (cm)",
                99, -198, 198, "Y (cm)");

        h_track_muv_separation_ = hs_.MakeTH2D( "h_track_muv_separation",
                "Track-Muon separation",
                99, -198, 198, "X (cm)",
                99, -198, 198, "Y (cm)");

        h_track_muv_separation_passed_ = hs_.MakeTH2D( "h_track_muv_separation_passed",
                "Track-Muon separation (passed)",
                99, -198, 198, "X (cm)",
                99, -198, 198, "Y (cm)");

        h_muon_pos_ = hs_.MakeTH2D( "h_muon_pos",
                "Muon at MUV",
                99, -396, 396, "X (cm)",
                99, -396, 396, "Y (cm)");
    }

    void MeasureMuv::process_event()
    {
        double wgt = get_weight();

        double average_muv_z = 0.5 * ( na62const::zMuv1 + na62const::zMuv2 );
        const SingleRecoTrack& srt = st_.get_single_track();

        TVector3 impact_1 = srt.extrapolate_ds( na62const::zMuv1 );
        TVector3 impact_2 = srt.extrapolate_ds( na62const::zMuv2 );

        double x = impact_2.X();
        double y = impact_1.Y();
        double r = std::hypot( x, y );
        double p = srt.get_mom();

        h_xy_total_->Fill( x, y, wgt) ;
        h_r_total_->Fill( r, wgt );
        h_p_total_->Fill( p, wgt );

        h_p_r_->Fill( p, r, wgt );

        if ( mv_.found_muon() )
        {
            TVector3 muv_pos = mv_.get_muon_position();

            h_muon_pos_->Fill(  muv_pos.X(), muv_pos.Y(), wgt );

            h_track_muon_separation_->Fill( 
                    x - muv_pos.X(), 
                    y - muv_pos.Y(), 
                    wgt );

            if ( muv_.check() )
            {
            h_track_muon_separation_passed_->Fill( 
                    x - muv_pos.X(), 
                    y - muv_pos.Y(), 
                    wgt );
            }
        }

        if ( muv_.check() )
        {
            h_xy_passed_->Fill( x, y );
            h_r_passed_->Fill( r );
            h_p_passed_->Fill( p );

        }
    }

    void MeasureMuv::end_processing()
    {
        TEfficiency muv_eff_xy( *h_xy_passed_, *h_xy_total_ );
        TEfficiency muv_eff_r( *h_r_passed_, *h_r_total_ );
        TEfficiency muv_eff_p( *h_p_passed_, *h_p_total_ );

        cd_p( &tfile_, folder_ );
        hs_.Write();
        muv_eff_xy.Write( "muv_eff_xy" );
        muv_eff_r.Write( "muv_eff_r" );
        muv_eff_p.Write( "muv_eff_p" );
    }

    template<>
        Subscriber * create_subscriber<MeasureMuv>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv_sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "muv_selection" ) );

            const MuonVeto*  mv = get_muon_veto( instruct, rf );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const SingleTrack * st = get_single_track( instruct, rf );

            return new MeasureMuv( *sel, *muv_sel, *st, *mv, tfile, folder );
        }
}

