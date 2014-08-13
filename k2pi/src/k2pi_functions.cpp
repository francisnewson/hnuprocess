#include "k2pi_functions.hh"
#include "Event.hh"
#include "KaonTrack.hh"
#include "K2piClusters.hh"
#include "CorrCluster.hh"
#include "NA62Constants.hh"
#include <boost/math/tools/roots.hpp>

namespace fn
{

    TVector3 compute_neutral_vertex( const fne::Event * e,
            const KaonTrack& kt, const K2piRecoClusters & k2pirc )
    {
        PhotonProjCorrCluster p1 {k2pirc.cluster1() };
        PhotonProjCorrCluster p2 {k2pirc.cluster2() };

        //Extract photon positions
        TVector3 pos1 = p1.get_pos();
        TVector3 pos2 = p2.get_pos();

        //Exract photon energies
        double E1 = p1.get_energy();
        double E2 = p2.get_energy();

        //minimize
        return {};
    }

    //--------------------------------------------------

    double bracket_solve_neutral_vertex
        (
         const KaonTrack& kt,
         double E1, const TVector3& pos1,
         double E2, const TVector3& pos2
        )
        {

            //what should cos_t be?
            double req_cos_t = pi0_cos_photon_opening( E1, E2 );

            //Functino to compare each time
            auto delta_to_minimize = [ &kt, &pos1, &pos2, &req_cos_t] ( double z )
            {
                //Check z is sensible
                if ( z > 15000 ) 
                { throw std::domain_error
                    ( "z: " + std::to_string( z ) ); }

                TVector3 new_vertex = kt.extrapolate_z( z );
                TVector3 p1 = pos1 - new_vertex;
                TVector3 p2 = pos2 - new_vertex;

                double new_cos_t =  std::cos( p1.Angle( p2) ) ;
                double delta_cos_t = new_cos_t - req_cos_t;

                //Check result is sensible
                if ( fabs( new_cos_t ) > 1 )
                { throw std::domain_error
                    ( "cos_t: " + std::to_string( new_cos_t ) ); }

                return delta_cos_t;
            };

            //Get an initial estimate
            double raw_vertex_z = 
                raw_solve_neutral_vertex( pos1, E1, pos2, E2 );

            //Check if increasing or decreasing
            double za = raw_vertex_z - 100;
            double zb = raw_vertex_z + 100;

            double fa = delta_to_minimize( za );
            double fb = delta_to_minimize( zb );

            //double increasing =  ( ( zb > za ) == ( fb > fa ) );

            //algorithm parameters
            boost::uintmax_t max_it = 100;
            auto termination =  [](double& min, double& max)
            { return fabs( max -  min ) < 3 ; };

            std::pair<double,double> fit_best_vertex ;

            try{
                fit_best_vertex = boost::math::tools::toms748_solve
                    ( delta_to_minimize, za, zb, fa, fb, termination, max_it) ;
            }
            catch ( std::runtime_error& e  )
            {
                std::cerr << "In fit_z_bracket_solve: " << std::endl;
                std::cerr << e.what() << std::endl;
                fit_best_vertex = std::make_pair( raw_vertex_z, 0 );
            }
            return fit_best_vertex.first;
        }

    //--------------------------------------------------

    double pi0_cos_photon_opening( double E1, double E2 )
    {
        return 1 - std::pow( na62const::mPi0, 2 ) 
            / ( 2 * E1 * E2 );
    }

    //--------------------------------------------------

    double raw_solve_neutral_vertex
        ( TVector3 pos1, double E1, TVector3 pos2, double E2 )
        {
            double separation = fabs( ( pos1 - pos2).Perp() );
            return na62const::zLkr 
                - separation * std::sqrt( E1 * E2 ) / na62const::mPi0 ;
        }
}
