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
    class CorrCluster
    {
        public:
            CorrCluster( const fne::RecoCluster& rc );
            virtual TVector3 get_pos() const;
            virtual double get_energy() const;

        protected:
            const fne::RecoCluster & rc_;
    };

    //--------------------------------------------------

    TVector3 project_cluster
        ( TVector3 pos, double energy, double project_depth );

    class PhotonProjCorrCluster : public CorrCluster
    {
        public:
            PhotonProjCorrCluster
                ( const fne::RecoCluster& rc );

            TVector3 get_pos() const;
    };

    //--------------------------------------------------

    class TrackProjCorrCluster : public CorrCluster
    {
        public:
            TrackProjCorrCluster
                ( const fne::RecoCluster& rc );

            TVector3 get_pos() const;
            double get_energy() const;
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
