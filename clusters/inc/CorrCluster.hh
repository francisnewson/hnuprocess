#ifndef CORRCLUSTER_HH
#define CORRCLUSTER_HH
#include "RecoCluster.hh"
#include "TVector3.h"
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

    class PhotonProjCorrCluster : public CorrCluster
    {
        public:
            PhotonProjCorrCluster
                ( const fne::RecoCluster& rc );

            TVector3 get_pos() const;

            static const double lkr_project_dist;
    };
}
#endif
