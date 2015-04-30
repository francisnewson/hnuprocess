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

namespace fne{ 
    class Event; 
    class RecoMuon;
}
namespace fn
{
    class SingleTrack;

    class SingleMuon : public Subscriber
    {
        public:
            virtual bool found() = 0;
            virtual double weight() = 0;
            virtual double x() = 0;
            virtual double y() = 0;
            virtual ~SingleMuon(){}
    };

    class RawSingleMuon : public SingleMuon
    {
        public:
            RawSingleMuon
                ( const fne::Event * e, const SingleTrack& st);

            virtual void new_event();

            bool found();
            double weight();
            double x();
            double y();

        private:
            const fne::Event * e_;
            const SingleTrack& st_;

            void update() const;

            mutable bool dirty_;
            mutable const fne::RecoMuon * rm_;
            mutable bool found_muon_;
    };
}
#endif

