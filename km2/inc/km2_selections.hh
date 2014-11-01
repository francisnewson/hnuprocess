#ifndef KM2_SELECTIONS_HH
#define KM2_SELECTIONS_HH
#include "Selection.hh"
#include "Km2Clusters.hh"
#include "Km2Reco.hh"

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
}
#endif
