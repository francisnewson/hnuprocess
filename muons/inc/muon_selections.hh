#ifndef MUON_SELECTIONS_HH
#define MUON_SELECTIONS_HH
#include "Selection.hh"
#include "Event.hh"
#include "Effs2D.hh"
#if 0
/*
 *                                           _           _   _
 *  _ __ ___  _   _  ___  _ __      ___  ___| | ___  ___| |_(_) ___  _ __  ___
 * | '_ ` _ \| | | |/ _ \| '_ \    / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * | | | | | | |_| | (_) | | | |   \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |_| |_| |_|\__,_|\___/|_| |_|___|___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                            |_____|
 *
 */
#endif
namespace fn
{

    class SingleMuon;
    class SingleTrack;
    class SingleRecoTrack;

    //Require a muon associated with the track
    //Always needs MUV1 and MUV2
    //Efficiency is handled by SingleMuon class
    //Additional cut on track-muon separation is handled here.
    class MuonReq : public CachedSelection
    {
        public:
            MuonReq( const SingleMuon& sm, 
                    const SingleTrack& st, double multiplier );

        private:
            bool do_check() const;
            double do_weight() const;

            const SingleMuon& sm_;
            const SingleTrack& st_;
            double multiplier_;

            REG_DEC_SUB( MuonReq );
    };

    template<>
        Subscriber * create_subscriber<MuonReq>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class NoAssMuon : public CachedSelection
    {
        public:
            NoAssMuon( const SingleMuon& sm, 
                    const SingleTrack& st, double multiplier );

        private:
            bool do_check() const;
            double do_weight() const;

            const SingleMuon& sm_;
            const SingleTrack& st_;
            double multiplier_;

            REG_DEC_SUB( NoAssMuon );
    };

    template<>
        Subscriber * create_subscriber<NoAssMuon>
        (YAML::Node& instruct, RecoFactory& rf );



    //--------------------------------------------------

    bool check_muon_track_distance
        ( const SingleMuon& sm, const SingleRecoTrack& srt, double multiplier );

    std::pair<double,double> get_muon_track_separation
        ( const SingleMuon& sm, const SingleRecoTrack& srt );

    //--------------------------------------------------

    class MuonXYWeight : public CachedSelection
    {
        public:
            MuonXYWeight( const SingleTrack& st,
                    std::string muon_effs_file );

        private:
            bool do_check() const;
            double do_weight() const;

            const SingleTrack& st_;
            Effs2D effs_;

            REG_DEC_SUB( MuonXYWeight );
    };

    template<>
        Subscriber * create_subscriber<MuonXYWeight>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class MuonTHXYWeight : public CachedSelection
    {
        public:
            MuonTHXYWeight( const SingleTrack& st, const TH2D& heffs );

        private:
            bool do_check() const;
            double do_weight() const;

            const SingleTrack& st_;
            EffsTH2D effs_;

            REG_DEC_SUB( MuonTHXYWeight );
    };

    template<>
        Subscriber * create_subscriber<MuonTHXYWeight>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------


    class MuonTHPWeight : public CachedSelection 
    {
        public:
            MuonTHPWeight( const SingleTrack& st , const TH1D& heffs );

        private:
            bool do_check() const;
            double do_weight() const;

            const SingleTrack& st_;
            EffsTH1D effs_;

            REG_DEC_SUB( MuonTHPWeight );
    };

    template<>
        Subscriber * create_subscriber<MuonTHPWeight>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
