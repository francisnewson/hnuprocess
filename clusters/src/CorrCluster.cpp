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

    TVector3 project_cluster
        ( TVector3 pos, double energy, double project_depth )
        {
            double lkr_project_dist = 10998;
            TVector3 result = pos;
            double cluster_depth = project_depth + 4.3 * std::log( energy);

            result.SetX( result.X() * ( 1 + cluster_depth / lkr_project_dist ) );
            result.SetY( result.Y() * ( 1 + cluster_depth / lkr_project_dist ) );
            result.SetZ( result.Z() + cluster_depth );

            return result;
        }

    //--------------------------------------------------

    PhotonProjCorrCluster::PhotonProjCorrCluster( const fne::RecoCluster& rc )
        :CorrCluster( rc ){}

    TVector3 PhotonProjCorrCluster::get_pos() const
    {
        return project_cluster( CorrCluster::get_pos(), get_energy(), 20.8 );
    }

    //--------------------------------------------------

    TrackProjCorrCluster::TrackProjCorrCluster
        ( const fne::RecoCluster& rc )
        :CorrCluster( rc )
        {}

    TVector3 TrackProjCorrCluster::get_pos() const
    {
        return project_cluster( CorrCluster::get_pos(), get_energy(),  16.5 );
    }

    double TrackProjCorrCluster::get_energy() const
    {
        return correct_eop_energy( rc_ );
    }
}
