#ifndef CORRCLUSTER_HH
#define CORRCLUSTER_HH
#include "RecoCluster.hh"
#include "TVector3.h"
#include "ClusterEnergyCorr.hh"
#if 0
/*
 *   ____                 ____ _           _            
 *  / ___|___  _ __ _ __ / ___| |_   _ ___| |_ ___ _ __ 
 * | |   / _ \| '__| '__| |   | | | | / __| __/ _ \ '__|
 * | |__| (_) | |  | |  | |___| | |_| \__ \ ||  __/ |   
 *  \____\___/|_|  |_|   \____|_|\__,_|___/\__\___|_|   
 *                                                      
 * 
 */
#endif
namespace fn
{
    struct calibrated_cluster_data
    {
        double energy;
        TVector3 position;
    };

    struct uncalibrated_cluster_data
    {
        double energy;
        TVector3 position;
    };

    class CorrCluster
    {
        public:
            //CorrCluster( const fne::RecoCluster& rc, bool mc );
            CorrCluster( const fne::RecoCluster& rc, const ClusterCorrector & cc, bool mc );

            CorrCluster( calibrated_cluster_data cluster, bool mc);
            CorrCluster( uncalibrated_cluster_data cluster, const ClusterCorrector& cc, bool mc);

            virtual const TVector3& get_pos() const;
            virtual double get_energy() const;

            bool has_recorded() const;
            virtual const TVector3& get_recorded_position() const;
            virtual double get_recorded_energy() const;


        private:
            void calibrate( const ClusterCorrector& cc );
            bool mc_;

            double energy_;
            TVector3 position_;

            bool has_recorded_;
            double rec_energy_;
            TVector3 rec_position_;
    };

    //--------------------------------------------------

    TVector3 project_cluster
        ( TVector3 pos, double energy, double project_depth );

    class PhotonProjCorrCluster 
    {
        public:
            PhotonProjCorrCluster( const CorrCluster& cc);
            TVector3 get_pos() const;
            double get_energy() const;

        private:
            const CorrCluster& cc_;
    };

    //--------------------------------------------------

    class TrackProjCorrCluster
    {
        public:
            TrackProjCorrCluster( const CorrCluster& cc);

            TVector3 get_pos() const;
            double get_energy() const;

        private:
            const CorrCluster& cc_;
    };

    //--------------------------------------------------
    
    class ClusterData
    {
        public:
            ClusterData( CorrCluster& c);
            ClusterData(){};
            void update( CorrCluster& c );
            TVector3 position;
            double energy;
    };
}
#endif
