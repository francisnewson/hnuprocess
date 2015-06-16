#include "MuonEfficiency.hh"
#include "muon_selections.hh"
#include "muon_functions.hh"
#include "tracking_selections.hh"
#include "SingleTrack.hh"
#include "NA62Constants.hh"
#include "TEfficiency.h"
#include "yaml_help.hh"
#include "SingleMuon.hh"
#include "TEfficiencyHelper.h"
#include "TGraphAsymmErrors.h"
#include "stl_help.hh"

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

        h_track_pr_ = hs_.MakeTH2D( "h_track_ptr", "Track: momentum vs r at MUV ",
                100, 0, 100, "P (GeV)",
                200, 0, 200, "r cm");

    }

    void MuonEfficiency::process_event()
    {
        const SingleRecoTrack& srt = st_.get_single_track();

        TVector3 impact_1 = srt.extrapolate_ds( na62const::zMuv1 );
        TVector3 impact_2 = srt.extrapolate_ds( na62const::zMuv2 );

        double x = impact_2.X();
        double y = impact_1.Y();
        double p = srt.get_mom();

        double weight = get_weight();

        //Denominator does not get reweighted
        h_xy_total_->Fill(  x, y , weight );
        h_p_total_->Fill( p , weight );
        h_track_pr_->Fill( p, std::hypot( x, y ), weight );

        bool found_muon = mr_.check();

        if ( found_muon )
        {
            //numerator does get reweighted
            weight *= mr_.get_weight();
            h_xy_passed_->Fill(  x, y , weight );
            h_p_passed_->Fill( p , weight );
        }

    }

    void MuonEfficiency::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();

        TH2D * h_xy_ratio = static_cast<TH2D*>
            ( h_xy_passed_->Clone( "h_xy_ratio" ) );
        h_xy_ratio->Sumw2();
        h_xy_total_->Sumw2();
        h_xy_ratio->Divide( h_xy_passed_, h_xy_total_, 1, 1, "B" );
        h_xy_ratio->Write( "h_xy_ratio" );

        TH2D * h_p_ratio = static_cast<TH2D*>
            ( h_p_passed_->Clone( "h_p_ratio" ) );
        h_p_ratio->Sumw2();
        h_p_total_->Sumw2();
        h_p_ratio->Divide( h_p_passed_, h_p_total_, 1, 1, "B" );
        h_p_ratio->Write( "h_p_ratio" );

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

    //--------------------------------------------------

    REG_DEF_SUB( MuonMultiplierEfficiency );

    MuonMultiplierEfficiency::MuonMultiplierEfficiency(const Selection& sel,
            std::vector<double> cut_values,
            const fne::Event * e, bool mc, const SingleTrack& st,
            TFile& tf, std::string folder )
        :Analysis( sel ),cut_values_( cut_values ),
        e_( e ), st_(st ), tf_( tf ), folder_( folder ), mc_(mc)
    {
        passed_ = std::vector<double>( cut_values.size(), 0 );
        total_ = 0;

        for (  double cut : cut_values_ )
        {
            if ( mc )
            {
                muon_recos_.push_back( make_unique<MCScatterSingleMuon>( e_, st ) );
            }
            else
            {
                muon_recos_.push_back( make_unique<DataMuRec>( e_, st, cut ));
            }
        }
    }

    void MuonMultiplierEfficiency::process_event()
    {
        double wgt = get_weight();

        total_ += wgt;

        for( unsigned int icut = 0 ; icut != cut_values_.size() ; ++icut )
        {
            double cut = cut_values_[icut];

            muon_recos_[icut]->new_event();
            if ( muon_recos_[icut]->found() )
            {
                passed_[icut] += wgt;
            }
        }
    }

    void MuonMultiplierEfficiency::end_processing()
    {
        double level = 0.6827;
        FeldmanCousinsBinomialInterval fc;
        double alpha = 1.-level;
        fc.Init(alpha);

        std::vector<double> effs;
        std::vector<double> eff_low;
        std::vector<double> eff_up;

        for( unsigned int icut = 0 ; icut != cut_values_.size() ; ++icut )
        {
            double pass = passed_[icut];
            double eff = pass / total_;

            double err = 1 / std::sqrt( total_ ) * sqrt( eff * ( 1 - eff ) );

            std::cout << cut_values_[icut] << " " << eff << " " << err << std::endl;

            effs.push_back( eff );
            eff_low.push_back( err );
            eff_up.push_back( err );
        }

        std::vector<double> xerr( effs.size(), 0 );

        TGraphAsymmErrors eff_graph( cut_values_.size() , cut_values_.data(), effs.data(), 
                xerr.data(), xerr.data(), eff_low.data(), eff_up.data());

        eff_graph.Print();
        std::cout << "eff_graph size: "  << eff_graph.GetN() << std::endl;

        cd_p( &tf_, folder_ );
        eff_graph.Write( "errs" );
    }


    template<>
        Subscriber * create_subscriber<MuonMultiplierEfficiency>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const SingleTrack * st = get_single_track( instruct, rf );

            const  fne::Event * e = rf.get_event_ptr();

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            std::vector<double> cut_values = get_yaml<std::vector<double>>
                ( instruct, "multiplier_values" );

            bool mc = rf.is_mc();

            return new MuonMultiplierEfficiency( *sel, cut_values, e, mc, *st, tfile, folder );
        }
}
