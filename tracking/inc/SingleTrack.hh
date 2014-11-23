#ifndef SINGLETRACK_HH
#define SINGLETRACK_HH
#include "TVector3.h"
#include "Subscriber.hh"
#include "RecoFactory.hh"
#include "Event.hh"
#include "RecoTrack.hh"
#include "BFCorrection.hh"
#include "Track.hh"
#include "KaonTrack.hh"
#if 0
/*
 *  ____  _             _     _____               _
 * / ___|(_)_ __   __ _| | __|_   _| __ __ _  ___| | __
 * \___ \| | '_ \ / _` | |/ _ \| || '__/ _` |/ __| |/ /
 *  ___) | | | | | (_| | |  __/| || | | (_| | (__|   <
 * |____/|_|_| |_|\__, |_|\___||_||_|  \__,_|\___|_|\_\
 *                |___/
 *
 */
#endif
namespace fn
{
    class SingleRecoTrack
    {
        public:
            virtual int get_charge() const = 0;

            virtual TVector3 get_3mom() const = 0;
            virtual double get_mom() const = 0;

            virtual TVector3 get_vertex() const = 0;
            virtual double get_cda() const = 0;

            virtual double get_time() const = 0;

            virtual double get_quality() const = 0;

            virtual double get_ddead_cell() const = 0;

            //downstream
            virtual TVector3 extrapolate_ds( double z) const = 0 ;
            virtual TVector3 get_ds_mom() const = 0;

            //raw upstream
            virtual TVector3 extrapolate_us( double z) const = 0 ;
            virtual TVector3 get_us_mom() const = 0;

            //BF track
            virtual TVector3 extrapolate_bf( double z) const = 0 ;

            virtual ~SingleRecoTrack(){}
    };

    class SingleTrack : public Subscriber
    {
        public:
            //Subscriber interface
            void new_event();

            //SingleTrack interface
            bool found_single_track() const;
            const SingleRecoTrack& get_single_track() const;

        protected:
            void set_reco_track( const SingleRecoTrack * srt );

        private:
            virtual bool process_event() const = 0;
            const SingleRecoTrack * single_reco_track_;

            //Results cacheing
            mutable bool dirty_;
            mutable bool found_;

            REG_DEC_SUB( SingleTrack );
    };

    template<>
        Subscriber * create_subscriber<SingleTrack>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    //BF

    struct processing_track
    {
        processing_track()
            :rt(0){ };
        processing_track( const processing_track& other );
        processing_track& operator= ( processing_track other );

        friend void swap
            ( processing_track& first, processing_track& second );

        double corr_mom;
        Vertex vert;
        bool good;

        fne::RecoTrack rt_;
        fne::RecoTrack * rt;
    };

    //--------------------------------------------------

    class BFSingleRecoTrack : public SingleRecoTrack
    {
        public:
            BFSingleRecoTrack();
            void update(  
                    const processing_track * proc_track, 
                    const fne::Event * event );

            int get_charge() const;

            TVector3 get_3mom() const;
            double get_mom() const;

            TVector3 get_vertex() const;
            double get_cda() const;

            int get_charge();
            double momentum();

            double get_time() const;

            double get_quality() const;

            double get_ddead_cell() const;

            TVector3 extrapolate_ds( double z) const;
            TVector3 extrapolate_us( double z) const;
            TVector3 extrapolate_bf( double z) const;

            TVector3 get_us_mom() const;
            TVector3 get_ds_mom() const;

        private:
            BFCorrection& bfc_;
            const processing_track * proc_track_;
            Track bf_track_;
    };

    //--------------------------------------------------


    class BFSingleTrack : public SingleTrack
    {
        public:
            BFSingleTrack( const fne::Event * event, 
                    YAML::Node& instruct, KaonTrack& kt );


        private:
            const fne::Event * event_;
            bool process_event() const;

            virtual void modify_processing_track
                ( processing_track& pt ) const
                { (void)pt; };

            mutable BFSingleRecoTrack single_reco_track_;

            SingleRecoTrack load_computed_track
                ( const processing_track& pt, 
                  SingleRecoTrack& srt ) const;

            //track monitoring
            mutable std::vector<processing_track> proc_tracks_;

            //tracking parameters
            double init_min_mom_;
            double init_max_mom_;

            double init_max_cda_;

            double init_min_z_;
            double init_max_z_;

            double dch_1_merge_sep_;

            KaonTrack& kt_;
    };

    //--------------------------------------------------
    
    class TrackScatterer
    {
        public:
        TrackScatterer(){}
        TrackScatterer(
                double angle_sigma, double angle_frequency,
                double mom_sigma, double mom_frequency );

        void set_angle_params( double sigma, double frequency);
        void set_mom_params( double sigma, double frequency);

        void scatter_track( Long64_t seed, 
                double& dxdz, double& dydz, double& mom ) const;

        private:
        double angle_sigma_;
        double angle_frequency_;

        double mom_sigma_;
        double mom_frequency_;
    };


    class BFScatterSingleTrack : public BFSingleTrack
    {
        public:
            BFScatterSingleTrack( const fne::Event * event,
                    YAML::Node& instruct, KaonTrack& kt );

        private:
            void modify_processing_track( processing_track& pt ) const;

            //Keep the event pointer so we can
            //generate pseudo random numbers
            const fne::Event * event_;

            TrackScatterer scatterer_;
    };

}
#endif
