#include "Km3Reco.hh"
#include "k2pi_reco_functions.hh"
#include "CorrCluster.hh"
#include "NA62Constants.hh"
#include "TLorentzVector.h"
#include "RecoFactory.hh"
#include "tracking_selections.hh"
#include "cluster_selections.hh"
#include "Xcept.hh"

namespace fn
{

    void Km3RecoEvent::update( const fne::Event * e,
            const KaonTrack *kt, 
            const SingleRecoTrack * st,
            const K2piRecoClusters * k2pirc,
            logger * slg, severity_level sl
            )
    {
        k2pirc_ = k2pirc;

        //Compute neutral vertex
        CorrCluster c1
        { k2pirc->cluster1(), k2pirc->get_cluster_corrector(), k2pirc->is_mc() };
        PhotonProjCorrCluster ph1{ c1 };

        CorrCluster c2
        { k2pirc->cluster2(), k2pirc->get_cluster_corrector(), k2pirc->is_mc() };
        PhotonProjCorrCluster ph2{ c2 };

        double neutral_z = 
            bracket_solve_neutral_vertex
            ( kt->get_kaon_track(), c1.get_energy(), c1.get_pos(),
              c2.get_energy(), c2.get_pos(), slg , sl );

        neutral_vertex_ = kt->get_kaon_track().extrapolate( neutral_z );

        //Compute charged vertex
        charged_vertex_ = st->get_vertex();

        //Determine four-momenta
        p4_k_ = kt->get_kaon_4mom();
        p4_mu_ = TLorentzVector{st->get_3mom(), std::hypot( st->get_mom(), na62const::mMu ) };

        TVector3 p1 = c1.get_pos() - neutral_vertex_;
        p4_c1_ = TLorentzVector{ c1.get_energy() * p1.Unit(), c1.get_energy()  };

        TVector3 p2 = c2.get_pos() - neutral_vertex_;
        p4_c2_ = TLorentzVector{ c2.get_energy() * p2.Unit(), c2.get_energy()  };

        p4_miss_ = p4_k_ - p4_mu_ - p4_c1_ - p4_c2_;

        cda_ = st->get_cda();
    }

    const TVector3& Km3RecoEvent::get_neutral_vertex() const
    {
        return neutral_vertex_;
    }

    const TVector3& Km3RecoEvent::get_charged_vertex() const
    {
        return charged_vertex_;
    }

    double Km3RecoEvent::get_m2m_miss() const
    {
        return p4_miss_.M2();
    }

    double Km3RecoEvent::get_eop() const
    {
        if ( k2pirc_->found_track_cluster() )
        {
            CorrCluster ct{ k2pirc_->track_cluster(), 
                k2pirc_->get_cluster_corrector(), k2pirc_->is_mc() };

            return ct.get_energy() / p4_mu_.P();
        }
        else
        {
            return 0;
        }
    }

    double Km3RecoEvent::get_pt_miss() const
    {
        return p4_miss_.Vect().Perp( p4_k_.Vect() );
    }

    double Km3RecoEvent::get_p_miss() const
    {
        return p4_miss_.P();
    }

    double Km3RecoEvent::get_cda() const
    {
        return cda_;
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km3Reco );

    Km3Reco::Km3Reco(const fne::Event * event,
            const SingleTrack& st,
            const KaonTrack& kt,
            const K2piClusters& k2pic,
            bool mc )
        :e_( event ), st_( st ), kt_( kt ), k2pic_( k2pic )
    {}

    void Km3Reco::new_event()
    {
        dirty_ = true;
    }

    bool Km3Reco::found_km3_event() const
    {
        return k2pic_.found_clusters();
    }

    const Km3RecoEvent& Km3Reco::get_reco_event() const
    {
        if ( dirty_ )
        {
            process_event();
            dirty_ = false;
        }

        assert ( !dirty_ );
        return reco_event_;
    }

    void Km3Reco::process_event() const
    {
        srt_ = &st_.get_single_track();
        k2pirc_ = &k2pic_.get_reco_clusters();
        reco_event_.update( e_, &kt_, srt_, k2pirc_, &get_log(), log_level() );
    }

    template<>
        Subscriber * create_subscriber<Km3Reco>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event =  rf.get_event_ptr();
            SingleTrack * st = get_single_track( instruct, rf );
            KaonTrack * kt = get_kaon_track( instruct, rf );
            const K2piClusters * k2pic  = get_k2pi_clusters( instruct,rf );
            bool mc = rf.is_mc();

            return new Km3Reco( event, *st, *kt, *k2pic,  mc  );
        }


    Km3Reco * get_km3_reco
        ( YAML::Node& instruct, RecoFactory& rf, std::string label )
        {
            YAML::Node yogt = instruct["inputs"][label];

            if ( !yogt )
            {throw Xcept<MissingNode>( label );}

            Km3Reco * ogt = dynamic_cast<Km3Reco*>
                ( rf.get_subscriber( yogt.as<std::string>() ));

            if ( !ogt )
            { throw Xcept<BadCast>( "KM3RECO" ); }

            return ogt;
        }
}
