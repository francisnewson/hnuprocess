#ifndef KM2EVENT_HH
#define KM2EVENT_HH
#include "Subscriber.hh"
#include "Event.hh"
#include "KaonTrack.hh"
#include "SingleTrack.hh"
#include "TLorentzVector.h"

#if 0
/*
 *  _  __          ____  _____                 _  
 * | |/ /_ __ ___ |___ \| ____|_   _____ _ __ | |_ 
 * | ' /| '_ ` _ \  __) |  _| \ \ / / _ \ '_ \| __|
 * | . \| | | | | |/ __/| |___ \ V /  __/ | | | |_ 
 * |_|\_\_| |_| |_|_____|_____| \_/ \___|_| |_|\__|
 *                                           
 * 
 */
#endif
namespace fn
{
    class Km2RecoEvent
    {
        public: 
            void update( const fne::Event * e, 
                    const KaonTrack * kt, const SingleTrack * st_ );

            double get_m2miss() const ;
            double get_muon_mom() const;
            double get_zvertex() const ;
            double get_cda() const; 

        private:
            const SingleRecoTrack * srt_;
            const fne::Event * e_;
            const KaonTrack * kt_;

            TLorentzVector p4miss_;
            double m2miss_;
    };

    //--------------------------------------------------

    class Km2Event : public Subscriber
    {
        public:
            Km2Event( const fne::Event * event, 
                    const SingleTrack& st,
                    bool mc );

            void new_event();
            const Km2RecoEvent& get_reco_event() const;

        private:
            void process_event() const ;
            const fne::Event * e_;
            const SingleTrack& st_;
            KaonTrack kt_;

            mutable Km2RecoEvent reco_event_;
            mutable bool dirty_;

            REG_DEC_SUB( Km2Event );
    };

    template<>
        Subscriber * create_subscriber<Km2Event>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    Km2Event * get_km2_event
        ( YAML::Node& instruct, RecoFactory& rf );

}
#endif
