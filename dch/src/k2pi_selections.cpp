#include "k2pi_selections.hh"
#include "NA62Constants.hh"
#include "FunctionCut.hh"
#include "PionPlotter.hh"
#include "BurstCount.hh"
#include "Summary.hh"

namespace fn
{
    K2piRecoBag::K2piRecoBag(K2piEventData& k2pi_event, bool mc, TFile& tfile, std::string channel)
        : is_mc( mc ),
        event( k2pi_event),
        raw_lkr( event.raw_lkr ),
        fit_lkr( event.fit_lkr ),
        raw_dch( event.raw_dch ),
        tfout( tfile ),
        next_id_(0),
        channel_( channel )
    {}

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

    void K2piRecoBag::new_event()
    {
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
        auto raw_eop = [&k2pirb]()
        {
            if ( k2pirb.event.found_track_cluster )
            {
                double eop = extract_eop( k2pirb.event, k2pirb.raw_dch, k2pirb.is_mc );
                return ( 0.0 < eop && eop < 0.95 );
            }
            else
            {
                return true;
            }
        };

        LambdaCut * eop_cut = new LambdaCut{ raw_eop};
        eop_cut->set_name( "eop_cut" );
        k2pirb.add_selection( eop_cut );

        //EXTRA EOP CUT
        auto raw_extra_eop = [&k2pirb]()
        {
            if ( k2pirb.event.found_track_cluster )
            {
                double eop = extract_eop( k2pirb.event, k2pirb.raw_dch, k2pirb.is_mc );
                return ( 0.2 < eop && eop < 0.95 );
            }
            else
            {
                return false;
            }
        };

        LambdaCut * extra_eop_cut = new LambdaCut{ raw_extra_eop};
        extra_eop_cut->set_name( "extra_eop_cut" );
        k2pirb.add_selection( extra_eop_cut );

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
        auto chi2_cut = get_chi2_cut( k2pirb, 0.1 );
        k2pirb.add_selection( chi2_cut );

        //--------------------

        //MUV CUT
        auto muv_cut = get_muv_cut( k2pirb, k2pirb.is_mc);
        k2pirb.add_selection( muv_cut );

        CompositeSelection * full_selection = new CompositeSelection( 
                { m2m_cut, eop_cut, track_cluster_sep_cut, min_photon_radius_cut } );
        full_selection->set_name( "full_selection" );
        k2pirb.add_selection( full_selection );

        CompositeSelection * fit_selection = new CompositeSelection( 
                {  chi2_cut, eop_cut, track_cluster_sep_cut, track_track_cluster_sep_cut,  min_photon_radius_cut, muv_cut} );
        fit_selection->set_name( "fit_selection" );
        k2pirb.add_selection( fit_selection );

        CompositeSelection * fit_extra_eop_selection = new CompositeSelection( 
                {  chi2_cut, eop_cut, extra_eop_cut, track_cluster_sep_cut, track_track_cluster_sep_cut,  min_photon_radius_cut} );
        fit_extra_eop_selection->set_name( "fit_extra_eop_selection" );
        k2pirb.add_selection( fit_extra_eop_selection );

        CompositeSelection * fit_no_eop_selection = new CompositeSelection( 
                {  chi2_cut, track_cluster_sep_cut, min_photon_radius_cut} );
        fit_no_eop_selection->set_name( "fit_no_eop_selection" );
        k2pirb.add_selection( fit_no_eop_selection );

        CompositeSelection * fit_no_chi2_selection = new CompositeSelection( 
                { track_cluster_sep_cut, min_photon_radius_cut} );
        fit_no_chi2_selection->set_name( "fit_no_chi2_selection" );
        k2pirb.add_selection( fit_no_chi2_selection );

        //**************************************************
        //Analyses
        //**************************************************

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //IMPORTANT
        //DchAnalyses implement dch_cuts
        //(in addition to anything in the selections above)
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        DchSelection * cda_cut_35 =  new DchSelection( []( K2piDchData* dch_data, K2piLkrData* lkr_data)
                {
                Vertex vertex = extract_vertex( *dch_data,  *lkr_data );
                double cda = vertex.cda;
                return cda < 3.5;
                } );


        k2pirb.add_dch_selection( cda_cut_35 );

        DchSelection * z_cut_m1000_6500 =  new DchSelection( []( K2piDchData* dch_data, K2piLkrData* lkr_data)
                {
                Vertex vertex = extract_vertex( *dch_data,  *lkr_data );
                double z = vertex.point.Z();
                return z > -2000 && z < 6500;
                } );

        k2pirb.add_dch_selection( z_cut_m1000_6500 );

        auto select_fit_folder =  k2pirb.get_folder("select_fit_k2pi_plots");
        std::cout << "Select fit folder: " << select_fit_folder << std::endl;

        auto select_fit_extra_eop_folder =  k2pirb.get_folder("select_fit_extra_eop_k2pi_plots");
        std::cout << "Select fit folder: " << select_fit_extra_eop_folder << std::endl;

        DchAnalysis * select_fit_dch_plotter  = new DchAnalysis(
                *fit_selection, k2pirb.tfout, select_fit_folder,k2pirb.event, "fit", k2pirb.is_mc );

        DchAnalysis * select_fit_extra_eop_dch_plotter  = new DchAnalysis(
                *fit_extra_eop_selection, k2pirb.tfout, select_fit_extra_eop_folder,k2pirb.event, "fit", k2pirb.is_mc );

        DchAnalysis * select_fit_no_chi2_dch_plotter  = new DchAnalysis(
                *fit_no_chi2_selection, k2pirb.tfout, k2pirb.get_folder("select_fit_no_chi2_k2pi_plots"),
                k2pirb.event, "fit", k2pirb.is_mc );

        auto * burst_count = new K2piBurstCount( *passer, k2pirb.tfout,
                k2pirb.get_folder("burst_count"), k2pirb.event );

        select_fit_dch_plotter->add_dch_selection( cda_cut_35 );
        select_fit_no_chi2_dch_plotter->add_dch_selection( cda_cut_35 );

        select_fit_dch_plotter->add_dch_selection( z_cut_m1000_6500 );
        select_fit_no_chi2_dch_plotter->add_dch_selection( z_cut_m1000_6500 );

        k2pirb.add_analysis( select_fit_dch_plotter );
        k2pirb.add_analysis( select_fit_extra_eop_dch_plotter );
        k2pirb.add_analysis( select_fit_no_chi2_dch_plotter );
        k2pirb.add_analysis( burst_count );

        Summary  * full_summary = new Summary( *passer, *fit_selection, std::cout );
        k2pirb.add_analysis( full_summary );

        Summary  * full_extra_summary = new Summary( *passer, *fit_extra_eop_selection, std::cout );
        k2pirb.add_analysis( full_extra_summary );
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
}
