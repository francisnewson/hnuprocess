#ifndef SINGLETRACK_HH
#define SINGLETRACK_HH
#include "TVector3.h"
#include "Subscriber.hh"
#include "RecoFactory.hh"
#include "Event.hh"
#include "RecoTrack.hh"
#include "BFCorrection.hh"
#include "Track.hh"
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
    //exceptions
    struct EventDoesNotContain{};
    struct UnknownSingleTrackMethod{};

    class SingleRecoTrack
    {
        public:
            int get_charge() const;
            TVector3 get_momentum() const;
    };

    class SingleTrack : public Subscriber
    {
        public:
            //Subscriber interface
            void new_event();

            //SingleTrack interface
            bool found_single_track();
            const SingleRecoTrack& get_single_track();

        protected:
            void set_reco_track( const SingleRecoTrack * srt );

        private:
            virtual bool process_event() = 0;
            const SingleRecoTrack * single_reco_track_;

            //Results cacheing
            bool dirty_;
            bool found_;

            REG_DEC_SUB( SingleTrack );
    };

    template<>
        Subscriber * create_subscriber<SingleTrack>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    //BF

    struct processing_track
    {
        double corr_mom;
        Vertex vert;
        bool good;

        fne::RecoTrack * rt;
    };

    class BFSingleRecoTrack : public SingleRecoTrack
    {
        public:
            void update(  
                    const processing_track * proc_track, 
                    const fne::Event * event );

            int get_charge();
            double momentum();

        private:
            BFCorrection bfc_;
            const processing_track * proc_track_;
            double momentum_;
            Track bf_track_;
    };


    class BFSingleTrack : public SingleTrack
    {
        public:
            BFSingleTrack( const fne::Event * event, 
                    YAML::Node& instruct );


        private:
            const fne::Event * event_;
            bool process_event();

            BFSingleRecoTrack single_reco_track_;

            SingleRecoTrack load_computed_track
                ( const processing_track& pt, 
                  SingleRecoTrack& srt );

            //track monitoring
            std::vector<processing_track> proc_tracks_;

            //tracking parameters
            double init_min_mom_;
            double init_max_mom_;

            double init_min_cda_;
            double init_max_cda_;

            double init_min_z_;
            double init_max_z_;

            double dch_1_merge_sep_;
    };

    //--------------------------------------------------



}
#endif
