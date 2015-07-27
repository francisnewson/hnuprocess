#include "k2pi_selections.hh"
#include "NA62Constants.hh"
#include "FunctionCut.hh"
#include "PionPlotter.hh"
#include "PullPlotter.hh"
#include "BurstCount.hh"
#include "Summary.hh"
#include "tracking_selections.hh"

namespace fn
{
    K2piRecoBag::K2piRecoBag(K2piEventData& k2pi_event, bool mc, TFile& tfile, std::string channel, logger& slg)
        : is_mc( mc ),
        event( k2pi_event),
        raw_lkr( event.raw_lkr ),
        fit_lkr( event.fit_lkr ),
        fit_lkr_err( event.fit_lkr_err ),
        raw_dch( event.raw_dch ),
        tfout( tfile ),
        next_id_(0),
        channel_( channel ),
        slg_( slg )
    { }

    void K2piRecoBag::add_selection( Selection * sel)
    { 
        sel->set_id( next_id_++ );
        sel_.emplace_back( std::unique_ptr<Selection>( sel ) );
    }

    void K2piRecoBag::add_analysis( Analysis * an )
    {
        an->set_id( next_id_++ );
        an_.emplace_back( std::unique_ptr<Analysis>( an ) );
    }

    void K2piRecoBag::add_dch_selection( DchSelection * dch_sel )
    {
        dch_sel->set_id( next_id_++ );
        dch_slections_.emplace_back( std::unique_ptr<DchSelection>( dch_sel ) );
    }

    void K2piRecoBag::add_subscriber( Subscriber * sub )
    {
        sub->set_id( next_id_++ );
        sub->set_log( slg_ );
        sub_.emplace_back( std::unique_ptr<Subscriber>( sub ) );
    }

    void K2piRecoBag::new_event()
    {
        for( auto& sub: sub_ ) { sub->new_event(); }
        for( auto& sel: sel_ ) { sel->new_event(); }
        for( auto& an: an_ ) { an->new_event(); }
    }

    void K2piRecoBag::end_processing()
    {
        for( auto& an: an_){an->end_processing(); }
    }

    std::string K2piRecoBag::get_folder( std::string folder )
    {
        return (boost::filesystem::path( channel_ ) / folder ).string();
    }

    //--------------------------------------------------

    Selection * get_photon_sep_cut( K2piRecoBag& k2pirb, double sep )
    {
        auto photon_sep = [&k2pirb, sep]()
        {
            return extract_photon_sep( k2pirb.raw_lkr ) > sep ;
        };

        LambdaCut * photon_sep_cut = new LambdaCut{ photon_sep};
        photon_sep_cut->set_name( "photon_sep_cut" );
        return photon_sep_cut;
    }

    Selection * get_track_cluster_sep_cut( K2piRecoBag& k2pirb, double sep )
    {
        auto track_cluster_sep = [&k2pirb, sep]()
        {
            return extract_min_track_cluster_sep( k2pirb.raw_lkr, k2pirb.raw_dch ) > sep ;
        };

        LambdaCut * track_cluster_sep_cut  = new LambdaCut{ track_cluster_sep};
        track_cluster_sep_cut->set_name( "track_cluster_sep_cut" );
        return track_cluster_sep_cut;
    }

    Selection * get_track_track_cluster_sep_cut( K2piRecoBag& k2pirb, double sep, bool mc )
    {
        auto track_track_cluster_sep = [&k2pirb, sep, mc]()
        {
            if ( k2pirb.event.found_track_cluster )
            {
                return extract_track_track_cluster_sep( k2pirb.event, k2pirb.raw_dch, mc ) < sep ;
            }
            else
            {
                return true;
            }
        };

        LambdaCut * track_track_cluster_sep_cut  = new LambdaCut{ track_track_cluster_sep};
        track_track_cluster_sep_cut->set_name( "track_track_cluster_sep_cut" );
        return track_track_cluster_sep_cut;
    }

    Selection * get_min_photon_radius_cut( K2piRecoBag& k2pirb, double rad )
    {
        auto min_photon_radius = [&k2pirb, rad]
        {
            return extract_min_photon_radius( k2pirb.raw_lkr ) > rad;

        };

        LambdaCut * min_photon_radius_cut = new LambdaCut{ min_photon_radius };
        min_photon_radius_cut->set_name( "min_photon_radius_cut" );
        return min_photon_radius_cut;
    }

    Selection * get_chi2_cut( K2piRecoBag& k2pirb, double chi2 )
    {
        auto max_chi2 = [&k2pirb, chi2]
        {
            return k2pirb.event.lkr_fit_chi2 < chi2 ;
        };

        LambdaCut * chi2_cut = new LambdaCut{ max_chi2 };
        chi2_cut->set_name( "chi2_cut" );
        return chi2_cut;
    }

    Selection * get_min_chi2_cut( K2piRecoBag& k2pirb, double chi2 )
    {
        auto min_chi2 = [&k2pirb, chi2]
        {
            return k2pirb.event.lkr_fit_chi2 > chi2 ;
        };

        LambdaCut * chi2_cut = new LambdaCut{ min_chi2 };
        chi2_cut->set_name( "min_chi2_cut" );
        return chi2_cut;
    }

    Selection * get_bad_prob_cut( K2piRecoBag& k2pirb, double prob )
    {
        auto bad_prob = [&k2pirb, prob]
        {
            return TMath::Prob( k2pirb.event.lkr_fit_chi2, 1 ) < 0.01;
        };
        LambdaCut * bad_prob_cut  = new LambdaCut{ bad_prob};
        bad_prob_cut->set_name( "bad_prob_cut");
        return bad_prob_cut;
    }

    Selection * get_muv_cut( K2piRecoBag& k2pirb, bool mc )
    {
        if (mc)
        {
            auto accept_event = [] { return true; };
            auto weight = [&k2pirb]
            {
                if ( k2pirb.event.muv.found_muon )
                {
                    return 1 - k2pirb.event.muv.eff;
                }
                else
                {
                    return 1.0;
                }
            };

            LambdaWgtCut * muv_cut = new LambdaWgtCut{ accept_event, weight };
            muv_cut->set_name( "mc_muv_cut" );
            return muv_cut;
        }
        else
        {
            auto accept_event = [&k2pirb]
            {
                return !k2pirb.event.muv.found_muon;
            };

            LambdaCut * muv_cut = new LambdaCut{ accept_event };
            muv_cut->set_name( "dt_muv_cut" );
            return muv_cut;
        }
    }

    K2piSingleTrack * get_k2pi_single_track( K2piRecoBag& k2pirb )
    {
        K2piSingleTrack * result = new K2piSingleTrack( k2pirb.raw_dch, k2pirb.fit_lkr );
        result->set_name( "k2pi_single_track" );
        return result;
    }

    Selection * get_eop_cut( K2piRecoBag& k2pirb, bool mc, double E_thresh, double min, double max )
    {
        auto raw_eop_cut = [&k2pirb, mc, E_thresh, min, max]()
        {
            if ( k2pirb.event.found_track_cluster && k2pirb.event.TCE > E_thresh )
            {
                double eop = extract_eop( k2pirb.event, k2pirb.raw_dch, k2pirb.is_mc );
                return ( min < eop && eop < max );
            }
            else
            {
                return true;
            }
        };

        LambdaCut * eop_cut = new LambdaCut{ raw_eop_cut };
        eop_cut->set_name( "eop_cut" );
        return eop_cut;
    }

    void add_dch_study( K2piRecoBag & k2pirb)
    {
        //**************************************************
        //Selections
        //**************************************************

        Selection * passer = new FunctionCut<auto_pass>( 0 );
        k2pirb.add_selection( passer );

        //M2M CUT
        auto m2m_pip = [&k2pirb]()
        {
            //Copy data into parameter array
            k2pi_params lkr_param_data;
            K2piLkrInterface lkr_interface( k2pirb.raw_lkr);
            copy( lkr_interface, lkr_param_data );

            //Extract 'fit' result
            k2pi_fit data_fit;
            compute_pion_mass( lkr_param_data, data_fit );

            //Lkr 4mom
            TLorentzVector& lkr_pip_4mom = data_fit.pip;
            double m2m =  lkr_pip_4mom.M2();
            double delta_m2m = m2m - std::pow(na62const::mPi, 2 );

            return ( fabs( delta_m2m ) <  0.005 ) ;
        };

        LambdaCut * m2m_cut = new LambdaCut{ m2m_pip};
        k2pirb.add_selection( m2m_cut );

        //--------------------

        //EOP CUT
        auto eop_cut_90 = get_eop_cut  (k2pirb, k2pirb.is_mc, 9, 0.0, 0.90 );
        eop_cut_90->set_name( "eop_cut_90" );
        k2pirb.add_selection( eop_cut_90 );

        auto eop_cut_80 = get_eop_cut  (k2pirb, k2pirb.is_mc, 9, 0.0, 0.80 );
        eop_cut_80->set_name( "eop_cut_80" );
        k2pirb.add_selection( eop_cut_80 );

        //--------------------

        //PHOTON SEPARATION
        auto photon_sep_cut =  get_photon_sep_cut(k2pirb, 20  ) ;
        k2pirb.add_selection( photon_sep_cut);

        //--------------------

        //TRACK CLUSTER SEPARATION
        auto track_cluster_sep_cut =  get_track_cluster_sep_cut(k2pirb, 40 ) ;
        k2pirb.add_selection  (track_cluster_sep_cut);

        //--------------------

        //TRACK TRACK CLUSTER SEPARATION
        auto track_track_cluster_sep_cut =  get_track_track_cluster_sep_cut(k2pirb, 30, k2pirb.is_mc ) ;
        k2pirb.add_selection  (track_track_cluster_sep_cut);

        //--------------------

        //MIN PHOTON RADIUS
        auto min_photon_radius_cut = get_min_photon_radius_cut( k2pirb, 15 );
        k2pirb.add_selection( min_photon_radius_cut );

        //--------------------

        //CHI2 CUT
        auto chi2_cut_010 = get_chi2_cut( k2pirb, 0.1 );
        k2pirb.add_selection( chi2_cut_010 );

        auto chi2_cut_050 = get_chi2_cut( k2pirb, 0.5 );
        chi2_cut_050->set_name( "chi2_cut_050" );
        k2pirb.add_selection( chi2_cut_050 );

        auto bad_prob_cut = get_bad_prob_cut( k2pirb, 0.01 );
        k2pirb.add_selection( bad_prob_cut );

        //--------------------

        //MUV CUT
        auto muv_cut = get_muv_cut( k2pirb, k2pirb.is_mc);
        k2pirb.add_selection( muv_cut );

        //--------------------

        //TRACKING
        SingleTrack * st = get_k2pi_single_track( k2pirb );
        k2pirb.add_subscriber( st );

        //MUV ACC
        Selection * muv_acc = new TrackXYUVAcceptance( *st, 
                TrackXYUVAcceptance::track_section::ds , na62const::zMuv1, 
                -100, 100, /* x */ -100, 100, /* y */
                -100, 100, /* u */ -100, 100 /* v */ );
        muv_acc->set_name( "muv_acc" );
        k2pirb.add_selection( muv_acc );

        Selection * muv_inner_acc = new TrackRadialAcceptance( *st,
                TrackRadialAcceptance::track_section::ds,
                na62const::zMuv1, 20.0, 200.0 );
        muv_inner_acc->set_name( "muv_inner_acc" );
        k2pirb.add_selection( muv_inner_acc );

        //--------------------

        //NO MUV
        CompositeSelection * no_muv = new CompositeSelection(  {passer} );
        k2pirb.add_selection( no_muv );
        CompositeSelection * no_muv_acc = new CompositeSelection(  {passer, muv_acc, muv_inner_acc} );
        k2pirb.add_selection( no_muv_acc );
       
        //NO FIT 
        CompositeSelection * no_fit_raw = new CompositeSelection( 
                { track_track_cluster_sep_cut,  track_cluster_sep_cut, min_photon_radius_cut, muv_acc, muv_inner_acc, muv_cut } );
        no_fit_raw->set_name( "no_fit_raw" );
        k2pirb.add_selection( no_fit_raw );

        CompositeSelection * no_fit_full = new CompositeSelection( { no_fit_raw, m2m_cut, eop_cut_90} );
        no_fit_full->set_name( "no_fit_full" );
        k2pirb.add_selection( no_fit_full );

        //FIT 
        CompositeSelection * fit_selection_eop_90 = new CompositeSelection( { no_fit_raw, eop_cut_90, chi2_cut_050 } );
        fit_selection_eop_90->set_name( "fit_selection_eop_90" );
        k2pirb.add_selection( fit_selection_eop_90 );

        CompositeSelection * fit_selection_eop_80 = new CompositeSelection( { no_fit_raw, eop_cut_80, chi2_cut_050 } );
        fit_selection_eop_80->set_name( "fit_selection_eop_80" );
        k2pirb.add_selection( fit_selection_eop_80 );

        //DODGY
        CompositeSelection * dodgy_selection = new CompositeSelection(
                { track_track_cluster_sep_cut,  track_cluster_sep_cut, 
                min_photon_radius_cut, muv_acc, muv_inner_acc,  muv_cut, eop_cut_90, bad_prob_cut } );
        dodgy_selection->set_name("dodgy_selection");
        k2pirb.add_selection( dodgy_selection);

        //**************************************************
        //DCH Cuts
        //**************************************************

        DchSelection * cda_cut_35 =  new DchSelection( []( K2piDchData* dch_data, K2piLkrData* lkr_data)
                {
                Vertex vertex = extract_vertex( *dch_data,  *lkr_data );
                double cda = vertex.cda;
                return cda < 3.5;
                } );

        k2pirb.add_dch_selection( cda_cut_35 );

        DchSelection * z_cut_m1500_6500 =  new DchSelection( []( K2piDchData* dch_data, K2piLkrData* lkr_data)
                {
                Vertex vertex = extract_vertex( *dch_data,  *lkr_data );
                double z = vertex.point.Z();
                return z > -1500 && z < 6500;
                } );

        k2pirb.add_dch_selection( z_cut_m1500_6500 );

        DchSelection *track_mom_cut_10_60 = new DchSelection(  []( K2piDchData* dch_data, K2piLkrData* lkr_data)
                {
                K2piDchInterface dch( *dch_data );
                return ( dch.p() > 10 ) && (dch.p() < 60 );
                } );

        //**************************************************
        //Analyses
        //**************************************************

        auto select_fit_folder =  k2pirb.get_folder("select_fit_k2pi_plots");
        std::cout << "Select fit folder: " << select_fit_folder << std::endl;

        bool do_scatter = true;
        bool dont_do_scatter = true;

#if 0
        DchAnalysis * no_fit_raw_plots  = new DchAnalysis(
                *no_fit_raw, k2pirb.tfout, k2pirb.get_folder( "no_fit_raw_plots" ),
                k2pirb.event, "raw", k2pirb.is_mc, dont_do_scatter );

        DchAnalysis * no_fit_fit_plots  = new DchAnalysis(
                *no_fit_raw, k2pirb.tfout, k2pirb.get_folder( "no_fit_fit_plots" ),
                k2pirb.event, "fit", k2pirb.is_mc, dont_do_scatter );

        DchAnalysis * no_fit_full_plots  = new DchAnalysis(
                *no_fit_full, k2pirb.tfout, k2pirb.get_folder( "no_fit_full_plots"),
                k2pirb.event, "raw", k2pirb.is_mc, dont_do_scatter );
#endif

        DchAnalysis * fit_eop_90_plots  = new DchAnalysis(
                *fit_selection_eop_90, k2pirb.tfout, k2pirb.get_folder( "fit_eop_90") ,
                k2pirb.event, "fit", k2pirb.is_mc, do_scatter );

        DchAnalysis * fit_eop_90_plots_no_cda  = new DchAnalysis(
                *fit_selection_eop_90, k2pirb.tfout, k2pirb.get_folder( "fit_eop_90_no_cda") ,
                k2pirb.event, "fit", k2pirb.is_mc, do_scatter );

#if 0
        DchAnalysis * fit_eop_80_plots  = new DchAnalysis(
                *fit_selection_eop_80, k2pirb.tfout, k2pirb.get_folder( "fit_eop_80") ,
                k2pirb.event, "fit", k2pirb.is_mc, dont_do_scatter );

        DchAnalysis * fit_eop_90_raw_plots  = new DchAnalysis(
                *fit_selection_eop_90, k2pirb.tfout, k2pirb.get_folder( "fit_eop_90_raw") ,
                k2pirb.event, "raw", k2pirb.is_mc, do_scatter );

        DchAnalysis * fit_dodgy_plots  = new DchAnalysis(
                *dodgy_selection, k2pirb.tfout, k2pirb.get_folder( "dodgy_fit") ,
                k2pirb.event, "fit", k2pirb.is_mc, do_scatter );


        DchAnalysis * raw_dodgy_plots  = new DchAnalysis(
                *dodgy_selection, k2pirb.tfout, k2pirb.get_folder( "dodgy_raw"),
                k2pirb.event, "raw", k2pirb.is_mc, do_scatter );
#endif

        auto * burst_count = new K2piBurstCount( *passer, k2pirb.tfout,
                k2pirb.get_folder("burst_count"), k2pirb.event );

        K2piMuvEff * muv_eff = new K2piMuvEff( *no_muv, *muv_cut,
                k2pirb.event, *st, k2pirb.tfout, k2pirb.get_folder( "muv_eff") );

        K2piMuvEff * muv_acc_eff = new K2piMuvEff( *no_muv_acc, *muv_cut,
                k2pirb.event, *st, k2pirb.tfout, k2pirb.get_folder( "muv_acc_eff") );


        //k2pirb.add_analysis( no_fit_raw_plots );
        //k2pirb.add_analysis( no_fit_fit_plots );
        //k2pirb.add_analysis( no_fit_full_plots );
        //k2pirb.add_analysis( fit_eop_80_plots );
        k2pirb.add_analysis( fit_eop_90_plots );
        //k2pirb.add_analysis( fit_eop_90_raw_plots );

        //add dch selections
        for ( auto an = k2pirb.an_begin() ; an != k2pirb.an_end() ; ++an )
        {
            auto dch_an = static_cast<DchAnalysis*>( an->get() );
            dch_an->add_dch_selection( cda_cut_35 );
            dch_an->add_dch_selection( z_cut_m1500_6500 );
            dch_an->add_dch_selection( track_mom_cut_10_60 );
        }

        k2pirb.add_analysis( fit_eop_90_plots_no_cda );
        fit_eop_90_plots_no_cda->add_dch_selection( z_cut_m1500_6500 );
        fit_eop_90_plots_no_cda->add_dch_selection( track_mom_cut_10_60 );

        //k2pirb.add_analysis( fit_dodgy_plots );
        //k2pirb.add_analysis( raw_dodgy_plots );

        k2pirb.add_analysis( burst_count );
        k2pirb.add_analysis( muv_eff );
        k2pirb.add_analysis( muv_acc_eff );

        Summary  * full_no_fit_summary = new Summary( *passer, *no_fit_full, std::cout );
        full_no_fit_summary->set_name("full_no_fit_summary");
        k2pirb.add_analysis( full_no_fit_summary );

        Summary  * full_fit_eop_90_summary = new Summary( *passer, *fit_selection_eop_90, std::cout );
        full_fit_eop_90_summary->set_name( "full_fit_eop_90_summary" );
        k2pirb.add_analysis( full_fit_eop_90_summary );
    }

    void add_pion_study( K2piRecoBag & k2pirb)
    {
        //**************************************************
        //Selections
        //**************************************************

        //PHOTON SEPARATION
        auto photon_sep_cut =  get_photon_sep_cut(k2pirb, 25  ) ;
        k2pirb.add_selection( photon_sep_cut);

        //--------------------

        //TRACK CLUSTER SEPARATION
        auto track_cluster_sep_cut =  get_track_cluster_sep_cut(k2pirb, 30 ) ;
        k2pirb.add_selection  (track_cluster_sep_cut);

        //--------------------


        //MIN PHOTON RADIUS
        auto min_photon_radius_cut = get_min_photon_radius_cut( k2pirb, 15 );
        k2pirb.add_selection( min_photon_radius_cut );

        //CHI2 CUT
        auto chi2_cut = get_chi2_cut( k2pirb, 0.016 );
        k2pirb.add_selection( chi2_cut );

        CompositeSelection * full_selection = new CompositeSelection( 
                {chi2_cut, track_cluster_sep_cut, min_photon_radius_cut } );

        full_selection->set_name( "full_selection" );
        k2pirb.add_selection( full_selection );

        //**************************************************
        //Analysis
        //**************************************************

        PionAnalysis * pion_plotter =  new PionAnalysis(
                *full_selection, k2pirb.tfout, "pion_plotter", k2pirb.event, k2pirb.is_mc );

        k2pirb.add_analysis( pion_plotter );
    }

    void add_pull_study( K2piRecoBag & k2pirb )
    {

        //**************************************************
        //Selections
        //**************************************************

        //PHOTON SEPARATION
        auto photon_sep_cut =  get_photon_sep_cut(k2pirb, 20  ) ;
        k2pirb.add_selection( photon_sep_cut);

        //--------------------

        //TRACK CLUSTER SEPARATION
        auto track_cluster_sep_cut =  get_track_cluster_sep_cut(k2pirb, 40 ) ;
        k2pirb.add_selection  (track_cluster_sep_cut);

        //--------------------

        //TRACK TRACK CLUSTER SEPARATION
        auto track_track_cluster_sep_cut =  get_track_track_cluster_sep_cut(k2pirb, 30, k2pirb.is_mc ) ;
        k2pirb.add_selection  (track_track_cluster_sep_cut);

        //--------------------

        //MIN PHOTON RADIUS
        auto min_photon_radius_cut = get_min_photon_radius_cut( k2pirb, 15 );
        k2pirb.add_selection( min_photon_radius_cut );


        //Chi2 selection
        auto chi2_cut = get_chi2_cut( k2pirb, 10000000000000 );
        k2pirb.add_selection( chi2_cut );

        auto good_chi2_cut = get_chi2_cut( k2pirb, 0.5 );
        k2pirb.add_selection( good_chi2_cut );


        //BAD PROB CUT
        auto bad_prob_cut = get_bad_prob_cut( k2pirb, 0.01 );
        k2pirb.add_selection( bad_prob_cut );

        CompositeSelection * full_selection = new CompositeSelection(
                {photon_sep_cut, track_cluster_sep_cut, track_track_cluster_sep_cut, min_photon_radius_cut,chi2_cut} );

        full_selection->set_name( "full_selection" );
        k2pirb.add_selection( full_selection );

        CompositeSelection * good_selection = new CompositeSelection(
                {photon_sep_cut, track_cluster_sep_cut, track_track_cluster_sep_cut, min_photon_radius_cut,good_chi2_cut} );

        good_selection->set_name( "good_selection" );
        k2pirb.add_selection( good_selection );

        CompositeSelection * dodgy_selection = new CompositeSelection(
                {photon_sep_cut, track_cluster_sep_cut, track_track_cluster_sep_cut,
                min_photon_radius_cut,chi2_cut, bad_prob_cut} );

        dodgy_selection->set_name( "dodgy_selection" );
        k2pirb.add_selection( dodgy_selection );


        //**************************************************
        //Analysis
        //**************************************************

        PullPlotterAnalysis * pull_plotter =  new PullPlotterAnalysis(
                *full_selection, k2pirb.tfout,  "pull_plotter",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::mc );

        k2pirb.add_analysis( pull_plotter );


        PullPlotterAnalysis * pull_plotter_dt =  new PullPlotterAnalysis(
                *full_selection, k2pirb.tfout,  "pull_plotter_dt",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::data );
        k2pirb.add_analysis( pull_plotter_dt );


        PullPlotterAnalysis * good_pull_plotter =  new PullPlotterAnalysis(
                *good_selection, k2pirb.tfout,  "good_pull_plotter",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::mc );

        k2pirb.add_analysis( good_pull_plotter );

        PullPlotterAnalysis * good_pull_plotter_mcafter =  new PullPlotterAnalysis(
                *good_selection, k2pirb.tfout,  "good_pull_plotter_mcafter",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::mcafter );

        k2pirb.add_analysis( good_pull_plotter_mcafter );


        PullPlotterAnalysis * good_pull_plotter_dt =  new PullPlotterAnalysis(
                *good_selection, k2pirb.tfout,  "good_pull_plotter_dt",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::data );

        k2pirb.add_analysis( good_pull_plotter_dt );


        PullPlotterAnalysis * dodgy_pull_plotter =  new PullPlotterAnalysis(
                *dodgy_selection, k2pirb.tfout,  "dodgy_pull_plotter",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::mc );

        k2pirb.add_analysis( dodgy_pull_plotter );


        PullPlotterAnalysis * dodgy_pull_plotter_dt =  new PullPlotterAnalysis(
                *dodgy_selection, k2pirb.tfout,  "dodgy_pull_plotter_dt",
                k2pirb.raw_lkr, k2pirb.fit_lkr, k2pirb.fit_lkr_err, k2pirb.event, PullPlotter::source::data );

        k2pirb.add_analysis( dodgy_pull_plotter_dt );
    }
}

