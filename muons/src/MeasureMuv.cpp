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
        h_xy_passed_ = hs_.MakeTH2D( "hx_xy_passed", "XY at Average MUV Z ( Passed )",
                150, -150, 150, "X (cm)",
                150, -150, 150, "Y (cm)");

        h_xy_total_ = hs_.MakeTH2D( "hx_xy_total", "XY at Average MUV Z ( Total )",
                150, -150, 150, "X (cm)",
                150, -150, 150, "Y (cm)");
    }

    void MeasureMuv::process_event()
    {
        double average_muv_z = 0.5 * ( na62const::zMuv1 + na62const::zMuv2 );
        const SingleRecoTrack& srt = st_.get_single_track();
        TVector3 impact_point = srt.extrapolate_ds( average_muv_z );
        h_xy_total_->Fill( impact_point.X(), impact_point.Y() );

        if ( muv_.check() )
        {
            h_xy_passed_->Fill( impact_point.X(), impact_point.Y() );
        }
    }

    void MeasureMuv::end_processing()
    {
        TEfficiency muv_eff( *h_xy_passed_, *h_xy_total_ );
        cd_p( &tfile_, folder_ );
        hs_.Write();
        muv_eff.Write( "muv_eff_xy" );
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

