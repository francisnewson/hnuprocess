#include "MuonShapes.hh"
#include "SingleTrack.hh"
#include "SingleMuon.hh"
#include "yaml_help.hh"
#include "Xcept.hh"
#include "tracking_selections.hh"
#include "NA62Constants.hh"

namespace fn
{

    REG_DEF_SUB( MuonShapes );

    MuonShapes::MuonShapes(const Selection& sel,  const SingleTrack& st, const SingleMuon & sm,
            TFile& tf, std::string folder )
        :Analysis(sel), st_(st), sm_( sm ), tf_( tf ), folder_( folder )
    {
        h_xy_ = hs_.MakeTH2D( "h_xy", "Muon positions", 
                150, -150, 150, "x (cm)",
                150, -150, 150, "y (cm)" );

        h_dxy_ = hs_.MakeTH2D( "h_dxy", "Muon - Track separation", 
                150, -150, 150, "x (cm)",
                150, -150, 150, "y (cm)" );

        h_dsvsp_ = hs_.MakeTH2D( "h_dsvsp", "Muon - Track separation vs momentum", 
                150, 0, 150, "s (cm)",
                100, 0, 100, "p (GeV)" );

        h_ds_ = hs_.MakeTH1D( "h_ds_", "Muon - Track separation [Magnitude]",
                150, 0, 150, "s (cm)" );

        h_dx_ = hs_.MakeTH1D( "h_dx_", "Muon - Track separation [X]",
                150, -150, 150, "x (cm)" );

        h_dy_ = hs_.MakeTH1D( "h_dy_", "Muon - Track separation [Y]",
                150, -150, 150, "y (cm)" );
    }

    void MuonShapes::process_event()
    {
        //Check and load track and muon
        auto& srt = st_.get_single_track();
        bool found_muon = sm_.found();

        if ( !found_muon ){ return; }
        double weight = get_weight() * sm_.weight();

        double mom = srt.get_mom();

        //Determine track and muon coordinates
        double muon_x = sm_.x();
        double muon_y = sm_.y();

        double track_x = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double track_y = srt.extrapolate_ds( na62const::zMuv1 ).y();

        //Extract separations
        double dx = muon_x - track_x;
        double dy = muon_y - track_y;
        double ds = std::hypot( dx, dy );

        //Plot
        h_xy_->Fill( muon_x, muon_y, weight );
        h_dxy_->Fill( dx, dy, weight );
        h_dsvsp_->Fill( ds, mom, weight );
        h_ds_->Fill( ds );
        h_dx_->Fill( dx );
        h_dy_->Fill( dy );
    }

    void MuonShapes::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<MuonShapes>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile(
                    get_yaml<std::string>( instruct, "tfile" ));

            std::string folder = get_folder ( instruct, rf );

            const SingleTrack * st = get_single_track( instruct, rf );
            const SingleMuon * sm = get_single_muon( instruct, rf );

            return new MuonShapes( *sel, *st, *sm, tfile, folder );
        }
}
