#ifndef K2PI_FITTING_HH
#define K2PI_FITTING_HH
#include "Math/Minimizer.h"
#include "TLorentzVector.h"
#include <array>

#if 0
/*
 *  _    ____        _      __ _ _   _   _
 * | | _|___ \ _ __ (_)    / _(_) |_| |_(_)_ __   __ _
 * | |/ / __) | '_ \| |   | |_| | __| __| | '_ \ / _` |
 * |   < / __/| |_) | |   |  _| | |_| |_| | | | | (_| |
 * |_|\_\_____| .__/|_|___|_| |_|\__|\__|_|_| |_|\__, |
 *            |_|    |_____|                     |___/
 *
 */
#endif
namespace fn
{
    class K2piLkrData;
    class ClusterCorrector;

    struct k2pi_params
    {
        std::array<double, 11> par_;

        double&  E1()  { return par_[0] ;} 
        double&  E2()  { return par_[1] ;}

        double&  posC1_X()  { return par_[2] ;}
        double&  posC2_X()  { return par_[3] ;}

        double&  posC1_Y()  { return par_[4] ;}
        double&  posC2_Y()  { return par_[5] ;}

        double&  pK_X()  { return par_[6] ;}
        double&  pK_Y()  { return par_[7] ;}
        double&  pK_Z()  { return par_[8] ;}

        double&  pos0K_X()  { return par_[9] ;}
        double&  pos0K_Y()  { return par_[10];}

    };

    struct k2pi_fit
    {
        TLorentzVector pi0;
        TLorentzVector p1;
        TLorentzVector p2;
        TLorentzVector pip;
        TVector3 neutral_vertex;
        double chi2;
    };

    class FNK2piFit
    {
        public:
            FNK2piFit ( const ClusterCorrector& cluster_corrector, bool mc );
            void load_raw_data( const K2piLkrData& measured );
            void prepare_errors();
            double operator()( const double * fit_params );
            double compute_chi2(double pion_mass );
            void set_variables(  ROOT::Math::Minimizer * minimizer );
            k2pi_params fit_;


            k2pi_params  measured_;
            k2pi_params  errors_;
            k2pi_fit result_;

        private:
            const ClusterCorrector& cluster_corrector_;
            bool mc_;
    };

    double compute_pion_mass( k2pi_params& fit, k2pi_fit& result );
}
#endif
