#ifndef K2PIRECO_HH
#define K2PIRECO_HH

#include "Subscriber.hh"
#include "K2piClusters.hh"
#include "Event.hh"
#include "KaonTrack.hh"
#include "TLorentzVector.h"
#include "CorrCluster.hh"

#if 0
/*
 *  _  ______        _ ____   
 * | |/ /___ \ _ __ (_)  _ \ ___  ___ ___  
 * | ' /  __) | '_ \| | |_) / _ \/ __/ _ \ 
 * | . \ / __/| |_) | |  _ <  __/ (_| (_) |
 * |_|\_\_____| .__/|_|_| \_\___|\___\___/ 
 *            |_|                          
 * 
 */
#endif
namespace fn
{
    class K2piRecoEvent
    {
        public:
            virtual void update( 
                    const fne::Event * event,
                    const KaonTrack& kt,
                    const SingleTrack& st,
                    const K2piClusters& k2pic ) = 0;

            virtual double get_zvertex() const = 0;

            virtual double get_m2pip() const = 0;
            virtual TLorentzVector get_p4pip() const = 0;

            virtual double get_m2pi0() const = 0;
            virtual TLorentzVector get_p4pi0() const = 0;

            virtual TLorentzVector get_p4g1() const = 0;
            virtual TLorentzVector get_p4g2() const = 0;

            virtual const ClusterData get_cluster1() const = 0;
            virtual const ClusterData get_cluster2()  const = 0;

            virtual double get_chi2() const = 0;


            void set_log( logger& slg );
            void set_log_level( severity_level sev );

            logger& get_log();
            severity_level log_level();

        private:
            logger * slg_;
            severity_level sl_;
    };

    class K2piReco : public Subscriber
    {
        public:
            K2piReco( K2piRecoEvent* k2pirc );
            void new_event();
            const K2piRecoEvent& get_reco_event() const;

        protected:
            mutable K2piRecoEvent * reco_event_;

        private:
            virtual void process_event() const = 0;
            mutable bool dirty_;

            REG_DEC_SUB( K2piReco );
    };


    template<>
        Subscriber * create_subscriber<K2piReco>
        (YAML::Node& instruct, RecoFactory& rf );


    //--------------------------------------------------

    class K2piRecoImp : public K2piReco
    {
        public:
            K2piRecoImp( 
                    const fne::Event * event,
                    const SingleTrack& st,
                    const K2piClusters& k2pic,
                    K2piRecoEvent * k2pirec,
                    bool mc );

        private:
            void process_event() const;
            const fne::Event * e_;
            const SingleTrack & st_;
            const K2piClusters & k2pic_;
            const KaonTrack kt_;
            bool mc_;
    };

    //--------------------------------------------------

    class K2piSimpleRecoEvent : public K2piRecoEvent
    {
        public: 
            K2piSimpleRecoEvent();
            void update( 
                    const fne::Event * event,
                    const KaonTrack& kt,
                    const SingleTrack& st,
                    const K2piClusters& k2pic );
        private: 
            double get_zvertex() const;
            double get_m2pip() const;
            double get_m2pi0() const;

            ClusterData c1_;
            ClusterData c2_;

            TLorentzVector get_p4pip() const;
            TLorentzVector get_p4pi0() const;

            TLorentzVector get_p4g1() const;
            TLorentzVector get_p4g2() const;

            const ClusterData get_cluster1()  const;
            const ClusterData get_cluster2()  const;

            double get_chi2() const;

            TVector3 neutral_vertex_ ;
            TLorentzVector pi0_;
            TLorentzVector pip_lkr_;

            TLorentzVector p1_;
            TLorentzVector p2_;
    };

    //--------------------------------------------------


    K2piReco * get_k2pi_reco
        ( YAML::Node& instruct, RecoFactory& rf );
}
#endif
