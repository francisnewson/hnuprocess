#ifndef MUONSELECTIONS_HH
#define MUONSELECTIONS_HH
#include "Selection.hh"
#include "Event.hh"
#include "RecoFactory.hh"
#include <set>

/*
 *
 *  _ __ ___  _   _  ___  _ __
 * | '_ ` _ \| | | |/ _ \| '_ \
 * | | | | | | |_| | (_) | | | |
 * |_| |_| |_|\__,_|\___/|_| |_|
 *           _           _   _
 *  ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *
 */
namespace fn
{
    class SingleTrack;

    class MuonReqStatus : public CachedSelection
    {
        public:
            MuonReqStatus( const fne::Event * e ,
                    std::set<int> allowed_status );

        private:
            bool do_check() const;
            const fne::Event * e_;
            std::set<int> allowed_status_;

            REG_DEC_SUB( MuonReqStatus );
    };

    template<>
        Subscriber * create_subscriber<MuonReqStatus>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class NoMuv : public CachedSelection
    {
        public:
            NoMuv( const fne::Event * e );

        private:
            bool do_check() const;
            const fne::Event * e_;

            REG_DEC_SUB( NoMuv );
    };

    template<>
        Subscriber * create_subscriber<NoMuv>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class FakeMuv : public CachedSelection
    {
        public:
            FakeMuv( const SingleTrack& st,
                    const fne::Event * e,
                    std::vector<double> muv1_effs,
                    std::vector<double> muv2_effs,
                    std::set<int> allowed_status );

        private:
            bool do_check() const;
            const SingleTrack& st_;
            const fne::Event * e_;
            std::vector<double> muv1_effs_;
            std::vector<double> muv2_effs_;
            std::set<int> allowed_status_;
            mutable std::uniform_real_distribution<double> uni_dist_;
            mutable std::default_random_engine generator_;

            REG_DEC_SUB( FakeMuv );
    };

    template<>
        Subscriber * create_subscriber<FakeMuv>
        (YAML::Node& instruct, RecoFactory& rf );
}

#endif
