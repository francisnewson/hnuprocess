#include "k2pi_reco_functions.hh"
#include "Event.hh"

#include "NA62Constants.hh"

#include "K2piEventData.hh"
#include "K2piInterface.hh"
#include "K2piClusters.hh"
#include "k2pi_fitting.hh"

#include "ClusterEnergyCorr.hh"
#include "CorrCluster.hh"

#include "Event.hh"
#include "RecoCluster.hh"
#include "KaonTrack.hh"
#include "Track.hh"

#include "root_help.hh"
#include <boost/math/tools/roots.hpp>
#include <boost/lexical_cast.hpp>

namespace fn
{
    double extract_eop( K2piEventData& event, K2piDchData& dch, bool mc )
    {
        fne::RecoCluster rc;
        rc.energy = event.TCE;
        rc.x = event.TCX;
        rc.y = event.TCY;
        CorrCluster cc{ calibrated_cluster_data{ event.TCE, { event.TCX, event.TCY, na62const::zLkr } }, mc };

        double energy = cc.get_energy();
        double momentum = dch.p;
        double eop = energy / momentum;

        return eop;
    }


    double extract_photon_sep ( K2piLkrData &raw_lkr )
    {
        K2piLkrInterface lkr( raw_lkr);
        TVector3 vec_sep = TVector3( lkr.posC1_X(), lkr.posC1_Y(), 0 ) 
            - TVector3( lkr.posC2_X(), lkr.posC2_Y() );
        double sep = vec_sep.Mag();
        return sep ;
    };

    double extract_min_track_cluster_sep ( K2piLkrData &raw_lkr, K2piDchData& raw_dch )
    {
        K2piLkrInterface lkr( raw_lkr);
        K2piDchInterface dch( raw_dch);
        Track dch_track( dch.x0(), dch.y0(), na62const::bz_tracking,
                dch.dxdz(), dch.dydz(), 1 );
        TVector3 track_pos = dch_track.extrapolate( na62const::zLkr );

        TVector3 sep1 = track_pos - 
            TVector3( lkr.posC1_X(), lkr.posC1_Y(), na62const::zLkr );

        TVector3 sep2 = track_pos - 
            TVector3( lkr.posC2_X(), lkr.posC2_Y(), na62const::zLkr );

        return std::min( sep1.Mag(), sep2.Mag() );
    }

    double extract_min_photon_radius( K2piLkrData& raw_lkr )
    {
        //Copy data into parameter array
        k2pi_params lkr_param_data;
        K2piLkrInterface lkr_interface( raw_lkr);
        copy( lkr_interface, lkr_param_data );

        //Extract 'fit' result
        k2pi_fit data_fit;
        compute_pion_mass( lkr_param_data, data_fit );

        Track track1( data_fit.neutral_vertex, data_fit.p1.Vect() );
        TVector3 pos1 = track1.extrapolate( na62const::zDch1 );

        Track track2 ( data_fit.neutral_vertex, data_fit.p2.Vect() );
        TVector3 pos2 = track2.extrapolate( na62const::zDch1 );

        return std::min( pos1.Perp(), pos2.Perp() );
    }

    double extract_cda( const K2piDchData& dch, const K2piLkrData& lkr )
    {
        Track kaon{ lkr.kx0, lkr.ky0, 0, lkr.pkx, lkr.pky, lkr.pkz };
        Track pion{ dch.x0, dch.y0, na62const::bz_tracking, dch.dxdz, dch.dydz, 1 };
        return compute_cda( kaon, pion ).cda;
    }


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

        std::cout << "CNV: " << pos1.X() << " " << pos1.Y() << " " << pos1.Z() << " "  << E1 << " "
            << pos2.X() << " " << pos2.Y() << " " << pos2.Z() <<  " " << E2  << std::endl;

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
#if 0
        std::cout << "BSNV: " << pos1.X() << " " << pos1.Y() << " " << pos1.Z() << " "  << E1 << " "
            << pos2.X() << " " << pos2.Y() << " " << pos2.Z() <<  " " << E2  << std::endl;
#endif

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


    k2pi_mc_parts extract_k2pi_particles( const fne::Event * e )
    {
        const TClonesArray& tca =  e->mc.particles;
        int nparticles = e->mc.npart;

        k2pi_mc_parts result;

        for ( int ip = 0; ip != nparticles ; ++ip )
        {
            const fne::McParticle* particle = 
                static_cast<const fne::McParticle*>( tca[ip]  );

            if ( particle->type == 16 )
            { result.photons.push_back( particle ); }

            if ( particle->type == 4 )
            { result.pi0 = particle; }

            if ( particle->type == 8 )
            { result.pip = particle; }

            if ( particle->type == 512 )
            { result.k = particle; }

        }

        std::sort( begin( result.photons), end( result.photons),
                [] ( const fne::McParticle * p,
                    const fne::McParticle * q ) 
                {
                return (p->momentum.E() < q->momentum.E() );
                } );

        return result;
    }

}
