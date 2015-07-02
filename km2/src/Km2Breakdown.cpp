#include "Km2Breakdown.hh"
#include "Km2Reco.hh"
#include "SingleTrack.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "NA62Constants.hh"
#include "Km2Clusters.hh"
#include "ClusterEnergyCorr.hh"
#include "stl_help.hh"

namespace fn
{
    //--------------------------------------------------

    //M2M QUALITY
    template <> TH1D * hinit<mp_m2m>( HistStore& hs ) { 
        return hs.MakeTH1D(
                "h_km2_m2m", "MissingMass2", 1000, -0.7, 0.3, "m^{2}_{miss} (GeV^{2}/c)"); }

    template <> double hfill<mp_m2m>( Km2Breakdown& km2b ) { 
        return km2b.get_reco_event().get_m2m_kmu(); }

    //TRACK QUALITY
    template <> TH1D * hinit<mp_track_quality>( HistStore& hs ) { 
        return hs.MakeTH1D(
                "hTrackQuality", "TrackQuality", 120, 0.0, 1.2, "quality"); }

    template <> double hfill<mp_track_quality>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().get_quality(); }


    //CDA
    template <> TH1D * hinit<mp_cda>( HistStore& hs ) { 
        return hs.MakeTH1D(
                "hCda", "CDA", 160, -1, 15, "CDA (cm)" ); }

    template <> double hfill<mp_cda>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().get_cda(); }

    //DCH1
    template <> TH1D * hinit<mp_r_dch1>( HistStore& hs ) { 
        return hs.MakeTH1D(
                "h_r_dch1", "Radius at DCH1", 200, 0, 200, "r at DCH1 (cm)" ); }

    template <> double hfill<mp_r_dch1>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().extrapolate_us( na62const::zDch1).Perp(); }

    //DCH4
    template <> TH1D * hinit<mp_r_dch4>( HistStore& hs ) { 
        return hs.MakeTH1D(
                "h_r_dch4", "Radius at DCH4", 200, 0, 200, "r at DCH4 (cm)" ); }

    template <> double hfill<mp_r_dch4>( Km2Breakdown& km2b ) { 
        return km2b.get_single_reco_track().extrapolate_us( na62const::zDch4).Perp(); }

    //LKR
    template <> TH2D * h2init<mp_xy_lkr>( HistStore& hs ){
        return hs.MakeTH2D(
                "h_xy_lkr", "XY at LKr", 200, -200, 200, "x (cm)", 200, -200, 200, "y (cm)"); }

    template <> std::pair<double,double> h2fill<mp_xy_lkr>( Km2Breakdown& km2b ) {
        TVector3 point = km2b.get_single_reco_track().extrapolate_ds( na62const::zLkr);
        return std::make_pair( point.X(), point.Y() ); }

    //DCHT
    template <> TH1D * hinit<mp_dcht>( HistStore& hs ) {
        return hs.MakeTH1D(
                "h_dcht", "t DCH (ns)", 200, -100, 100, "t DCH (ns) " ); }
    template <> double hfill<mp_dcht>( Km2Breakdown& km2b ) {
        return km2b.get_single_reco_track().get_time(); }

    //CLUSTER DISTANCE
    void  ClusterMiniPlot::init( HistStore& hs ){
        h_ds_ =  hs.MakeTH1D(
                "h_cluster_distance", "Cluster distance(cm)", 300, 0, 300, "d (cm) " );

        h_E_ =  hs.MakeTH1D(
                "h_cluster_energy", "Cluster energy(cm)", 200, 0, 100, "d (cm) " );

        h_ds_E_ =  hs.MakeTH2D(
                "h_cluster_distance_energy", "Cluster distance and energy" , 
                300, 0, 300 , "Cluster distance(cm)",
                200, 0, 100, "Cluster energy (GeV)" );

        h_ds_t_ =  hs.MakeTH2D(
                "h_cluster_distance_time", "Cluster distance and time" , 
                300, 0, 300 , "Cluster distance(cm)",
                200, -50, 50, "Cluster time (ns)" );
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

                h_ds_->Fill( track_cluster_sep, wgt );
                h_E_->Fill( track_cluster.get_energy(), wgt );
                h_ds_E_->Fill( track_cluster_sep, track_cluster.get_energy(), wgt );
                h_ds_t_->Fill( track_cluster_sep, (*clus)->time - srt.get_time(), wgt );

#if 0
                if ( track_cluster_sep > furthest_distance )
                {
                    furthest_distance = track_cluster_sep;
                    furthest_energy = track_cluster.get_energy();
                }
#endif
            }
#if 0
            h_ds_->Fill( furthest_distance, wgt );
            h_E_->Fill( furthest_energy, wgt );
            h_ds_E_->Fill( furthest_distance, furthest_energy, wgt );
#endif
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
    template <> TH2D * h2init<mp_zt>( HistStore& hs ){
        return hs.MakeTH2D(
                "h_zt", "T vs Z",
                150, -5000, 10000, "Z( cm) ",
                250, 0, 25e-3, "t( rad) "); }

    template <> std::pair<double,double> h2fill<mp_zt>( Km2Breakdown& km2b ) {
        auto e = km2b.get_reco_event();
        return std::make_pair( e.get_zvertex(), e.get_opening_angle() ); }

    //TPHI
    template <> TH2D * h2init<mp_tphi>( HistStore& hs )
    {
        return hs.MakeTH2D(
                "h_tphi", "phi vs T",
                250, 0, 25e-3, "t( rad) ",
                300, 0, 2 * na62const::pi, "phi( rad) " );
    }

    template <> std::pair<double,double> h2fill<mp_tphi>( Km2Breakdown& km2b )
    {
        auto e = km2b.get_reco_event();
        return std::make_pair( e.get_opening_angle(), e.get_muon_phi() );
    }

    //--------------------------------------------------

    SelPlot::SelPlot( TFile& tf, std::string folder, MiniPlotMap& mpm  )
        :tf_( tf ), folder_( folder ), mpm_( mpm )
    {}

    void SelPlot::AddMiniPlot( std::string plot_type )
    {
        //Load pre plot
        plot_store_.emplace_back( std::unique_ptr<MiniPlot>{
                mpm_.at( plot_type)( pre_hs_ ) } );

        pre_.push_back( plot_store_.back().get() );

        //Load post plot
        plot_store_.emplace_back( std::unique_ptr<MiniPlot>{
                mpm_.at( plot_type)( post_hs_ )} );

        post_.push_back( plot_store_.back().get() );

        assert( pre_.size() + post_.size() == plot_store_.size() );
    }

    void SelPlot::Fill( Km2Breakdown& km2b, double pre_wgt,  bool passed, double post_wgt )
    {
        for ( auto mp : pre_ )
        {
            mp->fill( km2b, pre_wgt );
        }

        if ( passed )
        {
            for ( auto mp : post_ )
            {
                mp->fill( km2b, post_wgt );
            }
        }
    }

    void SelPlot::Write()
    {
        if ( pre_hs_.size() > 0 )
        {
        cd_p( &tf_,  boost::filesystem::path{folder_}/ "pre" );
        pre_hs_.Write();

        cd_p( &tf_,  boost::filesystem::path{folder_}/ "post" );
        post_hs_.Write();
        }
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Breakdown );

    Km2Breakdown::Km2Breakdown( const Selection& sel, const Km2Event& km2e,
            const Selection& good_track, Km2Clusters& km2c,
            TFile& tf, std::string folder)
        :Analysis( sel ), km2e_( km2e ), good_track_( good_track), km2c_( km2c ),
        tf_(tf), folder_( folder )
    {
        register_plotter<Mini1DPlot<mp_m2m>>( "m2m" );
        register_plotter<Mini1DPlot<mp_track_quality>>( "track_quality" );
        register_plotter<Mini1DPlot<mp_cda>>( "cda" );
        register_plotter<Mini1DPlot<mp_r_dch1>>( "rdch1" );
        register_plotter<Mini1DPlot<mp_r_dch4>>( "rdch4" );
        register_plotter<Mini2DPlot<mp_xy_lkr>>( "xylkr" );
        register_plotter<Mini1DPlot<mp_dcht>>( "dcht" );
        register_plotter<ClusterMiniPlot>( "rcluster" );
        register_plotter<Mini2DPlot<mp_zt>>( "zt" );
        register_plotter<Mini2DPlot<mp_tphi>>( "tphi" );
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
        boost::filesystem::path folder_name = boost::filesystem::path{ folder_ } / (prefix + s->get_name());

        sel_plot_store_.emplace_back( make_unique<SelPlot>( tf_, folder_name.string() , mini_plot_map_ ) );

        for( auto& plot_type : plot_types )
        {
            sel_plot_store_.back()->AddMiniPlot( plot_type );
        }

        sel_plots_.push_back( std::make_pair( s, sel_plot_store_.back().get() ) );
    }

    void Km2Breakdown::process_event()
    {
        if ( good_track_.check() )
        {
            km2re_ = &km2e_.get_reco_event();
            km2rc_ = &km2c_.get_reco_clusters();
        }

        CompositeSelection selection_so_far;


        for( auto& sel_plot : sel_plots_ )
        {
            double wgt_so_far = selection_so_far.get_weight();
            double passed = sel_plot.first->check();
            double pass_wgt = sel_plot.first->get_weight() * wgt_so_far;

            sel_plot.second->Fill( *this, wgt_so_far, passed, pass_wgt );

            if ( passed )
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
        for ( auto& sel_plot : sel_plots_ )
        {
            sel_plot.second->Write();
        }
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
