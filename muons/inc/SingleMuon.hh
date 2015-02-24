#ifndef SINGLEMUON_HH
#define SINGLEMUON_HH
#include "Subscriber.hh"

#if 0
/*
 *  ____  _             _      __  __
 * / ___|(_)_ __   __ _| | ___|  \/  |_   _  ___  _ __
 * \___ \| | '_ \ / _` | |/ _ \ |\/| | | | |/ _ \| '_ \
 *  ___) | | | | | (_| | |  __/ |  | | |_| | (_) | | | |
 * |____/|_|_| |_|\__, |_|\___|_|  |_|\__,_|\___/|_| |_|
 *                |___/
 *
 */
#endif
namespace fne
{
    class Event;
    class RecoMuon;
}

namespace fn
{
    class SingleTrack;

    class SingleMuon : public Subscriber
    {
        public:
            virtual bool found_muon() const = 0;
            virtual int get_muon_id() const = 0;
            virtual const fne::RecoMuon* get_muon() const = 0;
            virtual void check_consistency() const = 0;

            REG_DEC_SUB( SingleMuon );
    };

    template<>
        Subscriber * create_subscriber<SingleMuon>
        (YAML::Node& instruct, RecoFactory& rf );

    SingleMuon * get_single_muon( YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------


    class TrackSingleMuon : public SingleMuon
    {
        public:
            TrackSingleMuon( const fne::Event * e, const SingleTrack& st );

            //subscriber interface
            virtual void new_event();

            //SingleMuon interface
            bool found_muon() const;
            int get_muon_id() const;
            const fne::RecoMuon* get_muon() const;

            void check_consistency() const;

        private:
            virtual void update() const;

            const fne::Event * e_;
            const SingleTrack& st_;

            mutable const fne::RecoMuon * rm_;
            mutable int muon_id_;
            mutable bool found_muon_;
            mutable bool dirty_;
    };
}
#endif
