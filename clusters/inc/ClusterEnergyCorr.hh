#ifndef CLUSTERENERGYCORR_HH
#define CLUSTERENERGYCORR_HH
#include <string>
#include "RecoCluster.hh"
#if 0
/*
 *   ____ _           _            _____                            
 *  / ___| |_   _ ___| |_ ___ _ __| ____|_ __   ___ _ __ __ _ _   _ 
 * | |   | | | | / __| __/ _ \ '__|  _| | '_ \ / _ \ '__/ _` | | | |
 * | |___| | |_| \__ \ ||  __/ |  | |___| | | |  __/ | | (_| | |_| |
 *  \____|_|\__,_|___/\__\___|_|  |_____|_| |_|\___|_|  \__, |\__, |
 *                                                      |___/ |___/ 
 *    ____                
 *   / ___|___  _ __ _ __ 
 *  | |   / _ \| '__| '__|
 *  | |__| (_) | |  | |   
 *   \____\___/|_|  |_|   
 */                        
#endif
namespace fn
{
    /*****************************************************
     * Cluster Energy Corr
     *
     * Apply cell dependent energy corrections to LKr.
     * Code from here:
     * http://goudzovs.web.cern.ch/goudzovs/ke2/EopRecal.c
     *
     *****************************************************/

    class ClusterEnergyCorr
    {
        public:
            ClusterEnergyCorr( std::string filename );
            double operator()(fne::RecoCluster& rc) const;
            double correct_energy(fne::RecoCluster& rc) const;


            int GetCpdCellIndex
                (double pos_x, double pos_y,
                 int *cpd_index, int *cell_index) const;

        private:
            float EopCorr[256][64];

            float CPDpos_leftDownCorner[256][2];
            float CELLpos_leftDownCorner[256][64][2];

            float CPDlength;
            float  CELLlength;
    };
}
#endif