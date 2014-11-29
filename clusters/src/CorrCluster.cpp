#include "CorrCluster.hh"
#include "NA62Constants.hh"
#include <cmath>

namespace fn
{
    //CALIBRATION ------------------------------

    void CorrCluster::calibrate( const ClusterCorrector& cc )
    {
        energy_ = cc.correct_energy( rec_energy_, rec_position_.X(), rec_position_.Y(), mc_ );

        if ( mc_ )
        {
            position_ = rec_position_;

        }
        else
        {
            double x = rec_position_.X();
            double y = rec_position_.Y();
            position_.SetXYZ(
                    x + 0.136 + 0.87e-3*y, 
                    y + 0.300 - 0.87e-3*x,
                    na62const::zLkr );
        }
    }

    //CONSTRUCTORS ------------------------------

    CorrCluster::CorrCluster( const fne::RecoCluster& rc, const ClusterCorrector & cc, bool mc  )
        :mc_( mc) , rec_energy_( rc.energy ), rec_position_( rc.x, rc.y, na62const::zLkr )
    {
        calibrate( cc );
        has_recorded_ = true; 
    }

    CorrCluster::CorrCluster( calibrated_cluster_data cluster, bool mc)
        :mc_( mc), energy_( cluster.energy), position_( cluster.energy), 
        has_recorded_ (false )
    {}

    CorrCluster::CorrCluster( uncalibrated_cluster_data cluster,
            const ClusterCorrector& cc, bool mc)
        :mc_( mc ), rec_energy_( cluster.energy), rec_position_( cluster.position )
    {
        calibrate( cc );
        has_recorded_ = true; 
    }

    //GETTERS ----------------------------------

    bool CorrCluster::has_recorded() const
    {
        return has_recorded_;
    }

    const TVector3& CorrCluster::get_recorded_position() const
    {
        if ( ! has_recorded() )
        { throw std::runtime_error
            ( "CorrCluster: Request for unknown recorded position" ); }

        return rec_position_;
    }

    double CorrCluster::get_recorded_energy() const
    {
        if ( ! has_recorded() )
        { throw std::runtime_error
            ( "CorrCluster: Request for unknown recorded energy" ); }

        return rec_energy_;
    }

    const TVector3& CorrCluster::get_pos() const
    {
        return position_;
    }

    double CorrCluster::get_energy() const
    {
        return energy_;
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

    PhotonProjCorrCluster::PhotonProjCorrCluster( const CorrCluster& cc )
        :cc_( cc ) {}

        TVector3 PhotonProjCorrCluster::get_pos() const
        {
            return project_cluster(cc_.get_pos(), cc_.get_energy(), 20.8 );
        }

    double PhotonProjCorrCluster::get_energy() const
    {
        return cc_.get_energy();
    }

    //--------------------------------------------------

    TrackProjCorrCluster::TrackProjCorrCluster ( const CorrCluster& cc)
        :cc_( cc ) {}

    TVector3 TrackProjCorrCluster::get_pos() const
    {
        return project_cluster( cc_.get_pos(), cc_.get_energy(),  16.5 );
    }

    double TrackProjCorrCluster::get_energy() const
    {
        return cc_.get_energy();
    }

    //--------------------------------------------------

    ClusterData::ClusterData( CorrCluster&  c )
        :position( c.get_pos() ), energy( c.get_energy())
    {}

    void ClusterData::update( CorrCluster& c )
    {
        position = c.get_pos();
        energy = c.get_energy();
    }
}
