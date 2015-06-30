#ifndef K3PI_SELECTIONS_HH
#define K3PI_SELECTIONS_HH
#include "Selection.hh"
#if 0
/*
 *  _    _____       _              _           _   _
 * | | _|___ / _ __ (_)    ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * | |/ / |_ \| '_ \| |   / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * |   < ___) | |_) | |   \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |_|\_\____/| .__/|_|___|___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *            |_|    |_____|
 *
 */
#endif
namespace fn
{
    class K3piReco;

    class FoundK3pi : public CachedSelection
    {
        public:
            FoundK3pi ( const K3piReco&  k3pi_reco );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            REG_DEC_SUB( FoundK3pi );
    };

    template<>
        Subscriber * create_subscriber<FoundK3pi>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piCharge : public CachedSelection
    {
        public:
            K3piCharge ( const K3piReco& k2pi_reco, int charge );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            int charge_;

            REG_DEC_SUB( K3piCharge );
    };

    template<>
        Subscriber * create_subscriber<K3piCharge>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piTrackSep : public CachedSelection
    {
        public:
            K3piTrackSep ( const K3piReco& k2pi_reco, 
                    double min_track_sep );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            double min_track_sep_;

            REG_DEC_SUB( K3piTrackSep );
    };

    template<>
        Subscriber * create_subscriber<K3piTrackSep>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piDY : public CachedSelection
    {
        public:
            K3piDY ( const K3piReco& k2pi_reco, 
                    double max_dy );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            double max_dy_;

            REG_DEC_SUB( K3piDY );
    };

    template<>
        Subscriber * create_subscriber<K3piDY>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piChi2 : public CachedSelection
    {
        public:
            K3piChi2 ( const K3piReco& k2pi_reco, 
                    double max_chi2 );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            double max_chi2_;

            REG_DEC_SUB( K3piChi2 );
    };

    template<>
        Subscriber * create_subscriber<K3piChi2>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piPT2 : public CachedSelection
    {
        public:
            K3piPT2 ( const K3piReco& k2pi_reco, 
                    double max_pt2 );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            double max_pt2_;

            REG_DEC_SUB( K3piPT2 );
    };

    template<>
        Subscriber * create_subscriber<K3piPT2>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piM2M : public CachedSelection
    {
        public:
            K3piM2M ( const K3piReco& k2pi_reco, 
                    double half_width );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            double half_width_;

            REG_DEC_SUB( K3piM2M );
    };

    template<>
        Subscriber * create_subscriber<K3piM2M>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K3piZVertex : public CachedSelection
    {
        public:
            K3piZVertex ( const K3piReco& k2pi_reco, 
                    double min_z, double max_z );

        private:
            bool do_check() const;
            const K3piReco& k3pi_reco_;

            double min_z_;
            double max_z_;

            REG_DEC_SUB( K3piZVertex );
    };

    template<>
        Subscriber * create_subscriber<K3piZVertex>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif
