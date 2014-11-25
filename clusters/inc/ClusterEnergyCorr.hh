#ifndef CLUSTERENERGYCORR_HH
#define CLUSTERENERGYCORR_HH
#include <string>
#include "RecoCluster.hh"
#include "Rtypes.h"
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
            double operator()(const fne::RecoCluster& rc, bool is_mc ) const;
            double correct_energy(const fne::RecoCluster& rc, bool is_mc) const;


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

    double correct_eop_energy( const fne::RecoCluster& re, bool is_mc );

    double user_lkrcalcor_SC(double energy, Long64_t run , int iflag);

    std::pair<int, int> get_cpd_cell_index( double pos_x, double pos_y);

}
#endif
