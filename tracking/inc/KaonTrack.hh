#ifndef KAONTRACK_HH
#define KAONTRACK_HH
#include "RecoFactory.hh"
#include "Subscriber.hh"
#include "Event.hh"
#include "Track.hh"
#include "TLorentzRotation.h"
#include "RandomSampler.hh"
#include "PolarityFinder.hh"
#if 0
/*
 *  _  __               _____               _    
 * | |/ /__ _  ___  _ _|_   _| __ __ _  ___| | __
 * | ' // _` |/ _ \| '_ \| || '__/ _` |/ __| |/ /
 * | . \ (_| | (_) | | | | || | | (_| | (__|   < 
 * |_|\_\__,_|\___/|_| |_|_||_|  \__,_|\___|_|\_\
 *                                               
 * 
 */
#endif
namespace fn
{
    Track get_kp_track( const fne::Conditions c );
    Track get_km_track( const fne::Conditions c );

    //--------------------------------------------------
    
    YAML::Node auto_kaon_track( const YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class KaonTrack : public Subscriber
    {
        public:
            virtual void new_event() ;
            TVector3 get_kaon_3mom() const;
            TVector3 get_kaon_point() const;
            TLorentzVector get_kaon_4mom() const;
            TVector3 extrapolate_z( double z ) const;

            virtual const Track& get_kaon_track() const = 0;
            virtual double get_kaon_mom() const = 0;

            REG_DEC_SUB( KaonTrack );
    };

    template<>
        Subscriber * create_subscriber<KaonTrack>
        (YAML::Node& instruct, RecoFactory& rf );

    KaonTrack * get_kaon_track( YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class CachedKaonTrack : public KaonTrack
    {
        public:
            virtual const Track& get_kaon_track() const;
            virtual double get_kaon_mom() const;
            void new_event();

        private:
            virtual Track load_kaon_track() const = 0;
            virtual double load_kaon_mom() const = 0;

            Track kt_;
            double kmom_;
    };

    //--------------------------------------------------

    class RawKPTrack : public CachedKaonTrack
    {
        public:
            RawKPTrack ( const fne::Event * e , bool mc );
            virtual Track load_kaon_track() const;
            virtual double load_kaon_mom() const;

        private:
            const fne::Event * e_;
            bool mc_;
    };

    //--------------------------------------------------

    class RawKMTrack : public CachedKaonTrack
    {
        public:
            RawKMTrack ( const fne::Event * e , bool mc );

            virtual  Track load_kaon_track() const;
            virtual double load_kaon_mom() const;

        private:
            const fne::Event * e_;
            bool mc_;
    };

    //--------------------------------------------------

    struct kaon_properties
    {
        double dxdz;
        double dydz;
        double xoff;
        double yoff;
        double pmag;
    };


    std::istream& operator >> ( std::istream& is , kaon_properties& kp );
    std::ostream& operator << ( std::ostream& is , kaon_properties& kp );

    //--------------------------------------------------

    class WeightedKTrack : public KaonTrack
    {
        public:
            WeightedKTrack ( const fne::Event * e, bool mc, 
                    const YAML::Node& instruct );

            virtual void new_event() ;
            virtual const Track& get_kaon_track() const;
            virtual double get_kaon_mom() const ;

        private:
            const fne::Event * e_;
            bool mc_;

            Track kt_;
            double kmom_;

            RandomSampler< kaon_properties > pos_pol_kaon_sampler_; 
            RandomSampler< kaon_properties > neg_pol_kaon_sampler_; 
            PolarityFinder pf_;
    };

}
#endif
