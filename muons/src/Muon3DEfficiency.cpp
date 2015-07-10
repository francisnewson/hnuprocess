#include "Muon3DEfficiency.hh"
#include "tracking_selections.hh"
#include "SingleTrack.hh"
#include "NA62Constants.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
namespace fn
{
            REG_DEF_SUB( Muon3DEfficiency );

    Muon3DEfficiency::Muon3DEfficiency( const Selection& sel,
            const Selection& mr,
            const SingleTrack& st,
            TFile& tf, std::string folder )
        :Analysis( sel ),mr_( mr ), st_( st ), tf_( tf), folder_(folder )
    {
        h_xyp_passed_ = hs_.MakeTH3D( "h_xyp_passed",
                "MUV events in x, y and p (PASSED)",
                150, -150, 150, "x (cm)",
                150, -150, 150, "y (cm)",
                100, 0, 100, "p (GeV)" );

        h_xyp_total_ = hs_.MakeTH3D( "h_xyp_total",
                "MUV events in x, y and p (TOTAL)",
                150, -150, 150, "x (cm)",
                150, -150, 150, "y (cm)",
                100, 0, 100, "p (GeV)" );
    }

    void Muon3DEfficiency::process_event()
    {
        const SingleRecoTrack& srt = st_.get_single_track();

        TVector3 impact_1 = srt.extrapolate_ds( na62const::zMuv1 );
        TVector3 impact_2 = srt.extrapolate_ds( na62const::zMuv2 );

        double x = impact_2.X();
        double y = impact_1.Y();
        double p = srt.get_mom();

        double weight = get_weight();

        //Denominator does not get reweighted
        h_xyp_total_->Fill(  x, y , p, weight );

        bool found_muon = mr_.check();

        if ( found_muon )
        {
            //numerator does get reweighted
            weight *= mr_.get_weight();
            h_xyp_passed_->Fill(  x, y , p, weight );
        }
    }

    void Muon3DEfficiency::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<Muon3DEfficiency>
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

        return new Muon3DEfficiency( *sel, *muv_sel, *st, tfile, folder );
    }

}
