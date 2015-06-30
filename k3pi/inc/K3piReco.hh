#ifndef K3PIRECO_HH
#define K3PIRECO_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "TLorentzVector.h"
#include "GetSubscriber.hh"

#if 0
/*
 *  _  _______       _ ____
 * | |/ /___ / _ __ (_)  _ \ ___  ___ ___
 * | ' /  |_ \| '_ \| | |_) / _ \/ __/ _ \
 * | . \ ___) | |_) | |  _ <  __/ (_| (_) |
 * |_|\_\____/| .__/|_|_| \_\___|\___\___/
 *            |_|
 *
 */
#endif

class TFile;

namespace fne
{
    class Event;
    class RecoTrack;
}

namespace fn
{
    class KaonTrack;
    class ClusterCorrector;

    class K3piRecoEvent
    {
        public:
            int get_charge() const;
            double get_chi2() const;
            double get_min_dch1_sep() const;
            double get_max_dy_dch4_sep() const;
            double get_max_eop() const;
            double get_pt() const;
            double get_mom() const;
            double get_invariant_mass2() const;
            double get_z_vertex() const;

            void update( int charge, double chi2,
                    double min_dch1_sep, 
                    double max_dy_dch4, double max_eop, 
                    TLorentzVector p4_total,
                    TVector3 vertex, TVector3 kaon_mom);

        private:
            int charge_;
            double chi2_;
            double min_dch1_sep_;
            double max_dy_dch4_;
            double max_eop_;
            TLorentzVector p4_total_;
            TVector3 vertex_;
            TVector3 kaon_mom_;
    };

    class K3piReco : public Subscriber
    {
        public:
            K3piReco( const fne::Event * e, int charge,
                    const KaonTrack & kt, const ClusterCorrector& cc );
            void end_processing();

            void new_event();
            bool reconstructed_k3pi() const;

            const K3piRecoEvent& get_k3pi_reco_event() const;

        private:
                bool process_event() const;
                const fne::Event *  e_;

                const KaonTrack& kt_;

                const ClusterCorrector& cc_;

                int req_charge_;


                mutable bool dirty_;
                mutable bool found_;

                mutable K3piRecoEvent re_;

                REG_DEC_SUB( K3piReco );
    };
    double y_shift_dch4( fne::RecoTrack& rt );

    template<>
        Subscriber * create_subscriber<K3piReco>
        (YAML::Node& instruct, RecoFactory& rf );

    template  <>
        K3piReco * get_sub<K3piReco>( YAML::Node& instruct, RecoFactory& rf);

}
#endif

