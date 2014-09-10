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
    class MuonReqStatus : public CachedSelection
    {
        public:
            MuonReqStatus( const fne::Event * e ,
                    std::set<int> allowed_status );

        private:
            bool do_check() const;
            const fne::Event * e_;
            std::set<int> allowed_status_;

    };

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
}

#endif