#include "Km2Breakdown.hh"
#include "Km2Reco.hh"
#include "SingleTrack.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "NA62Constants.hh"
#include "Km2Clusters.hh"
#include "ClusterEnergyCorr.hh"

namespace fn
{
    //--------------------------------------------------

    //TRACK QUALITY
    template <> TH1D * hinit<mp_track_quality>( Km2Breakdown& km2b ) { 
        return km2b.get_hist_store().MakeTH1D(
                "hTrackQuality", "TrackQuality", 120, 0.0, 1.2, "quality"); }

    template <> double hfill<mp_track_quality>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().get_quality(); }


    //CDA
    template <> TH1D * hinit<mp_cda>( Km2Breakdown& km2b ) { 
        return km2b.get_hist_store().MakeTH1D(
                "hCda", "CDA", 160, -1, 15, "CDA (cm)" ); }

    template <> double hfill<mp_cda>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().get_cda(); }

    //DCH1
    template <> TH1D * hinit<mp_r_dch1>( Km2Breakdown& km2b ) { 
        return km2b.get_hist_store().MakeTH1D(
                "h_r_dch1", "Radius at DCH1", 200, 0, 200, "r at DCH1 (cm)" ); }

    template <> double hfill<mp_r_dch1>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().extrapolate_us( na62const::zDch1).Perp(); }

    //DCH4
    template <> TH1D * hinit<mp_r_dch4>( Km2Breakdown& km2b ) { 
        return km2b.get_hist_store().MakeTH1D(
                "h_r_dch4", "Radius at DCH4", 200, 0, 200, "r at DCH4 (cm)" ); }

    template <> double hfill<mp_r_dch4>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().extrapolate_us( na62const::zDch4).Perp(); }

    //LKR
    template <> TH2D * h2init<mp_xy_lkr>( Km2Breakdown& km2b ){
        return km2b.get_hist_store().MakeTH2D(
                "h_xy_lkr", "XY at LKr", 200, -200, 200, "x (cm)", 200, -200, 200, "y (cm)"); }

    template <> std::pair<double,double> h2fill<mp_xy_lkr>( Km2Breakdown& km2b ) {
        TVector3 point = km2b.get_single_reco_track().extrapolate_ds( na62const::zLkr);
        return std::make_pair( point.X(), point.Y() ); }

    //DCHT
    template <> TH1D * hinit<mp_dcht>( Km2Breakdown& km2b ) {
        return km2b.get_hist_store().MakeTH1D(
                "h_dcht", "t DCH (ns)", 200, -100, 100, "t DCH (ns) " ); }
    template <> double hfill<mp_dcht>( Km2Breakdown& km2b ) {
        return km2b.get_single_reco_track().get_time(); }

    //CLUSTER DISTANCE
    void  ClusterMiniPlot::init( Km2Breakdown& km2b ){
        h_ds_ =  km2b.get_hist_store().MakeTH1D(
                "h_cluster_distance", "Cluster distance(cm)", 300, 0, 300, "d (cm) " );
        h_E_ =  km2b.get_hist_store().MakeTH1D(
                "h_cluster_energy", "Cluster energy(cm)", 200, 0, 100, "d (cm) " );

        h_ds_E_ =  km2b.get_hist_store().MakeTH2D(
                "h_cluster_distance_energy", "Cluster distance and energy" , 
                300, 0, 300 , "Cluster distance(cm)",
                200, 0, 100, "Cluster energy (GeV)" );
    }

    void ClusterMiniPlot::fill( Km2Breakdown& km2b, double wgt ){

        const Km2RecoClusters& km2rc = km2b.get_reco_clusters();
        const ClusterCorrector& cluster_corrector = km2rc.get_cluster_corrector();
        bool ismc= km2rc.is_mc();
        const SingleRecoTrack& srt = km2b.get_single_reco_track();
        double furthest_distance = 0;
        double furthest_energy = 0;

        if ( km2rc.all_size() > 0 )
        {

            for ( auto clus = km2rc.all_begin() ; clus != km2rc.all_end() ; ++clus )
            {
                CorrCluster  cc{ **clus, cluster_corrector, ismc };
                TrackProjCorrCluster track_cluster{ cc };
                TVector3 cluster_pos = track_cluster.get_pos();
                TVector3 trk_lkr = srt.extrapolate_ds( cluster_pos.Z() );
                double track_cluster_sep = (trk_lkr - cluster_pos ).Mag();
                if ( track_cluster_sep > furthest_distance )
                {
                    furthest_distance = track_cluster_sep;
                    furthest_energy = track_cluster.get_energy();
                }
            }
            h_ds_->Fill( furthest_distance, wgt );
            h_E_->Fill( furthest_energy, wgt );
            h_ds_E_->Fill( furthest_distance, furthest_energy, wgt );
        }
    }

    void ClusterMiniPlot::set_name( std::string name )
    { 
        h_ds_->SetName(  (std::string("h_ds_") + name.c_str() ).c_str() );
        h_E_->SetName(  (std::string("h_E_") + name.c_str() ).c_str() );
        h_ds_E_->SetName(  (std::string("h_ds_E_") + name.c_str() ).c_str() );
    }

    std::string ClusterMiniPlot::get_name()
    { return ""; }

    //LKR
    template <> TH2D * h2init<mp_zt>( Km2Breakdown& km2b ){
        return km2b.get_hist_store().MakeTH2D(
                "h_zt", "T vs Z",
                150, -5000, 10000, "Z( cm) ",
                250, 0, 25e-3, "t( rad) "); }

    template <> std::pair<double,double> h2fill<mp_zt>( Km2Breakdown& km2b ) {
        auto e = km2b.get_reco_event();
        return std::make_pair( e.get_zvertex(), e.get_opening_angle() ); }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Breakdown );

    Km2Breakdown::Km2Breakdown( const Selection& sel, const Km2Event& km2e,
            const Selection& good_track, Km2Clusters& km2c,
            TFile& tf, std::string folder)
        :Analysis( sel ), km2e_( km2e ), good_track_( good_track), km2c_( km2c ),
        tf_(tf), folder_( folder )
    {
        register_plotter<Mini1DPlot<mp_track_quality>>( "track_quality" );
        register_plotter<Mini1DPlot<mp_cda>>( "cda" );
        register_plotter<Mini1DPlot<mp_r_dch1>>( "rdch1" );
        register_plotter<Mini1DPlot<mp_r_dch4>>( "rdch4" );
        register_plotter<Mini2DPlot<mp_xy_lkr>>( "xylkr" );
        register_plotter<Mini1DPlot<mp_dcht>>( "dcht" );
        register_plotter<ClusterMiniPlot>( "rcluster" );
        register_plotter<Mini2DPlot<mp_zt>>( "zt" );
    }

    const SingleRecoTrack& Km2Breakdown::get_single_reco_track()
    {
        return *km2re_->get_reco_track();
    }

    const Km2RecoClusters& Km2Breakdown::get_reco_clusters()
    {
        return *km2rc_;
    }
    const Km2RecoEvent& Km2Breakdown::get_reco_event()
    {
        return *km2re_;
    }

    void Km2Breakdown::add_selection( const Selection * s,
            std::vector<std::string>& plot_types, std::string prefix  )
    {
        std::vector<MiniPlot*> plots;
        for ( auto &plot_type : plot_types )
        {
            plot_store_.emplace_back( std::unique_ptr<MiniPlot>{ 
                    mini_plot_map_.at( plot_type)( *this ) } );

            plot_store_.back()->set_name( prefix  + plot_store_.back()->get_name() );

            plots.push_back( plot_store_.back().get() );
        }
        selection_plots_.push_back( std::make_pair( s, plots ) );
    }

    void Km2Breakdown::process_event()
    {
        if ( good_track_.check() )
        {
            km2re_ = &km2e_.get_reco_event();
            km2rc_ = &km2c_.get_reco_clusters();
        }

        CompositeSelection selection_so_far;

        for ( auto& sel_plot : selection_plots_ )
        {
            BOOST_LOG_SEV( get_log(), log_level() )
                << "Processing: " << sel_plot.first->get_name()  << " " 
                << std::boolalpha << sel_plot.first->check() << std::noboolalpha;

            for ( auto& plot : sel_plot.second )
            {
                plot->fill( *this, selection_so_far.get_weight() );
            }

            if (sel_plot.first->check() )
            {
                selection_so_far.AddChild( sel_plot.first );
            }
            else
            {
                break;
            }
        }
    }

    void Km2Breakdown::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2Breakdown>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Km2Event * km2e = get_km2_event( instruct, rf, "km2event" );

            const Selection * base = rf.get_selection(
                    get_yaml<std::string>( instruct, "base" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Selection *  good_track = 
                rf.get_selection( get_yaml<std::string>( instruct, "good_track" ) );

            auto  * km2_clusters = get_km2_clusters( instruct, rf );

            auto result  = make_unique<Km2Breakdown>( *base, *km2e, *good_track, 
                    *km2_clusters, tfile, folder );

            const YAML::Node& selection_list = instruct["selection"];
            for ( const auto& selmap  : selection_list )
            {
                const Selection * sel 
                    = rf.get_selection( get_yaml<std::string>( selmap, "cut") );

                std::vector<std::string> plots 
                    = selmap["plot"].as<std::vector<std::string>>();

                std::string name = selmap["name"].as<std::string>( "" );

                result->add_selection(  sel, plots, name );
            }
            return result.release();
        }
}
