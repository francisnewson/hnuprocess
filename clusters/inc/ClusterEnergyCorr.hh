#ifndef CLUSTERENERGYCORR_HH
#define CLUSTERENERGYCORR_HH
#include <string>
#include "RecoCluster.hh"
#include "Rtypes.h"
#include "Subscriber.hh"
#include "GlobalStatus.hh"
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

            double correct_energy(const fne::RecoCluster& rc, 
                    bool is_mc, Long64_t run) const;

            double correct_energy( double x, double y, double energy,  bool is_mc ) const;

            double correct_energy( double x, double y, double energy, 
                    bool is_mc , Long64_t run ) const;


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

    //--------------------------------------------------

    class ClusterCorrector : public Subscriber
    {
        public:
            virtual double correct_energy
                ( const fne::RecoCluster& rc, bool is_mc) const = 0 ;
            virtual double correct_energy
                ( double x, double y, double energy,  bool is_mc) const = 0;

            virtual bool is_mc() const  = 0;

            virtual ~ClusterCorrector(){}
        private:
            REG_DEC_SUB( ClusterCorrector );
    };

    template<>
        Subscriber * create_subscriber<ClusterCorrector>
        (YAML::Node& instruct, RecoFactory& rf );

    class DefaultClusterCorrector : public ClusterCorrector
    {
        public:
            DefaultClusterCorrector( std::string filename, const GlobalStatus& gs, bool corr_active );
            virtual bool is_mc() const { return global_status_.is_mc() ; }

            virtual double correct_energy( const fne::RecoCluster& rc, bool is_mc) const;
            virtual double correct_energy
                ( double x, double y, double energy,  bool is_mc) const;

            virtual double correct_energy( const fne::RecoCluster& rc ) const
            { return correct_energy( rc, is_mc() ); }
            virtual double correct_energy ( double x, double y, double energy) const
            { return correct_energy( x, y , energy, is_mc() ) ; }

            virtual ~DefaultClusterCorrector(){}

        private:
            const ClusterEnergyCorr cec_;
            const GlobalStatus& global_status_;
            bool corr_active_;

    };

    ClusterCorrector * get_cluster_corrector
        ( YAML::Node& instruct, RecoFactory& rf );

}
#endif
