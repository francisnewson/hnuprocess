#ifndef K2PI_FITTING_HH
#define K2PI_FITTING_HH

#include "K2piFitReco.hh"

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
    class FNK2piFit
    {
        public:
            void load_raw_data( const K2piLkrData& measured );
            void prepare_errors();
            double operator()( const double * fit_params );
            double compute_chi2(double pion_mass );
            double compute_pion_mass( k2pi_params& fit );
            void set_variables(  ROOT::Math::Minimizer * minimizer );
            k2pi_params fit_;


            k2pi_params  measured_;
            k2pi_params  errors_;
            k2pi_fit result_;

        private:
    };

}
#endif
