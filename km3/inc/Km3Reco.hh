#ifndef KM3RECO_HH
#define KM3RECO_HH
#if 0
/*
 *  _  __          _____ ____
 * | |/ /_ __ ___ |___ /|  _ \ ___  ___ ___
 * | ' /| '_ ` _ \  |_ \| |_) / _ \/ __/ _ \
 * | . \| | | | | |___) |  _ <  __/ (_| (_) |
 * |_|\_\_| |_| |_|____/|_| \_\___|\___\___/
 *
 *
 */
#endif
#include "SingleTrack.hh"
#include "K2piClusters.hh"
#include "Event.hh"
#include "Analysis.hh"

namespace fn
{
    class Km3RecoEvent
    {
        public:
            void update( const fne::Event * e,
                    const KaonTrack *kt, 
                    const SingleRecoTrack * srt,
                    const K2piRecoClusters * k2pirc,
                    logger * slg, severity_level sl
                    );


            const TVector3& get_neutral_vertex() const;
            const TVector3& get_charged_vertex() const;
            double get_m2m_miss() const;
            double get_eop() const;
            double get_pt_miss() const;
            double get_p_miss() const;
            double get_cda() const;

        private:
            TVector3 charged_vertex_;
            TVector3 neutral_vertex_;

            TLorentzVector p4_k_;
            TLorentzVector p4_mu_;
            TLorentzVector p4_c1_;
            TLorentzVector p4_c2_;
            TLorentzVector p4_miss_;

            double cda_;

            const K2piRecoClusters * k2pirc_;
    };

    //--------------------------------------------------


    class Km3Reco : public Subscriber
    {
        public:
            Km3Reco(const fne::Event * event,
                    const SingleTrack& st,
                    const KaonTrack& kt,
                    const K2piClusters& k2pic,
                    bool mc );

            void new_event();
            bool found_km3_event() const;
            const Km3RecoEvent& get_reco_event() const;

        private:

            void process_event() const;
            const fne::Event * e_;
            const SingleTrack& st_;
            const KaonTrack& kt_;
            const K2piClusters& k2pic_;

            mutable Km3RecoEvent reco_event_;
            mutable bool dirty_;

            mutable const SingleRecoTrack * srt_;
            mutable const K2piRecoClusters * k2pirc_;

            REG_DEC_SUB( Km3Reco );
    };

    template <>
        Subscriber * create_subscriber<Km3Reco>
        (YAML::Node& instruct, RecoFactory& rf );

    Km3Reco * get_km3_reco
        ( YAML::Node& instruct, RecoFactory& rf, std::string label  = "km3reco");
}
#endif
