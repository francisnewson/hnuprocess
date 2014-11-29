#include "CorrClusterPlotter.hh"
#include "Event.hh"
#include "ClusterEnergyCorr.hh"
#include "CorrCluster.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include <iomanip>

namespace fn
{
    CorrClusterPlots::CorrClusterPlots()
    {
        h_dE = hs_.MakeTH1D( "h_dE", "Lkr Energy Correction",
                1000, -5, 5, "Correction( GeV)" );

        h_E = hs_.MakeTH1D( "h_E", "Lkr Energy ",
                1000, 0, 100, "Correction( GeV)" );

        h_dEvsE = hs_.MakeTH2D( "h_dEvsE", "Lkr Energy Correction vs Energy",
                100, 0, 100, "Energy( GeV)",
                300, -3, 3, "Correction (GeV)"
                );

        h_dXY = hs_.MakeTH2D( "h_dXY", "Cluster position correction",
                100, -0.5, 0.5, "dX (cm)", 100, -0.5, 0.5, "dY (cm)" );

    }

    void CorrClusterPlots::Fill( const CorrCluster & c, double weight )
    {

        if ( c.has_recorded() )
        {
            h_E->Fill( c.get_energy(), weight );
            h_dE->Fill( c.get_energy() - c.get_recorded_energy(), weight );
            h_dEvsE->Fill( c.get_energy(),
                    c.get_energy() - c.get_recorded_energy(), weight );

            TVector3 rec = c.get_recorded_position();
            TVector3 pos = c.get_pos();
            TVector3 delta= pos - rec;

            h_dXY->Fill(  delta.X(), delta.Y(), weight );
        }
    }

    void CorrClusterPlots::Write()
    {
        hs_.Write();
    }

    //--------------------------------------------------

    REG_DEF_SUB( CorrClusterPlotter );

    CorrClusterPlotter::CorrClusterPlotter( const Selection& sel, const fne::Event * e, 
            const ClusterCorrector& cluster_corrector, 
            TFile& tf, std::string folder )
        :Analysis( sel ), e_( e ), cluster_corrector_( cluster_corrector ),
        tf_( tf ), folder_( folder )
    {}

    void CorrClusterPlotter::process_event()
    {
        //Loop over all raw clusters
        //Extract clusters from event
        int nclusters = e_->detector.nclusters;
        auto& eclusters = e_->detector.clusters;

        //Loop over clusters - Ignore noise-like clusters
        for (  int iclus = 0 ; iclus != nclusters ; ++iclus )
        {

            fne::RecoCluster & rc =  * static_cast<fne::RecoCluster*>( eclusters[iclus] );
            CorrCluster cc ( rc, cluster_corrector_, cluster_corrector_.is_mc() );
            cluster_plots_.Fill( cc, get_weight() );
        }
    }

    void CorrClusterPlotter::end_processing()
    {
        cd_p( &tf_, folder_ );
        cluster_plots_.Write();
    }

    template<>
        Subscriber * create_subscriber<CorrClusterPlotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * e = rf.get_event_ptr();

            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tf= rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const ClusterCorrector * cluster_corrector =
                get_cluster_corrector( instruct, rf );

            return new CorrClusterPlotter( *sel, e, *cluster_corrector, tf, folder );

        }
}
