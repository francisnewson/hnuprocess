#include "Km2ClusterPlotter.hh"
#include "Km2Clusters.hh"
#include "Km2Reco.hh"
#include "km2_functions.hh"
#include "yaml_help.hh"


namespace fn
{
    Km2ClusterPlots::Km2ClusterPlots( bool mc, const ClusterCorrector& cluster_corrector)
        :mc_( mc), cluster_corrector_( cluster_corrector)
    {
        h_eop_ = hs_.MakeTH1D( "h_eop_", "E/p for single track in Lkr",
                150, 0, 1.5 , "E/P");

        h_n_ass_cluster_ = hs_.MakeTH1D( "h_ass_ncluster", "Number of clusters in event",
                10, -0.5, 9.5 , "N");

        h_n_bad_cluster_ = hs_.MakeTH1D( "h_bad_ncluster", "Number of clusters in event",
                10, -0.5, 9.5 , "N");

        h_n_ign_cluster_ = hs_.MakeTH1D( "h_ign_ncluster", "Number of clusters in event",
                10, -0.5, 9.5 , "N");

        h_n_cluster_ = hs_.MakeTH1D( "h_ncluster", "Number of clusters in event",
                10, -0.5, 9.5 , "N");

        h_cluster_energy_ = hs_.MakeTH1D( "h_cluster_energy", "All cluster energies",
                100, 0, 100 , "E( GeV )");

        h_cluster_locations_ = hs_.MakeTH2D( "h_cluster_locations", "All cluster locations",
                200, -200, 200 , "x(cm)", 200, -200, 200, "y(cm)" );

         h_non_Ep_ = hs_.MakeTH2D( "h_non_Ep_", "All non-track clusters" ,
                100, 0, 100, "p(GeV)",  100, 0, 100 , "E (GeV)" );

         h_non_rp_ = hs_.MakeTH2D( "h_non_rp_", "All non-track clusters" ,
                100, 0, 100, "p(GeV)",  200, 0, 200 , "r (cm)" );
    }

    void Km2ClusterPlots::Fill( const SingleRecoTrack& srt, 
            const Km2RecoClusters& km2rc,  const Km2Clusters& km2_clusters,
            double weight )
    {
        h_n_ass_cluster_->Fill( km2rc.associate_size(), weight );
        h_n_bad_cluster_->Fill( km2rc.bad_size(), weight );
        h_n_ign_cluster_->Fill( km2rc.ignored_size(), weight );
        h_n_cluster_->Fill( km2rc.all_size(), weight );

        if ( km2rc.associate_size() == 1 )
        {h_eop_->Fill( km2_eop( km2rc,cluster_corrector_,  srt), weight );}

        for( auto itclus = km2rc.all_begin() ; itclus != km2rc.all_end() ; ++itclus )
        {
            CorrCluster cc( **itclus,  cluster_corrector_, mc_ );
            TrackProjCorrCluster track_cluster{ cc};
            h_cluster_energy_->Fill( track_cluster.get_energy() );
            h_cluster_locations_->Fill( (*itclus)->x, (*itclus)->y );

            PhotonProjCorrCluster photon_cluster{ cc} ;
            double x = (*itclus)->x;
            double y = (*itclus)->y;

            if (  km2_clusters.id_cluster( *itclus ) != cluster_type::ASS )
            {
                h_non_Ep_->Fill( srt.get_mom(), photon_cluster.get_energy(), weight );
                h_non_rp_->Fill( srt.get_mom(), std::hypot( x, y ), weight );
            }
        }
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


        km2_cluster_plots_.Fill( *srt, km2rc, km2_clusters_,  get_weight() );
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
