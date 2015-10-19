#ifndef SINGLEMUON_HH
#define SINGLEMUON_HH
#include "Subscriber.hh"
#include "MuonScatterSim.hh"
#include "PolarityFinder.hh"
#include  <boost/optional.hpp>
#include "MuvGeom.hh"
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

    class PassMuon : public SingleMuon
    {
        public:
            PassMuon( const SingleTrack& st);
            void new_event();
            bool found() const { return true; }
            double weight() const { return 1.0; }
            double x() const{ return x_; }
            double y() const{ return y_; }

        private:
            const SingleTrack& st_;
            mutable double x_;
            mutable double y_;
    };


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

    class ScatterSingleMuon  : public SingleMuon
    {
        public:
            ScatterSingleMuon
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

            virtual boost::optional<toymc::track_params> 
                get_muon_track( const fne::Event * e, const SingleTrack& st ) const = 0;

            virtual std::pair<double,double> get_muon_position( toymc::track_params tp) const;

            mutable bool dirty_;
            mutable bool found_muon_;
            mutable double x_;
            mutable double y_;

            MuonScatterSim mss_;
            PolarityFinder pf_;

            MuvGeom muv_geom_;
    };

    //--------------------------------------------------

    //Uses MC muon particle and simulates scattering
    class MCScatterSingleMuon : public ScatterSingleMuon
    {
        public:
            MCScatterSingleMuon
                ( const fne::Event * e, const SingleTrack& st);

            boost::optional<toymc::track_params> 
                get_muon_track( const fne::Event * e, const SingleTrack& st ) const;
    };

    //--------------------------------------------------

    //Uses halo muon track and simulates scattering
    class HaloScatterSingleMuon : public ScatterSingleMuon
    {
        public:
            HaloScatterSingleMuon
                ( const fne::Event * e, const SingleTrack& st);

            boost::optional<toymc::track_params> 
                get_muon_track( const fne::Event * e, const SingleTrack& st ) const;
    };
}
#endif

