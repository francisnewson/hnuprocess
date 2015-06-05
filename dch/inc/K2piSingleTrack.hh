#ifndef K2PISINGLETRACK_HH

#define K2PISINGLETRACK_HH
#include "SingleTrack.hh"
#include "K2piEventData.hh"
#if 0
/*
 *  _  ______        _ ____  _             _     _____               _
 * | |/ /___ \ _ __ (_) ___|(_)_ __   __ _| | __|_   _| __ __ _  ___| | __
 * | ' /  __) | '_ \| \___ \| | '_ \ / _` | |/ _ \| || '__/ _` |/ __| |/ /
 * | . \ / __/| |_) | |___) | | | | | (_| | |  __/| || | | (_| | (__|   <
 * |_|\_\_____| .__/|_|____/|_|_| |_|\__, |_|\___||_||_|  \__,_|\___|_|\_\
 *            |_|                    |___/
 *
 */
#endif
namespace fn
{
    class K2piSingleRecoTrack : public SingleRecoTrack
    {
        public:
            void update( const K2piDchData& dch_data, const K2piLkrData& lkr_data );

                virtual int get_charge() const ;

            virtual TVector3 get_3mom() const ;
            virtual double get_mom() const ;

            virtual TVector3 get_vertex() const ;
            virtual double get_cda() const ;

            virtual double get_time() const ;
            virtual double get_adjusted_time() const ;

            virtual double get_quality() const ;

            virtual double get_ddead_cell() const ;

            virtual TVector3 get_unscattered_3mom() const   ;

            //downstream
            virtual TVector3 extrapolate_ds( double z) const  ;
            virtual TVector3 get_ds_mom() const ;

            //raw upstream
            virtual TVector3 extrapolate_us( double z) const  ;
            virtual TVector3 get_us_mom() const ;

            //BF track
            virtual TVector3 extrapolate_bf( double z) const  ;

            virtual int get_compact_id() const ;
            virtual int get_muon_id() const ;

            virtual ~K2piSingleRecoTrack(){}

        private:
            Track us_track_;
            Track ds_track_;
            Track kaon_track_;
            double mom_;
            Vertex vertex_;
    };


    //--------------------------------------------------

    class K2piSingleTrack : public SingleTrack
    {
        public:
            K2piSingleTrack(const K2piDchData & dch_data, 
                    const K2piLkrData& lkr_data);

        private:
            virtual bool process_event() const;
            const K2piDchData & dch_data_;
            const K2piLkrData & lkr_data_;
            mutable K2piSingleRecoTrack single_reco_track_;
    };
}
#endif

