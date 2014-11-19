#include "km2_functions.hh"
#include "Km2Clusters.hh"
#include "SingleTrack.hh"

namespace fn
{
    double km2_eop( const Km2RecoClusters& km2rc, const SingleRecoTrack& srt )
    {
        if (  km2rc.associate_size() != 1 )
        {
            throw std::runtime_error( "km2_eop with n_cluster != 1 " );
        }

        const fne::RecoCluster* tc = * (km2rc.associate_begin() );
        TrackProjCorrCluster tcep( *tc );
        double track_cluster_energy = tcep.get_energy();

        double track_mom = srt.get_mom();
        double eop = track_cluster_energy / track_mom;
        return eop;
    }
}
