#include "k2pi_selections.hh"
#include "NA62Constants.hh"
#include "FunctionCut.hh"
#include "PionPlotter.hh"

namespace fn
{
    K2piRecoBag::K2piRecoBag(K2piEventData& k2pi_event, bool mc, TFile& tfile)
        : is_mc( mc ),
        event( k2pi_event),
        raw_lkr( event.raw_lkr ),
        fit_lkr( event.fit_lkr ),
        raw_dch( event.raw_dch ),
        tfout( tfile ),
        next_id_(0){}

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

    void add_dch_study( K2piRecoBag & k2pirb)
    {

        //**************************************************
        //Selections
        //**************************************************

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
                return ( 0.2 < eop && eop < 0.8 );
            }
            else
            {
                return true;
            }
        };

        LambdaCut * eop_cut = new LambdaCut{ raw_eop};
        eop_cut->set_name( "eop_cut" );
        k2pirb.add_selection( eop_cut );

        //--------------------

        //PHOTON SEPARATION
        auto photon_sep_cut =  get_photon_sep_cut(k2pirb, 20  ) ;
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
                { m2m_cut, eop_cut, track_cluster_sep_cut, min_photon_radius_cut } );
        full_selection->set_name( "full_selection" );
        k2pirb.add_selection( full_selection );

        CompositeSelection * fit_selection = new CompositeSelection( 
                {  chi2_cut, eop_cut, track_cluster_sep_cut, min_photon_radius_cut} );
        fit_selection->set_name( "fit_selection" );
        k2pirb.add_selection( fit_selection );

        CompositeSelection * fit_no_eop_selection = new CompositeSelection( 
                {  chi2_cut, track_cluster_sep_cut, min_photon_radius_cut} );
        fit_no_eop_selection->set_name( "fit_no_eop_selection" );
        k2pirb.add_selection( fit_no_eop_selection );

        //**************************************************
        //Analysis
        //**************************************************

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //IMPORTANT
        //DchAnalysis implements a cut on CDA!
        //(in addition to anything in the selections above)
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        DchSelection * cda_cut_35 =  new DchSelection( []( K2piDchData* dch_data, K2piLkrData* lkr_data)
                {
                Vertex vertex = extract_vertex( *dch_data,  *lkr_data );
                double cda = vertex.cda;
                return cda < 3.5;
                } );

        k2pirb.add_dch_selection( cda_cut_35 );

        DchAnalysis * select_fit_dch_plotter  = new DchAnalysis(
                *fit_selection, k2pirb.tfout, "select_fit_k2pi_plots", k2pirb.event, "fit", k2pirb.is_mc );

        select_fit_dch_plotter->add_dch_selection( cda_cut_35 );

        k2pirb.add_analysis( select_fit_dch_plotter );
    }

    void add_pion_study( K2piRecoBag & k2pirb)
    {
        //**************************************************
        //Selections
        //**************************************************

        //PHOTON SEPARATION
        auto photon_sep_cut =  get_photon_sep_cut(k2pirb, 20  ) ;
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
