#ifndef KM2_SELECTIONS_HH
#define KM2_SELECTIONS_HH
#include "Selection.hh"
#include "Km2Clusters.hh"

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
}
#endif
