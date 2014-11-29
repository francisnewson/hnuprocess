#ifndef KM2_FUNCTIONS_HH
#define KM2_FUNCTIONS_HH

namespace fn
{
    class Km2RecoClusters;
    class SingleRecoTrack;
    class ClusterCorrector;
    double km2_eop( const Km2RecoClusters& km2rc, 
            const ClusterCorrector& cluster_corrector, const SingleRecoTrack& srt );

}
#endif
