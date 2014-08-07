#include "CorrCluster.hh"
#include "NA62Constants.hh"
#include <cmath>

namespace fn
{
    CorrCluster::CorrCluster( const fne::RecoCluster& rc )
        :rc_( rc ){}

    TVector3 CorrCluster::get_pos() const
    {
        return TVector3( 
                rc_.x + 0.136 + 0.87e-3*rc_.y, 
                rc_.y + 0.300 - 0.87e-3*rc_.x,
                na62const::zLkr );
    }

    double CorrCluster::get_energy() const
    {
        return rc_.energy;
    }

    //--------------------------------------------------

    PhotonProjCorrCluster::PhotonProjCorrCluster( const fne::RecoCluster& rc )
        :CorrCluster( rc ){}

    const double PhotonProjCorrCluster::lkr_project_dist = 10998;

    TVector3 PhotonProjCorrCluster::get_pos() const
    {
        //get rotated position
        TVector3 result = CorrCluster::get_pos();
        double cluster_depth = 20.8 + 4.3 * std::log( get_energy() );

        result.SetX( result.X() * ( 1 + cluster_depth / lkr_project_dist ) );
        result.SetY( result.Y() * ( 1 + cluster_depth / lkr_project_dist ) );
        result.SetZ( result.Z() + cluster_depth );

        return result;
    }
}
