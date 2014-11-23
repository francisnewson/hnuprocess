#include "k2pi_fitting.hh"
#include "K2piInterface.hh"
#include "K2piEventData.hh"
#include "TVector3.h"
#include "k2pi_functions.hh"
#include "Xcept.hh"
#include "NA62Constants.hh"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include <cmath>
#include <algorithm>
#include <iomanip>

namespace fn
{
    void FNK2piFit::load_raw_data( const K2piLkrData& measured )
    {
        K2piLkrInterface measured_interface( const_cast<K2piLkrData&>(measured) );
        copy( measured_interface, measured_ );
    }

    void FNK2piFit::prepare_errors()
    {
        //ERRORS
        //lkr resolutions
        double pos_res1 = lkr_pos_res( measured_.E1() );
        double pos_res2 = lkr_pos_res( measured_.E1() );

        errors_.E1() =  lkr_energy_res( measured_.E1() );
        errors_.E2() =  lkr_energy_res( measured_.E2() );

        errors_.posC1_X() = pos_res1;
        errors_.posC2_X() = pos_res2;

        errors_.posC1_Y() = pos_res1;
        errors_.posC2_Y() = pos_res2;

        //beam resolutions
        errors_.pK_X() = 1e-3;
        errors_.pK_Y() = 1e-3;
        errors_.pK_Z() = 1.5;

        errors_.pos0K_X() = 0.3;
        errors_.pos0K_Y() = 0.3;
    }

    double FNK2piFit::operator()( const double * fit )
    {
        std::copy( &fit[0], &fit[11], begin( fit_.par_ ) );
        double pion_mass = compute_pion_mass( fit_, result_ );
        double chi2 = compute_chi2( pion_mass );
        return chi2;
    }

    double compute_pion_mass( k2pi_params& fit, k2pi_fit& result )
    {
        //find neutral vertex
        TVector3 kaon_point = TVector3{ fit.pos0K_X(), fit.pos0K_Y(), 0};
        TVector3 kaon_3mom = TVector3{ fit.pK_X(), fit.pK_Y(), fit.pK_Z() };

        Track kt{ kaon_point , kaon_3mom };

        TVector3 pos1{ fit.posC1_X(), fit.posC1_Y(), 
            na62const::zLkr + 20.8 + 4.3 * std::log( fit.E1() ) };

        TVector3 pos2{ fit.posC2_X(), fit.posC2_Y(), 
            na62const::zLkr + 20.8 + 4.3 * std::log( fit.E2() ) };

        double neutral_z = bracket_solve_neutral_vertex
            ( kt, fit.E1(),  pos1, fit.E2(), pos2 );

        result.neutral_vertex = kt.extrapolate( neutral_z );

        //compute photon momenta
        TVector3 v1 = pos1 - result.neutral_vertex;
        result.p1 = TLorentzVector{ fit.E1() * v1.Unit(), fit.E1() };

        TVector3 v2 = pos2 - result.neutral_vertex;
        result.p2 = TLorentzVector{ fit.E2() * v2.Unit(), fit.E2() };

        //Compute pion momenta
        result.pi0 = result.p1 + result.p2;

        double kaon_energy = std::sqrt( kaon_3mom.Mag2() + na62const::mK2 );

        TLorentzVector k{ kaon_3mom , kaon_energy };

        result.pip = k - result.pi0;

        //compute pion mass
        return result.pip.M();
    }

    double FNK2piFit::compute_chi2(double pion_mass )
    {
        double result = 0;

        //loop over fit parameters
        for ( int i = 0 ; i != 11 ; ++ i )
        {
            result += std::pow(
                    (fit_.par_[i] - measured_.par_[i]) / errors_.par_[i] , 2 );
        }

        //Include pi+ constraint
        result += std::pow( 10000*( pion_mass - na62const::mPi), 2 );
        return result;
    }

    void FNK2piFit::set_variables(  ROOT::Math::Minimizer * minimizer )
    {
        for ( unsigned int i = 0 ; i != 11 ; ++i )
        {
            double min_val = measured_.par_[i] - errors_.par_[i];
            double max_val = measured_.par_[i] + errors_.par_[i];

            if ( i == 0 || i == 1 )
            {
                min_val = std::max( min_val, 0.0 );
            }

            minimizer->SetLimitedVariable
                ( i, std::to_string( i ), measured_.par_[i], 0.1*errors_.par_[i],
                  min_val, max_val );
        }
    }

}
