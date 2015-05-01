#include "PionPlotter.hh"
#include "k2pi_fitting.hh"
#include "K2piEventData.hh"
#include "K2piInterface.hh"
#include "root_help.hh"
#include "k2pi_reco_functions.hh"
namespace fn
{
    PionPlotter::PionPlotter(TFile & tf, std::string folder )
        :tf_( tf ), folder_( folder )
    {
        heop_ = hs_.MakeTH1D( "heop", "Track Cluster E/p",
                1500, 0.0 , 5.0,  "E/P", "#events" );

        hE_ = hs_.MakeTH1D( "hE", "Track Cluster E",
                100, 0.0 , 100,  "E", "#events" );

        hp_ = hs_.MakeTH1D( "hp", "Track Cluster p",
                100, 0.0 , 100,  "p", "#events" );

        heop_p_ = hs_.MakeTH2D( "heop_p", "Track Cluster E/p vs p",
                100, 0.0 , 100,  "p",
                120, 0.0 , 5.0,  "E/p" );

        hEp_ = hs_.MakeTH2D( "hEp", "Track Cluster E vs p",
                100, 0.0 , 100,  "p",
                100, 0.0 , 100,  "E" );
    }

    void PionPlotter::plot_pion( K2piEventData& event_data,
            K2piDchData& dch_data, double weight, bool mc )
    {
        double eop = 0;
        double E = 0;
        if (event_data.found_track_cluster)
        {
            eop = extract_eop( event_data, dch_data, mc );
            E = extract_eop_E( event_data, dch_data, mc );
        }

        heop_->Fill( eop, weight );
        heop_p_->Fill( dch_data.p, eop, weight );
        hE_->Fill( E, weight );
        hp_->Fill( dch_data.p, weight );
        heop_p_->Fill( dch_data.p, eop, weight );
        hEp_->Fill( dch_data.p, E, weight );
    }

    void PionPlotter::write()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    //--------------------------------------------------

    PionAnalysis::PionAnalysis( Selection& sel, 
            TFile& tf_, std::string folder_, 
            K2piEventData& k2pi_data, bool is_mc )
        :Analysis(sel), k2pi_data_( k2pi_data),
        pion_plots_( tf_, folder_ ), is_mc_( is_mc )
    {}

    void PionAnalysis::process_event()
    {
        pion_plots_.plot_pion( k2pi_data_, k2pi_data_.raw_dch, k2pi_data_.weight, is_mc_ );
    }

    void PionAnalysis::end_processing()
    {
        pion_plots_.write();
    }
}
