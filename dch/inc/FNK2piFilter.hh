#ifndef FNK2PIFILTER_HH
#define FNK2PIFILTER_HH
#include "K2piEventData.hh"
#include "Analysis.hh"
#if 0
/*
 *  _____ _   _ _  ______        _ _____ _ _ _
 * |  ___| \ | | |/ /___ \ _ __ (_)  ___(_) | |_ ___ _ __
 * | |_  |  \| | ' /  __) | '_ \| | |_  | | | __/ _ \ '__|
 * |  _| | |\  | . \ / __/| |_) | |  _| | | | ||  __/ |
 * |_|   |_| \_|_|\_\_____| .__/|_|_|   |_|_|\__\___|_|
 *                        |_|
 *
 */
#endif

class TFile;
class TTree;

namespace fne
{
    class Event;
}

namespace fn
{
    class Selection;
    class SingleTrack;
    class KaonTrack;
    class K2piClusters;
    class K2piRecoClusters;
    class ClusterCorrector;
    class SingleRecoTrack;



    class FNK2piExtractor : public Subscriber
    {
        public:
            FNK2piExtractor( 
                    const Selection& weighter,
                    const fne::Event * event,
                    const SingleTrack & st ,
                    const K2piClusters& k2pi_clusters,
                    const ClusterCorrector& cluster_corrector,
                    const KaonTrack& kt,
                    bool mc );

            const K2piEventData * get_k2pi_event_ptr();
            void new_event();
            void update();

        private:
            void process_event();
            const Selection& weighter_;
            const fne::Event * e_;
            const SingleTrack& st_;
            const K2piClusters& k2pi_clusters_;
            const ClusterCorrector& cluster_corrector_;
            const KaonTrack& kt_;

            K2piEventData k2pi_event_data_;
            bool mc_;
            bool dirty_;


            REG_DEC_SUB( FNK2piExtractor );
    };

    FNK2piExtractor * get_fnk2pi_extractor
        ( YAML::Node& instruct, RecoFactory& rf );


    template<>
        Subscriber * create_subscriber<FNK2piExtractor>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    void extract_raw_lkr( const K2piRecoClusters& k2pirc, const ClusterCorrector& cc,
            const KaonTrack& kt , K2piLkrData& dest );

    void extract_raw_dch( const SingleRecoTrack& srt, 
            K2piDchData& dest );

    void extract_k2pi_mc( const fne::Event& e, K2piMcData& mc );

    double fit_lkr( const K2piLkrData& raw, const ClusterCorrector& cluster_corrector, 
            K2piLkrData& fit, K2piLkrData& err );


    //--------------------------------------------------

    class FNK2piFilter : public Analysis
    {
        public:
            FNK2piFilter( 
                    const Selection& sel, 
                    TFile& tfile, std::string tree_name,
                    FNK2piExtractor& extractor, bool mc) ;

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            FNK2piExtractor& extractor_;
            const K2piEventData * k2pi_event_;
            TTree * ttree_;

            bool mc_;

            REG_DEC_SUB( FNK2piFilter );
    };

    template<>
        Subscriber * create_subscriber<FNK2piFilter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
