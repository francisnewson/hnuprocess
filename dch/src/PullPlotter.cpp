#include "PullPlotter.hh"
#include "NA62Constants.hh"
#include "Track.hh"
namespace fn
{
    PullPlotter::PullPlotter( TFile& tf, std::string folder, source data_source )
        :tf_( tf ), folder_( folder ), data_source_( data_source)
    {

        double width = 5;
        double bins = 100;
        if ( data_source_ == source::data )
        {
            width = 30;
            bins = 3000;
        }

        hE1_ = hs_.MakeTH1D( "hE1", "E1 Pulls", bins, -width, width , "Pull" );
        hE2_ = hs_.MakeTH1D( "hE2", "E2 Pulls", bins, -width, width , "Pull" );

        hX1_ = hs_.MakeTH1D( "hX1", "X1 Pulls", bins, -width, width , "Pull" );
        hX2_ = hs_.MakeTH1D( "hX2", "X2 Pulls", bins, -width, width , "Pull" );

        hY1_ = hs_.MakeTH1D( "hY1", "Y1 Pulls", bins, -width, width , "Pull" );
        hY2_ = hs_.MakeTH1D( "hY2", "Y2 Pulls", bins, -width, width , "Pull" );

        hpkx_ = hs_.MakeTH1D( "hpkx", "pkx Pulls", bins, -width, width , "Pull" );
        hpky_ = hs_.MakeTH1D( "hpky", "pky Pulls", bins, -width, width , "Pull" );
        hpkz_ = hs_.MakeTH1D( "hpkz", "pkz Pulls", bins, -width, width , "Pull" );

        hkx0_ = hs_.MakeTH1D( "hkx0", "kx0 Pulls", bins, -width, width , "Pull" );
        hky0_= hs_.MakeTH1D( "hky0", "ky0 Pulls", bins, -width, width , "Pull" );

        hx0_ = hs_.MakeTH1D( "hx0", "True x0", 100, -5, 5 , "x0" );
        hy0_ = hs_.MakeTH1D( "hy0", "True y0", 100, -5, 5 , "y0" );

        h_prob_ = hs_.MakeTH1D( "hprob", "Upper tail probability", 
                1000, 0.0, 1.0 , "Probability" );

        h_chi2_ = hs_.MakeTH1D( "hchi2_", "Chi2", 
                10000, 0, 1000 , "chi2" );
    }

    void PullPlotter::plot_pulls( const K2piLkrData& before, const K2piLkrData& after,
            const K2piLkrData& errs, const K2piEventData& event, 
            double wgt, double chi2 )
    {

        if ( data_source_ == source::mc || data_source_ == source::mcafter )
        {
            double true_E1 = event.mc.p4g1.E();
            double true_E2 = event.mc.p4g2.E();

            Track track1{ event.mc.vertex, event.mc.p4g1.Vect() };
            TVector3 cluster1  = track1.extrapolate( na62const::zLkr );
            double true_X1 = cluster1.X();
            double true_Y1 = cluster1.Y();

            Track track2{ event.mc.vertex, event.mc.p4g2.Vect() };
            TVector3 cluster2  = track2.extrapolate( na62const::zLkr );
            double true_X2 = cluster2.X();
            double true_Y2 = cluster2.Y();

            TVector3 pk = event.mc.p4K.Vect();
            double true_pkx = pk.X();
            double true_pky = pk.Y();
            double true_pkz = pk.Z();

            Track track_k{ event.mc.vertex, pk };
            TVector3 k0 = track_k.extrapolate( 0 );
            double true_kx0 = k0.X();
            double true_ky0 = k0.Y();

            const auto mydata = (data_source_ == source::mc) ? before : after;


            hE1_->Fill( ( mydata.E1 - true_E1) / errs.E1, wgt );
            hE2_->Fill( ( mydata.E2 - true_E2) / errs.E2, wgt );

            hX1_->Fill( ( mydata.X1 - true_X1) / errs.X1, wgt );
            hX2_->Fill( ( mydata.X2 - true_X2) / errs.X2, wgt );

            hY1_->Fill( ( mydata.Y1 - true_Y1) / errs.Y1, wgt );
            hY2_->Fill( ( mydata.Y2 - true_Y2) / errs.Y2, wgt );

            hpkx_->Fill( ( mydata.pkx - true_pkx) / errs.pkx, wgt );
            hpky_->Fill( ( mydata.pky - true_pky) / errs.pky, wgt );
            hpkz_->Fill( ( mydata.pkz - true_pkz) / errs.pkz, wgt );

            hkx0_->Fill( ( mydata.kx0 - true_kx0) / errs.kx0, wgt );
            hky0_->Fill( ( mydata.ky0 - true_ky0) / errs.ky0, wgt );

            hx0_->Fill( true_kx0, wgt );
            hy0_->Fill( true_ky0, wgt );

            h_prob_->Fill( TMath::Prob( chi2, 1), wgt );
            h_chi2_->Fill(  chi2, wgt );
        }
        else if ( data_source_ == source::data )
        {
            const auto& m = errs;
            const auto& e = event.fit_lkr_fit_err;

            //std::cout << after.X1  << " " << before.X1 << " "  << after.X1 - before.X1 << " "
                //<< m.X1 << " " << e.X1 << std::endl;

            hE1_->Fill( ( after.E1 - before.E1)    / std::sqrt( m.E1*m.E1 - e.E1*e.E1) , wgt );
            hE2_->Fill( ( after.E2 - before.E2)    / std::sqrt( m.E2*m.E2 - e.E2*e.E2), wgt );

            hX1_->Fill( ( after.X1 - before.X1)    / std::sqrt( m.X1*m.X1 - e.X1*e.X1), wgt );
            hX2_->Fill( ( after.X2 - before.X2)    / std::sqrt( m.X2*m.X2 - e.X2*e.X2), wgt );

            hY1_->Fill( ( after.Y1 - before.Y1)    / std::sqrt( m.Y1*m.Y1 - e.Y1*e.Y1), wgt );
            hY2_->Fill( ( after.Y2 - before.Y2)    / std::sqrt( m.Y2*m.Y2 - e.Y2*e.Y2), wgt );

            hpkx_->Fill( ( after.pkx - before.pkx) / std::sqrt( m.pkx*m.pkx - e.pkx*e.pkx), wgt );
            hpky_->Fill( ( after.pky - before.pky) / std::sqrt( m.pky*m.pky - e.pky*e.pky), wgt );
            hpkz_->Fill( ( after.pkz - before.pkz) / std::sqrt( m.pkz*m.pkz - e.pkz*e.pkz), wgt );

            hkx0_->Fill( ( after.kx0 - before.kx0) / std::sqrt( m.kx0*m.kx0 - e.kx0*e.kx0), wgt );
            hky0_->Fill( ( after.ky0 - before.ky0) / std::sqrt( m.ky0*m.ky0 - e.ky0*e.ky0), wgt );
        }
        else
        {
            throw std::runtime_error( "PullPlotter source problem");
        }

    }

    void PullPlotter::write()
    { 
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    PullPlotterAnalysis::PullPlotterAnalysis( 
            Selection& sel,
            TFile& tf, std::string folder,
            const K2piLkrData& before,
            const K2piLkrData& after,
            const K2piLkrData& errs ,
            const K2piEventData& event,
            PullPlotter::source data_source
            )
        :Analysis( sel ), pull_plotter_( tf, folder, data_source ),
        before_( before), after_(after), errs_(errs ), event_( event )
    {
    }

    void PullPlotterAnalysis::process_event()
    {
        pull_plotter_.plot_pulls( before_, after_, errs_,
                event_, event_.weight, event_.lkr_fit_chi2 );
    }

    void PullPlotterAnalysis::end_processing()
    {
        pull_plotter_.write();
    }
}
