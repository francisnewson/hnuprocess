#ifndef MUONSELECTIONS_HH
#define MUONSELECTIONS_HH
#include "Selection.hh"
#include "Event.hh"
#include "RecoFactory.hh"
#include "MuonVeto.hh"
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
    class SingleMuon;

    class MuonReqStatus : public CachedSelection
    {
        public:
            MuonReqStatus( const MuonVeto& muv, 
                    std::set<int> allowed_status );

        private:
            bool do_check() const;
            const MuonVeto& muv_;
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
    
    //Handles both data and MC as simply as possible
    class CombinedMuonVeto : public CachedSelection
    {
        public:
            CombinedMuonVeto( const fne::Event * e, 
                    Eff2D muv_eff, const SingleMuon& sm,
                    const SingleTrack& st, 
                    double range_scale, bool mc );

        private:
            bool do_check() const;
            double do_weight() const;

            const fne::Event * e_;
            const SingleMuon& sm_;
            const SingleTrack& st_;

            double range_scale_;

            Eff2D muv_eff_;
            PolarityFinder pf_;
            bool mc_;

            REG_DEC_SUB( CombinedMuonVeto );
    };

    template<>
        Subscriber * create_subscriber<CombinedMuonVeto>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    double mu_error_0902_sc( double mom, int muv_plane );

}

#endif
