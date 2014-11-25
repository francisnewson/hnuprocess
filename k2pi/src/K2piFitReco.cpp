#include "K2piFitReco.hh"
#include "k2pi_functions.hh"
#include "Xcept.hh"
#include "NA62Constants.hh"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "CorrCluster.hh"
#include <cmath>
#include <algorithm>
#include <iomanip>

namespace fn
{
    
    k2pi_fitter::k2pi_fitter (
            const fne::Event * event, const KaonTrack& kt,
            const SingleRecoTrack& srt, const K2piRecoClusters& k2pirc, bool mc )
        :e_( event), kt_( kt ), srt_( srt ), k2pirc_( k2pirc ), mc_( mc ),
        names_{ "E1", "E2", "C1_X", "C2_X", "C1_Y", "C2_Y",
            "PK_X", "PK_Y", "PK_Z", "posK_X", "posK_Y" }
    {

    }

    void k2pi_fitter::init()
    {
        //compute measured params and errors

        //PARAMS
        //Extract cluster 1
        const fne::RecoCluster& cluster1 = k2pirc_.cluster1();
        PhotonProjCorrCluster corr_cluster1{ cluster1, mc_ };
        measured_.E1() = corr_cluster1.get_energy();
        TVector3 cluster1_pos = corr_cluster1.get_pos();
        measured_.posC1_X() = cluster1_pos.X();
        measured_.posC1_Y() = cluster1_pos.Y();

        //Extract cluster 2
        const fne::RecoCluster& cluster2 = k2pirc_.cluster2();
        PhotonProjCorrCluster corr_cluster2{ cluster2, mc_ };
        measured_.E2() = corr_cluster2.get_energy();
        TVector3 cluster2_pos = corr_cluster2.get_pos();
        measured_.posC2_X() = cluster2_pos.X();
        measured_.posC2_Y() = cluster2_pos.Y();

        //Extract kaon momentum
        TVector3 kaon_3mom = kt_.get_kaon_3mom();
        measured_.pK_X() = kaon_3mom.X();
        measured_.pK_Y() = kaon_3mom.Y();
        measured_.pK_Z() = kaon_3mom.Z();

        //Extract kaon position
        TVector3 kaon_point = kt_.get_kaon_point();
        measured_.pos0K_X() = kaon_point.X();
        measured_.pos0K_Y() = kaon_point.Y();

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

    void k2pi_fitter::set_variables(  ROOT::Math::Minimizer * minimizer )
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

    double k2pi_fitter::operator()( const double * fit_params )
    {
        std::copy( &fit_params[0], &fit_params[11], begin( fit_.par_ ) );

        //find neutral vertex
        TVector3 kaon_point = TVector3{ fit_.pos0K_X(), fit_.pos0K_Y(), 0};
        TVector3 kaon_3mom = TVector3{ fit_.pK_X(), fit_.pK_Y(), fit_.pK_Z() };

        Track kt{ kaon_point , kaon_3mom };

        TVector3 pos1{ fit_.posC1_X(), fit_.posC1_Y(), 
            na62const::zLkr + 20.8 + 4.3 * std::log( fit_.E1() ) };

        TVector3 pos2{ fit_.posC2_X(), fit_.posC2_Y(), 
            na62const::zLkr + 20.8 + 4.3 * std::log( fit_.E2() ) };

        double neutral_z = bracket_solve_neutral_vertex
            ( kt, fit_.E1(),  pos1, fit_.E2(), pos2 );

        result_.neutral_vertex = kt.extrapolate( neutral_z );

        //compute photon momenta
        TVector3 v1 = pos1 - result_.neutral_vertex;
        result_.p1 = TLorentzVector{ fit_.E1() * v1.Unit(), fit_.E1() };

        TVector3 v2 = pos2 - result_.neutral_vertex;
        result_.p2 = TLorentzVector{ fit_.E2() * v2.Unit(), fit_.E2() };

        //Compute pion momenta
        result_.pi0 = result_.p1 + result_.p2;

        double kaon_energy = std::sqrt( kaon_3mom.Mag2() + na62const::mK2 );

        TLorentzVector k{ kaon_3mom , kaon_energy };

        result_.pip = k - result_.pi0;

        //compute pion mass
        double pion_mass = result_.pip.M();

        result_.chi2 = chi2(  pion_mass );
        return result_.chi2;
    }

    double k2pi_fitter::chi2(double pion_mass )
    {
        double result = 0;

        //loop over fit parameters
        for ( int i = 0 ; i != 11 ; ++ i )
        {
#if 0
            std::cerr
                << " n: " << std::setw( 12 ) << names_[i]
                << " f: " << std::setw( 12 ) << fit_.par_[i] 
                << " m: " << std::setw( 12 ) << measured_.par_[i]
                << std::endl;
#endif

            result += std::pow(
                    (fit_.par_[i] - measured_.par_[i]) / errors_.par_[i] , 2 );
        }

        //Include pi+ constraint
        result += std::pow( 10000*( pion_mass - na62const::mPi), 2 );

#if 0
        std::cerr
            << " n: " << std::setw( 12 ) << "mPi"
            << " f: " << std::setw( 12 ) <<  pion_mass
            << " m: " << std::setw( 12 ) << na62const::mPi
            << std::endl;

        std::cerr << "Chi2: " << result << "\n" << std::endl;
#endif

        return result;
    }

    //--------------------------------------------------

    K2piFitRecoEvent::K2piFitRecoEvent(){}

    void K2piFitRecoEvent::update( 
            const fne::Event * event,
            const KaonTrack& kt,
            const SingleTrack& st,
            const K2piClusters& k2pic,bool mc )
    {

        //create fit object
        k2pi_fitter fit_object( event, kt, st.get_single_track(),
                k2pic.get_reco_clusters(), mc );

        fit_object.init();

        ROOT::Math::Functor fit_functor( fit_object, 11 );
        auto minimizer = 
            ROOT::Math::Factory::CreateMinimizer( "Minuit", "MIGRAD" );

        minimizer->SetMaxFunctionCalls( 1000 );
        minimizer->SetMaxIterations( 100 );
        minimizer->SetTolerance( 0.01 );
        minimizer->SetFunction( fit_functor );

        fit_object.set_variables( minimizer );
        minimizer->Minimize();

        fit_object( minimizer->X() );
        fit_result_ = fit_object.result();

        BOOST_LOG_SEV( get_log(), log_level() )
            << "DONE: " << fit_object.result().chi2 << " " << fit_result_.chi2;

        //Save cluster information
        const K2piRecoClusters& k2pirc = k2pic.get_reco_clusters();

        //Extract photon clusters
        PhotonProjCorrCluster c1 {k2pirc.cluster1(), mc };
        PhotonProjCorrCluster c2 {k2pirc.cluster2(), mc };

        c1_.update( c1 );
        c2_.update( c2 );

        //Extract track cluster
        found_track_cluster_ = k2pirc.found_track_cluster();
        if ( found_track_cluster_)
        {
            TrackProjCorrCluster tc { k2pirc.track_cluster(), mc };
            tc_.update( tc );
        }
    }


    double K2piFitRecoEvent::get_zvertex() const
    { return fit_result_.neutral_vertex.Z(); }

    TVector3 K2piFitRecoEvent::get_vertex() const 
    { return fit_result_.neutral_vertex ; }

    double K2piFitRecoEvent::get_m2pip() const
    { return fit_result_.pip.M2(); }

    TLorentzVector K2piFitRecoEvent::get_p4pip() const
    { return fit_result_.pip; }

    double K2piFitRecoEvent::get_m2pi0() const
    { return fit_result_.pi0.M2(); }

    TLorentzVector K2piFitRecoEvent::get_p4pi0() const
    { return fit_result_.pi0; }

    TLorentzVector K2piFitRecoEvent::get_p4g1() const
    { return fit_result_.p1; }

    TLorentzVector K2piFitRecoEvent::get_p4g2() const
    { return fit_result_.p2; }

    const ClusterData K2piFitRecoEvent::get_cluster1()  const
    { return c1_; }

    const ClusterData K2piFitRecoEvent::get_cluster2()  const
    { return c2_; }

    double K2piFitRecoEvent::get_chi2() const
    { return fit_result_.chi2; }

    bool K2piFitRecoEvent::found_track_cluster() const
    { return found_track_cluster_; }

    const ClusterData K2piFitRecoEvent::get_track_cluster() const
    { 
        if ( ! found_track_cluster_ )
        { throw Xcept<EventDoesNotContain>( "track_cluster " __FILE__);}
        return tc_;
    }
}
