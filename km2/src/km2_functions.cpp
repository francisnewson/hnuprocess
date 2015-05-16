#include "km2_functions.hh"
#include "CorrCluster.hh"
#include "ClusterEnergyCorr.hh"
#include "Km2Clusters.hh"
#include "SingleTrack.hh"

namespace fn
{
    double km2_eop( const Km2RecoClusters& km2rc, 
            const ClusterCorrector& cluster_corrector, const SingleRecoTrack& srt )
    {
        if (  km2rc.associate_size() != 1 )
        {
            throw std::runtime_error( "km2_eop with n_cluster != 1 " );
        }

        const fne::RecoCluster* tc = * (km2rc.associate_begin() );
        CorrCluster cc( *tc, cluster_corrector  , km2rc.is_mc() );
        TrackProjCorrCluster tcep( cc );
        double track_cluster_energy = tcep.get_energy();

        double track_mom = srt.get_mom();
        double eop = track_cluster_energy / track_mom;
        return eop;
    }


    bool extra_lkr_acc( double pos_x, double pos_y)
    {
        //ignore inner radius
        double radius = std::hypot( pos_x, pos_y );
        if ( radius < 20.0 ){ return false; }

        //ignore hot cells
        std::pair<int, int> cpd_cell = get_cpd_cell_index( pos_x, pos_y );
        if ( cpd_cell.first == 134 && ( cpd_cell.second == 56 || cpd_cell.second == 57 ) )
        {
            return false;
        }

        return true;
    }
}
