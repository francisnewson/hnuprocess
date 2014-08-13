#include "K2piReco.hh"
#include <cassert>
#include "Xcept.hh"
#include "SingleTrack.hh"
#include "CorrCluster.hh"
#include "tracking_selections.hh"
#include "cluster_selections.hh"
#include "k2pi_functions.hh"

namespace fn
{
    void K2piRecoEvent::set_log( logger& slg )
    { slg_ = &slg; }

    void K2piRecoEvent::set_log_level( severity_level sev )
    { sl_ = sev; }

    logger& K2piRecoEvent::get_log()
    { return *slg_; }

    severity_level K2piRecoEvent::log_level()
    { return sl_; }

    void K2piReco::new_event()
    {
        reco_event_->set_log( get_log() );
        reco_event_->set_log_level( log_level() );
        dirty_ = true;
    }

    K2piReco::K2piReco(  K2piRecoEvent* k2pirc )
        :reco_event_( k2pirc )
    {
        reco_event_->set_log( get_log() );
        reco_event_->set_log_level( log_level() );
    }

    //--------------------------------------------------

    REG_DEF_SUB( K2piReco );

    const K2piRecoEvent& K2piReco::get_reco_event() const
    {
        if ( dirty_ )
        {
            process_event();
            dirty_ = false;
        }

        assert(!dirty_);
        return *reco_event_;
    }

    template<>
        Subscriber * create_subscriber<K2piReco>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack* st = get_single_track( instruct, rf );
            const K2piClusters* k2pic = get_k2pi_clusters( instruct, rf );
            std::string method = instruct["method"].as<std::string>();
            bool mc = rf.is_mc();

            K2piRecoEvent * re;

            if ( method == "simple" )
            {
                re = new K2piSimpleRecoEvent;
            }
            else 
            {
                throw Xcept<UnknownK2piRecoMethod>( method );
            }

            return new K2piRecoImp( 
                    event, *st, *k2pic, re, mc );
        }

    //--------------------------------------------------

    K2piRecoImp::K2piRecoImp( const fne::Event * event,
            const SingleTrack& st,
            const K2piClusters& k2pic,
            K2piRecoEvent* k2pirec,
            bool mc )
        :K2piReco( k2pirec),
        e_( event), st_( st), k2pic_( k2pic),
        kt_( event, mc), mc_( mc)
    { }

    void K2piRecoImp::process_event() const
    {
        reco_event_->update( e_, kt_, st_, k2pic_ );
    }

    //--------------------------------------------------

    K2piSimpleRecoEvent::K2piSimpleRecoEvent()
    {}

    void K2piSimpleRecoEvent::update( 
            const fne::Event * event,
            const KaonTrack& kt,
            const SingleTrack& st,
            const K2piClusters& k2pic )
    {
        const K2piRecoClusters& k2pirc= k2pic.get_reco_clusters();

        //compute neutral vertex
        neutral_vertex_ =  compute_neutral_vertex
            ( event, kt, k2pic.get_reco_clusters(),
              &get_log(), log_level() );

        //compute momenta
        PhotonProjCorrCluster c1 {k2pirc.cluster1() };
        PhotonProjCorrCluster c2 {k2pirc.cluster2() };

        TVector3 v1 = c1.get_pos() - neutral_vertex_;
        TLorentzVector p1{ c1.get_energy()* v1.Unit(), c1.get_energy() };

        TVector3 v2 = c2.get_pos() - neutral_vertex_;
        TLorentzVector p2{ c2.get_energy()* v2.Unit(), c2.get_energy() };

        BOOST_LOG_SEV( get_log(), log_level() )
            << "RECO: photons: = " << p1.E() <<  " " << p2.E() ;

        pi0_ = p1 + p2;

        BOOST_LOG_SEV( get_log(), log_level() )
            << "RECO: Epi0 = " << pi0_.E();

        TLorentzVector k  = kt.get_kaon_4mom();

        pip_lkr_ = k - pi0_;
    }

    double K2piSimpleRecoEvent::get_zvertex() const
    {
        return neutral_vertex_.Z();
    }

    double K2piSimpleRecoEvent::get_m2pip() const
    {
        return pip_lkr_.M2();
    }

    double K2piSimpleRecoEvent::get_m2pi0() const
    {
        return pi0_.M2();
    }

    double K2piSimpleRecoEvent::get_chi2() const
    { return 0; }

    //--------------------------------------------------

    K2piReco * get_k2pi_reco
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            YAML::Node yk2pi = instruct["inputs"]["k2pi_reco"];

            if ( !yk2pi )
            {
                throw Xcept<MissingNode>( "k2pi_reco");
            }

            K2piReco * k2pir = dynamic_cast<K2piReco*>
                ( rf.get_subscriber( yk2pi.as<std::string>() ) );

            if ( !k2pir )
            { throw Xcept<BadCast>( "K2PIRECO" ); }

            return k2pir;
        }

}
