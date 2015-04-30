#ifndef SINGLEMUON_HH
#define SINGLEMUON_HH
#include "Subscriber.hh"
#include "MuonScatterSim.hh"
#include "PolarityFinder.hh"
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
            virtual bool found() const = 0;
            virtual double weight() const = 0;
            virtual double x() const = 0;
            virtual double y() const = 0;
            virtual ~SingleMuon(){}

            REG_DEC_SUB( SingleMuon );
    };

    template<>
        Subscriber * create_subscriber<SingleMuon>
        (YAML::Node& instruct, RecoFactory& rf );

    SingleMuon * get_single_muon( YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    //Uses raw RecoMuons from event.detector.muons
    class RawSingleMuon : public SingleMuon
    {
        public:
            RawSingleMuon
                ( const fne::Event * e, const SingleTrack& st);

            virtual void new_event();

            bool found() const;
            double weight() const;
            double x() const;
            double y() const;

        private:
            const fne::Event * e_;
            const SingleTrack& st_;

            void update() const;

            mutable bool dirty_;
            mutable const fne::RecoMuon * rm_;
            mutable bool found_muon_;
    };

    //--------------------------------------------------

    //Uses MC muon particle and simulates scattering
    class MCScatterSingleMuon : public SingleMuon
    {
        public:
            MCScatterSingleMuon
                ( const fne::Event * e, const SingleTrack& st);

            virtual void new_event();
            virtual void new_run();

            bool found() const;
            double weight() const;
            double x() const;
            double y() const;

        private:
            const fne::Event * e_;
            const SingleTrack& st_;

            void update() const;

            mutable bool dirty_;
            mutable bool found_muon_;
            mutable double x_;
            mutable double y_;

            MuonScatterSim mss_;
            PolarityFinder pf_;
    };
}
#endif

