#include "MeasureMuv.hh"
#include "NA62Constants.hh"
#include "TFile.h"
#include "TEfficiency.h"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "yaml_help.hh"
namespace fn
{
    REG_DEF_SUB( MeasureMuv);

    MeasureMuv::MeasureMuv( const Selection& base, const Selection& muv, 
            const SingleTrack& st,
            TFile& tfile, std::string folder
            )
        :Analysis( base ), muv_( muv ), st_( st ),
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
    }

    void MeasureMuv::process_event()
    {
        double average_muv_z = 0.5 * ( na62const::zMuv1 + na62const::zMuv2 );
        const SingleRecoTrack& srt = st_.get_single_track();

        TVector3 impact_1 = srt.extrapolate_ds( na62const::zMuv1 );
        TVector3 impact_2 = srt.extrapolate_ds( na62const::zMuv2 );

        double x = impact_2.X();
        double y = impact_1.Y();
        double r = std::hypot( x, y );
        double p = srt.get_mom();

        h_xy_total_->Fill( x, y) ;
        h_r_total_->Fill( r );
        h_p_total_->Fill( p );

        h_p_r_->Fill( p, r );

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

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const SingleTrack * st = get_single_track( instruct, rf );

            return new MeasureMuv( *sel, *muv_sel, *st, tfile, folder );
        }
}

