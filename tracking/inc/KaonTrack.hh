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
    //Convenience functions to return track pos and dir
    //(no momentum info)
    Track get_kp_track( const fne::Conditions c );
    Track get_km_track( const fne::Conditions c );

    //--------------------------------------------------
    
    //Generate the appropriate yaml node based on channel
    YAML::Node auto_kaon_track( const YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    //base class for kaon track objects
    class KaonTrack : public Subscriber
    {
        public:
            virtual void new_event() ;

            //These are secondary methods which
            //maintain the obvious relationships
            TVector3 get_kaon_3mom() const;
            TVector3 get_kaon_point() const;
            TLorentzVector get_kaon_4mom() const;
            TVector3 extrapolate_z( double z ) const;

            //These methods actually define the track
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
            //These now return cached variables
            virtual const Track& get_kaon_track() const;
            virtual double get_kaon_mom() const;
            void new_event();

        private:
            //The work is delegated to theses functions
            virtual Track load_kaon_track() const = 0;
            virtual double load_kaon_mom() const = 0;

            //the cache
            Track kt_;
            double kmom_;
    };

    //--------------------------------------------------

    //Return K+ track with beta correction
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
    
    //Return K+ track without beta correction
    class UCKPTrack : public CachedKaonTrack
    {
        public:
            UCKPTrack ( const fne::Event * e , bool mc );
            virtual Track load_kaon_track() const;
            virtual double load_kaon_mom() const;

        private:
            const fne::Event * e_;
            bool mc_;
    };

    //--------------------------------------------------

    //Return K- track with beta correction
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

    //kaon properties for WeightedKTrack
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

    //Return K+ track from list of tracks weighted within polarities
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
