#ifndef KM2_SELECTIONS_HH
#define KM2_SELECTIONS_HH
#include "Selection.hh"
#include "Km2Clusters.hh"
#include "Km2Reco.hh"
#include "AreaCut.hh"

namespace fn
{
    class Km2NoBadCluster : public CachedSelection
    {
        public:
            Km2NoBadCluster( const Km2Clusters& km2c );

        private:
            bool do_check() const;
            const Km2Clusters& km2c_;

            REG_DEC_SUB( Km2NoBadCluster );
    };

    template<>
        Subscriber * create_subscriber<Km2NoBadCluster>
        (YAML::Node& instruct, RecoFactory& rf );

    class Km2NoCluster : public CachedSelection
    {
        public:
            Km2NoCluster( const Km2Clusters& km2c );

        private:
            bool do_check() const;
            const Km2Clusters& km2c_;

            REG_DEC_SUB( Km2NoCluster );
    };

    template<>
        Subscriber * create_subscriber<Km2NoCluster>
        (YAML::Node& instruct, RecoFactory& rf );


    //--------------------------------------------------

    class Km2M2Miss : public CachedSelection
    {
        public:
            Km2M2Miss( const Km2Event& km2_event,
                    double min_m2, double max_m2);

        private:
            bool do_check() const;
            const Km2Event& km2_event_;

            double min_m2_;
            double max_m2_;

            REG_DEC_SUB( Km2M2Miss );
    };


    template<>
        Subscriber * create_subscriber<Km2M2Miss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
    
    class K2piM2Miss : public CachedSelection
    {
        public:
            K2piM2Miss( const Km2Event& km2_event,
                    double min_m2, double max_m2);

        private:
            bool do_check() const;
            const Km2Event& km2_event_;

            double min_m2_;
            double max_m2_;

            REG_DEC_SUB( K2piM2Miss );
    };

    template<>
        Subscriber * create_subscriber<K2piM2Miss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------


    class Km2PM2Miss : public CachedSelection
    {
        public:
            Km2PM2Miss(  const Km2Event& km2_event,
                    std::vector<rectangle> pm2_recs );

        private:
            bool do_check() const;

            const Km2Event& km2_event_;
            const AreaCut area_cut_;

            REG_DEC_SUB( Km2PM2Miss );
    };

    template<>
        Subscriber * create_subscriber<Km2PM2Miss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km2HotLkr : public CachedSelection
    {
        public:
            Km2HotLkr( const  Km2Event& km2_event,
                    const Km2Clusters& km2_clusters,
                    std::vector<std::pair<int, int> > hot_lkr_cells );

        private:
            bool do_check() const;

            const Km2Event& km2_event_;
            const Km2Clusters& km2_clusters_;
            std::vector<std::pair<int, int> > hot_lkr_cells_;

            REG_DEC_SUB( Km2HotLkr );
    };

    template<>
        Subscriber * create_subscriber<Km2HotLkr>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km2TrackClusterEP : public CachedSelection
    {
        public:
            Km2TrackClusterEP
                ( const Km2Clusters& km2c, const SingleTrack& st,
                  double min_eop, double max_eop, 
                  bool check_multiple);

        private:
            bool do_check() const;
            const Km2Clusters& km2_clusters_;
            const SingleTrack& st_;

            double min_eop_;
            double max_eop_;
            bool check_multiple_;

            REG_DEC_SUB( Km2TrackClusterEP );
    };

    template<>
        Subscriber * create_subscriber<Km2TrackClusterEP>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km2Angle : public CachedSelection
    {
        public:
            Km2Angle ( const Km2Event& km2_event, double min_t, double max_t );

        private:
            bool do_check() const;

            const Km2Event& km2_event_;
            double min_t_;
            double max_t_;

            REG_DEC_SUB( Km2Angle );
    };

    template<>
        Subscriber * create_subscriber<Km2Angle>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class TrackZT : public CachedSelection
    {
        public:
            TrackZT( const Km2Event& km2e,
                    std::vector<polygon_type> zt_region );

        private:
            bool do_check() const;

            const Km2Event& km2e_;
            const AreaCut area_cut_;

            REG_DEC_SUB( TrackZT);
    };

    template<>
        Subscriber * create_subscriber<TrackZT>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
    
    class TrackTPhi : public CachedSelection
    {
        public:
            TrackTPhi( const Km2Event& km2e,
                    std::vector<polygon_type> tphi_region );

        private:
            bool do_check() const;

            const Km2Event& km2e_;
            const AreaCut area_cut_;

            REG_DEC_SUB( TrackTPhi);
    };

    template<>
        Subscriber * create_subscriber<TrackTPhi>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
    //
    class TrackPT : public CachedSelection
    {
        public:
            TrackPT( const Km2Event& km2e,
                    std::vector<polygon_type> pt_region );

        private:
            bool do_check() const;

            const Km2Event& km2e_;
            const AreaCut area_cut_;

            REG_DEC_SUB( TrackPT);
    };

    template<>
        Subscriber * create_subscriber<TrackPT>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif
