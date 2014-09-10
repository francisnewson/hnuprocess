#ifndef K2PISELECTIONS_HH
#define K2PISELECTIONS_HH
#include "K2piReco.hh"
#include "Selection.hh"
/*
 *  _    ____        _
 * | | _|___ \ _ __ (_)
 * | |/ / __) | '_ \| |
 * |   < / __/| |_) | |
 * |_|\_\_____| .__/|_|
 *           _|_|
 *  ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *
 *
 */
namespace fn
{
    class PipM2miss : public CachedSelection
    {
        public:
            PipM2miss( const K2piReco& k2pic,
                    double min_m2, double max_m2 );

        private:
            bool do_check() const;
            const K2piReco& k2pic_;
            double min_m2_;
            double max_m2_;

            REG_DEC_SUB( PipM2miss );
    };

    template<>
        Subscriber * create_subscriber<PipM2miss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
    
    class PhotonRadialCut : public CachedSelection
    {
        public: 
            PhotonRadialCut( const K2piReco& k2pic,
                        double z, double minR, double maxR );

        private:
                bool do_check() const;
                bool check_cluster
                    ( const ClusterData& cluster,
                      const TVector3& vertex ) const;

            const K2piReco& k2pic_;
                double z_;
                double min_r_;
                double max_r_;

            REG_DEC_SUB( PhotonRadialCut );
    };

    template<>
        Subscriber * create_subscriber<PhotonRadialCut>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif