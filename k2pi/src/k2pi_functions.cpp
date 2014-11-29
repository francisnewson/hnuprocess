#include "k2pi_functions.hh"
#include "Event.hh"
#include "KaonTrack.hh"
#include "K2piClusters.hh"
#include "CorrCluster.hh"
#include "NA62Constants.hh"
#include "root_help.hh"
#include <boost/math/tools/roots.hpp>
#include <boost/lexical_cast.hpp>

namespace fn
{

    TVector3 compute_neutral_vertex( const fne::Event * e,
            const KaonTrack& kt, const K2piRecoClusters & k2pirc ,
            logger * slg , severity_level sl )
    {
        //sl = always_print;
        if ( slg ){ BOOST_LOG_SEV( *slg, sl )
            << "NEUT VERT: Extracting clusters"; }

        CorrCluster c1( k2pirc.cluster1(), k2pirc.get_cluster_corrector(), k2pirc.is_mc() );
        CorrCluster c2( k2pirc.cluster2(), k2pirc.get_cluster_corrector(), k2pirc.is_mc() );

        PhotonProjCorrCluster p1{c1};
        PhotonProjCorrCluster p2{c2};

        //Extract photon positions
        TVector3 pos1 = p1.get_pos();
        TVector3 pos2 = p2.get_pos();

        //Exract photon energies
        double E1 = p1.get_energy();
        double E2 = p2.get_energy();

        //minimize
        if ( slg ){ BOOST_LOG_SEV( *slg, sl )
            << "NEUT VERT: Minimizing"; }

        if ( slg ){
            std::stringstream ss; ss << pos1;
            BOOST_LOG_SEV( *slg, sl )
                << "C1: " << ss.str() << " E1: " << E1 ; 

            ss.str(""); ss << pos2; 
            BOOST_LOG_SEV( *slg, sl )
                << "C2: " << ss.str() << " E2: " << E2 ; 
        }

        if ( slg && e->header.mc ){ BOOST_LOG_SEV( *slg, sl )
            << "NEUT VERT: true z " << e->mc.decay.decay_vertex.Z() ; }

        double best_z = 0;

        Track kaon_track{ kt.get_kaon_point(), kt.get_kaon_3mom() };

        //Do the numerical bit
        best_z = bracket_solve_neutral_vertex
            ( kaon_track, E1, pos1, E2, pos2, slg, sl );

        return kt.extrapolate_z( best_z );
    }

    //--------------------------------------------------

    double bracket_solve_neutral_vertex
        (
         const Track& kt,
         double E1, const TVector3& pos1,
         double E2, const TVector3& pos2,
         logger * slg , severity_level sl )
        {

            //what should cos_t be?
            double req_cos_t = pi0_cos_photon_opening( E1, E2 );

            if( slg ) BOOST_LOG_SEV( *slg, sl )
                << "target cos_t: " << req_cos_t; 

            //Functino to compare each time
            auto delta_to_minimize = [ &kt, &pos1, &pos2, &req_cos_t] ( double z )
            {
                //Check z is sensible
                if ( z > 15000 ) 
                { throw std::domain_error
                    ( "z: " + std::to_string( z ) ); }

                TVector3 new_vertex = kt.extrapolate( z );
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
            catch ( std::exception & e  )
            {
                std::cerr << "In fit_z_bracket_solve: " << std::endl;
                std::cerr << e.what() << std::endl;
                fit_best_vertex = std::make_pair( raw_vertex_z, 0 );

                if ( slg ){
                    BOOST_LOG_SEV( *slg, sl )
                        << "E1:" << E1 << " E2: " << E2 ;

                    BOOST_LOG_SEV( *slg, sl )
                        << " pos1: " << pos1.X() << " pos2: " << pos2.X();

                    BOOST_LOG_SEV( *slg, sl )
                        << "zrange: " << za << " - " << zb 
                        << " deltarange: " << fa << " - " << fb 
                        ;
                }
                std::cerr << "Error in compute_neutral_vertex\n";
            }

            if ( slg )BOOST_LOG_SEV( *slg, sl )
                << "raw: " << raw_vertex_z 
                    << " fit: " << fit_best_vertex.first
                    << " " << fit_best_vertex.second ;

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

    //--------------------------------------------------

    double  lkr_energy_res( double energy)
    {
        const double& ce = energy;
        return std::sqrt(
                std::pow( 0.032 * std::sqrt( ce ), 2 )
                + std::pow( 0.09, 2 )
                + std::pow( 0.0042 * ce, 2 ) );
    }

    double  lkr_pos_res( double energy)
    {
        const double& ce = energy;
        return std::sqrt(
                std::pow( 0.42 / std::sqrt(ce),  2 )
                + std::pow( 0.06 * ce, 2 )
                );
    }

}
