#ifndef TRACKING_SELECTIONS_HH
#define TRACKING_SELECTIONS_HH
#include "SingleTrack.hh"
#include "Selection.hh"
#include "AreaCut.hh"
#include "KaonTrack.hh"

#if 0
/*
 *  _                  _    _             
 * | |_ _ __ __ _  ___| | _(_)_ __   __ _ 
 * | __| '__/ _` |/ __| |/ / | '_ \ / _` |
 * | |_| | | (_| | (__|   <| | | | | (_| |
 *  \__|_|  \__,_|\___|_|\_\_|_| |_|\__, |
 *                                  |___/ 
 *           _           _   _                 
 *  ___  ___| | ___  ___| |_(_) ___  _ __  ___ 
 * / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                                             
 */
#endif

namespace fn
{
    SingleTrack * get_single_track
        ( YAML::Node& instruct, RecoFactory& rf );

    class FoundGoodTrack : public CachedSelection
    {
        public: 
            FoundGoodTrack( const SingleTrack& st );

        private:
            bool do_check() const;
            const SingleTrack& st_;

            REG_DEC_SUB( FoundGoodTrack);
    };

    template<>
        Subscriber * create_subscriber<FoundGoodTrack>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackCharge : public CachedSelection
    {
        public: 
            TrackCharge( const SingleTrack& st, int charge );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            int charge_;

            REG_DEC_SUB( TrackCharge);
    };

    template<>
        Subscriber * create_subscriber<TrackCharge>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackQuality : public CachedSelection
    {
        public:
            TrackQuality( const SingleTrack& st, double quality );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            double quality_;

            REG_DEC_SUB( TrackQuality);
    };

    template<>
        Subscriber * create_subscriber<TrackQuality>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackRadialAcceptance : public CachedSelection
    {
        public:

            enum class track_section { us, ds, bf };

            TrackRadialAcceptance( const SingleTrack& st, 
                    track_section ts, double z, 
                    double inner, double outer );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            mutable TVector3 zpoint_;
            track_section ts_;
            double z_;
            double inner_;
            double outer_;

            REG_DEC_SUB( TrackRadialAcceptance);
    };

    template<>
        Subscriber * create_subscriber<TrackRadialAcceptance>
        (YAML::Node& instruct, RecoFactory& rf );



    //--------------------------------------------------

    class TrackXAcceptance : public CachedSelection
    {
        public:
            enum class track_section { us, ds, bf };

            TrackXAcceptance( const SingleTrack& st , track_section ts,
                    double z, double minX, double maxX );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            mutable TVector3 zpoint_;
            track_section ts_;
            double z_;
            double min_x_;
            double max_x_;

            REG_DEC_SUB( TrackXAcceptance);
    };

    template<>
        Subscriber * create_subscriber<TrackXAcceptance>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackYAcceptance : public CachedSelection
    {
        public:
            enum class track_section { us, ds, bf };

            TrackYAcceptance( const SingleTrack& st , track_section ts,
                    double z, double minY, double maxY );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            mutable TVector3 zpoint_;
            track_section ts_;
            double z_;
            double min_y_;
            double max_y_;

            REG_DEC_SUB( TrackYAcceptance);
    };

    template<>
        Subscriber * create_subscriber<TrackYAcceptance>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
    class TrackPZ : public CachedSelection
    {
        public:
            TrackPZ( const SingleTrack& st,
                    std::vector<rectangle> recs );

        private:
            bool do_check() const;

            const SingleTrack& st_;
            const AreaCut area_cut_;

            REG_DEC_SUB( TrackPZ);
    };

    template<>
        Subscriber * create_subscriber<TrackPZ>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackCda : public CachedSelection
    {
        public:
            TrackCda( const SingleTrack& st,
                    double min_cda, double max_cda );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            double min_cda_;
            double max_cda_;

            REG_DEC_SUB( TrackCda);
    };

    template<>
        Subscriber * create_subscriber<TrackCda>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackMomentum : public CachedSelection
    {
        public:
            TrackMomentum( const SingleTrack& st,
                    double min_p , double max_p  );

        private:
            bool do_check() const;

            const SingleTrack& st_;
            double min_p_;
            double max_p_;

            REG_DEC_SUB( TrackMomentum);
    };

    template<>
        Subscriber * create_subscriber<TrackMomentum>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackVertexZ : public CachedSelection
    {
        public:
            TrackVertexZ( const SingleTrack& st,
                    double min_z , double max_z  );

        private:
            bool do_check() const;

            const SingleTrack& st_;
            double min_z_;
            double max_z_;
            REG_DEC_SUB( TrackVertexZ);
    };

    template<>
        Subscriber * create_subscriber<TrackVertexZ>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
    
    class TrackTime : public CachedSelection
    {

        public:
            TrackTime( const fne::Event * e,
                    const SingleTrack& st ,
                    double max_dt );

        private:
            bool do_check() const;

            const fne::Event * e_;
            const SingleTrack& st_;
            double max_dt_;

            REG_DEC_SUB( TrackTime);
    };


    template<>
        Subscriber * create_subscriber<TrackTime>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    struct PZTRegion
    {
        double minT;
        double maxT;
        AreaCut pz_cut;
    };

    class TrackPZT : public CachedSelection
    {
        public:
            TrackPZT( const fne::Event * e,
                    const SingleTrack& st, 
                    KaonTrack& kt,
                    bool mc ,
                    const YAML::Node& regions );

        private:
            bool do_check() const;
            const fne::Event * e_;
            const SingleTrack& st_;
            KaonTrack& kt_;
            std::vector< PZTRegion > regions_;

            REG_DEC_SUB( TrackPZT);
    };

    template<>
        Subscriber * create_subscriber<TrackPZT>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackLkrAcceptance : public CachedSelection
    {
        public:
            TrackLkrAcceptance( 
                    const fne::Event * e,
                    const SingleTrack& st, 
                    double margin_parameter );
        private:
            bool do_check() const;
            const SingleTrack& st_;
            const fne::Event * e_;

            double margin_parameter_;

            REG_DEC_SUB( TrackLkrAcceptance);
    };

    template<>
        Subscriber * create_subscriber<TrackLkrAcceptance>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackDDeadCell : public CachedSelection
    {
        public:
            TrackDDeadCell(
                    const SingleTrack& st,
                    double min_ddead_cell );

        private:
            bool do_check() const;
            const SingleTrack& st_;

            double min_ddead_cell_;
            REG_DEC_SUB( TrackDDeadCell);
    };

    template<>
        Subscriber * create_subscriber<TrackDDeadCell>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
