#ifndef KM3_SELECTIONS_HH
#define KM3_SELECTIONS_HH
#include "Km3Reco.hh"
#if 0
/*
 *  _              _____             _           _   _
 * | | ___ __ ___ |___ /    ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * | |/ / '_ ` _ \  |_ \   / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * |   <| | | | | |___) |  \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |_|\_\_| |_| |_|____/___|___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                    |_____|
 *
 */
#endif
namespace fn
{

    class Km3M2Miss : public CachedSelection
    {
        public:
            Km3M2Miss( const Km3Reco& km3_reco,
                    double min_m2, double max_m2 );

        private:
            bool do_check() const;
            const Km3Reco& km3r_;
            double min_;
            double max_;

            REG_DEC_SUB( Km3M2Miss );
    };

    template<>
        Subscriber * create_subscriber<Km3M2Miss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km3PtMiss : public CachedSelection
    {
        public:
            Km3PtMiss( const Km3Reco& km3_reco,
                    double min_pt, double max_pt );

        private:
            bool do_check() const;
            const Km3Reco& km3r_;

            double min_;
            double max_;

            REG_DEC_SUB( Km3PtMiss );
    };

    template<>
        Subscriber * create_subscriber<Km3PtMiss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km3MomMiss : public CachedSelection
    {
        public:
            Km3MomMiss( const Km3Reco& km3_reco,
                    double min, double max );

        private:
            bool do_check() const;
            const Km3Reco& km3r_;

            double min_;
            double max_;

            REG_DEC_SUB( Km3MomMiss );
    };

    template<>
        Subscriber * create_subscriber<Km3MomMiss>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km3DV : public CachedSelection
    {
        public:
            Km3DV( const Km3Reco& km3_reco,
                    double min, double max );

        private:
            bool do_check() const;
            const Km3Reco& km3r_;

            double min_;
            double max_;

            REG_DEC_SUB( Km3DV );
    };

    template<>
        Subscriber * create_subscriber<Km3DV>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km3eop : public CachedSelection
    {
        public:
            Km3eop( const Km3Reco& km3_reco,
                    double min, double max );

        private:
            bool do_check() const;
            const Km3Reco& km3r_;

            double min_;
            double max_;

            REG_DEC_SUB( Km3eop );
    };

    template<>
        Subscriber * create_subscriber<Km3eop>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km3cda : public CachedSelection
    {
        public:
            Km3cda( const Km3Reco& km3_reco,
                    double min, double max );

        private:
            bool do_check() const;
            const Km3Reco& km3r_;

            double min_;
            double max_;

            REG_DEC_SUB( Km3cda );
    };

    template<>
        Subscriber * create_subscriber<Km3cda>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif

