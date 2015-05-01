#include "MuonEfficiency.hh"
#include "muon_selections.hh"
#include "tracking_selections.hh"
#include "SingleTrack.hh"
#include "NA62Constants.hh"
#include "TEfficiency.h"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( MuonEfficiency );

    MuonEfficiency::MuonEfficiency(  const Selection& sel,
            const Selection& mr, const SingleTrack& st,
            TFile& tf, std::string folder )
        :Analysis( sel ),mr_( mr ), st_( st ), tf_( tf), folder_(folder )
    {
        h_xy_passed_ = hs_.MakeTH2D( "h_xy_passed", "XY at MUV Z ( Passed )",
                150, -150, 150, "X (cm)",
                150, -150, 150, "Y (cm)");

        h_xy_total_ = hs_.MakeTH2D( "h_xy_total", "XY at MUV Z ( Total )",
                150, -150, 150, "X (cm)",
                150, -150, 150, "Y (cm)");

        h_p_passed_ = hs_.MakeTH1D( "h_p_passed", "P ( Passed )",
                100, 0, 100, "Momentum (GeV)" ); 

        h_p_total_ = hs_.MakeTH1D( "h_p_total", "P ( Total )",
                100, 0, 100, "Momentum (GeV)" ); 

    }

    void MuonEfficiency::process_event()
    {
        const SingleRecoTrack& srt = st_.get_single_track();

        TVector3 impact_1 = srt.extrapolate_ds( na62const::zMuv1 );
        TVector3 impact_2 = srt.extrapolate_ds( na62const::zMuv2 );

        double x = impact_2.X();
        double y = impact_2.Y();
        double p = srt.get_mom();

        double weight = get_weight();
        bool found_muon = mr_.check();

        if ( found_muon )
        {
            weight *= found_muon;
            h_xy_passed_->Fill(  x, y , weight );
            h_p_passed_->Fill( p , weight );
        }

        h_xy_total_->Fill(  x, y , weight );
        h_p_total_->Fill( p , weight );
    }

    void MuonEfficiency::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();

        TEfficiency muv_eff_xy( *h_xy_passed_, *h_xy_total_ );
        TEfficiency muv_eff_p( *h_p_passed_, *h_p_total_ );

        muv_eff_xy.Write( "muv_eff_xy" );
        muv_eff_p.Write( "muv_eff_p" );
    }

    template<>
        Subscriber * create_subscriber<MuonEfficiency>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv_sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "muv_selection" ) );

            const SingleTrack* st = get_single_track( instruct, rf );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            return new MuonEfficiency( *sel, *muv_sel, *st, tfile, folder );
        }
}
