#include "Km2ClusterPlotter.hh"
#include "Km2Clusters.hh"
#include "Km2Reco.hh"
#include "km2_functions.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"


namespace fn
{

    void ClusterHists::init( HistStore& hs, std::string prefix, std::string name )
    {
        h_n = hs.MakeTH1D( 
                Form( "h_n_%s", prefix.c_str() ), 
                Form( "N Clusters for %s", name.c_str() ),
                10, -0.5, 9.5, "N" );

        h_E = hs.MakeTH1D( 
                Form( "h_E_%s", prefix.c_str() ), 
                Form( "Cluster energy for %s", name.c_str() ),
                100, 0, 100, "E (GeV)" );

        h_ds = hs.MakeTH1D( 
                Form( "h_ds_%s", prefix.c_str() ), 
                Form( "Track cluster distance for %s", name.c_str() ),
                400, 0, 400, "ds (cm)" );

        h_t = hs.MakeTH1D( 
                Form( "h_t_%s", prefix.c_str() ), 
                Form( "Cluster time for %s", name.c_str() ),
                100, -25, 25, "E (GeV)" );

        h_xy = hs.MakeTH2D( 
                Form( "h_xy_%s", prefix.c_str() ), 
                Form( "Cluster position for %s", name.c_str() ),
                400, -200, 200 , "x(cm)", 400, -200, 200, "y(cm)" );

        h_gt = hs.MakeTH1D( 
                Form( "h_gt_%s", prefix.c_str() ), 
                Form( "Photon angle for %s", name.c_str() ),
                100, 0, 0.05, "angle (rad)" );

        h_Ep = hs.MakeTH2D( 
                Form( "h_Ep_%s", prefix.c_str() ), 
                Form( "Cluster energy vs track momentum for %s", name.c_str() ),
                100, 0, 100, "p(GeV)",  100, 0, 100 , "E (GeV)" );

        h_Em2m = hs.MakeTH2D( 
                Form( "h_Em2m_%s", prefix.c_str() ), 
                Form( "Cluster energy vs m2m for %s", name.c_str() ),
                100, 0, 0.2, "m(GeV^2)",  100, 0, 100 , "E (GeV)" );

        h_pr = hs.MakeTH2D( 
                Form( "h_pr_%s", prefix.c_str() ), 
                Form( "Cluster radius vs track momentum for %s", name.c_str() ),
                100, 0, 100, "p(GeV)",  100, 0, 200 , "r (cm)" );
    }

    void ClusterHists::fill( Km2RecoClusters::const_iterator begin, Km2RecoClusters::const_iterator end,
            double track_momentum, double m2m, double track_time, const SingleRecoTrack& srt, const TVector3& vertex,
            const ClusterCorrector& cluster_corrector, bool mc, double weight )
    {
        h_n->Fill( std::distance( begin, end ) );

        for ( auto itclus = begin ; itclus != end ; ++itclus )
        {
            double x= (*itclus)->x;
            double y= (*itclus)->y;

            CorrCluster cc( **itclus,  cluster_corrector, mc );
            TrackProjCorrCluster track_cluster{cc};
            TVector3 track_lkr = srt.extrapolate_ds( track_cluster.get_pos().Z() );

            PhotonProjCorrCluster photon_cluster{cc};
            double photon_angle = (photon_cluster.get_pos() - vertex ).Theta();
            h_gt->Fill( photon_angle, weight );

            h_E->Fill( track_cluster.get_energy(), weight );
            h_ds->Fill( (track_cluster.get_pos() - track_lkr).Mag() );
            h_t->Fill( (*itclus)->time - track_time );
            h_xy->Fill( x, y, weight );
            h_Ep->Fill( track_momentum, track_cluster.get_energy(), weight );
            h_pr->Fill( track_momentum, std::hypot(x, y ), weight );
            h_Em2m->Fill( m2m, track_cluster.get_energy(), weight );
        }
    }


    Km2ClusterPlots::Km2ClusterPlots( bool mc, const ClusterCorrector& cluster_corrector)
        :mc_( mc), cluster_corrector_( cluster_corrector)
    {
        all_clusters_.init( hs_, "all", "all clusters" );
        bad_clusters_.init( hs_, "bad", "bad clusters" ) ;
        ign_clusters_.init( hs_, "ign", "ignored clusters" );
        ass_clusters_.init( hs_, "ass", "associated clusters" );
        not_ass_clusters_.init( hs_, "not_ass", "un-associated clusters" );

        h_eop_ = hs_.MakeTH1D( "h_eop_", "E/p for single track in Lkr",
                150, 0, 1.5 , "E/P");
    }

    void Km2ClusterPlots::Fill( const SingleRecoTrack& srt,  const Km2RecoEvent& km2re,
            const Km2RecoClusters& km2rc, const Km2Clusters& km2_clusters,
            double weight )
    {
        double m2m = km2re.get_m2m_kmu();
        double track_time = srt.get_time();
        TVector3 vertex = srt.get_vertex();

        all_clusters_.fill( km2rc.all_begin(), km2rc.all_end(), 
                srt.get_mom(), m2m, track_time, srt, vertex, cluster_corrector_, mc_, weight);

        bad_clusters_.fill( km2rc.bad_begin(), km2rc.bad_end(), 
                srt.get_mom(), m2m, track_time, srt, vertex, cluster_corrector_, mc_, weight);

        ign_clusters_.fill( km2rc.ignored_begin(), km2rc.ignored_end(), 
                srt.get_mom(), m2m, track_time, srt,vertex,  cluster_corrector_, mc_, weight);

        ass_clusters_.fill( km2rc.associate_begin(), km2rc.associate_end(), 
                srt.get_mom(), m2m, track_time, srt, vertex, cluster_corrector_, mc_, weight);

        not_ass_clusters_.fill( km2rc.not_associate_begin(), km2rc.not_associate_end(), 
                srt.get_mom(), m2m, track_time, srt, vertex, cluster_corrector_, mc_, weight);

        if ( km2rc.associate_size() == 1 )
        {h_eop_->Fill( km2_eop( km2rc,cluster_corrector_,  srt), weight );}
    }

    void Km2ClusterPlots::Write()
    {
        hs_.Write();
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2ClusterPlotter);

    Km2ClusterPlotter::Km2ClusterPlotter( const Selection& sel, 
            TFile& tfile, std::string folder,
            const Km2Event& km2_event,
            const  Km2Clusters& km2_clusters,
            const ClusterCorrector& cluster_corrector, bool mc)
        :Analysis( sel ), tfile_( tfile ), folder_( folder ),
        km2_event_( km2_event), km2_clusters_( km2_clusters ), 
        km2_cluster_plots_( mc, cluster_corrector )
    {}

    void Km2ClusterPlotter::process_event()
    {
        const Km2RecoEvent&  km2re = km2_event_.get_reco_event();
        const SingleRecoTrack * srt =  km2re.get_reco_track();
        const Km2RecoClusters& km2rc = km2_clusters_.get_reco_clusters();

        km2_cluster_plots_.Fill( *srt, km2re, km2rc, km2_clusters_,  get_weight() );
    }

    void Km2ClusterPlotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        km2_cluster_plots_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2ClusterPlotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );
            const Km2Clusters* km2c  = get_km2_clusters( instruct, rf );
            const ClusterCorrector * cluster_corrector = get_cluster_corrector( instruct, rf );

            bool is_mc  = rf.is_mc();

            return new Km2ClusterPlotter( *sel, tfile, folder, *km2_event,
                    *km2c, *cluster_corrector, is_mc);
        }
}
