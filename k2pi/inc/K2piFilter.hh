#ifndef K2PIFILTER_HH
#define K2PIFILTER_HH
#include "Analysis.hh"
#include "K2piReco.hh"
#include "Event.hh"
#include "TTree.h"
#include "TLorentzVector.h"
#include "K2piVars.hh"

#if 0
/*
 *  _  ______        _ _____ _ _ _
 * | |/ /___ \ _ __ (_)  ___(_) | |_ ___ _ __
 * | ' /  __) | '_ \| | |_  | | | __/ _ \ '__|
 * | . \ / __/| |_) | |  _| | | | ||  __/ |
 * |_|\_\_____| .__/|_|_|   |_|_|\__\___|_|
 *            |_|
 *
 */
#endif

namespace fn
{
    class K2piFilter : public Analysis
    {
        public:
            K2piFilter( 
                    const Selection& sel, 
                    TFile& tfile_, std::string tree_name,
                    const fne::Event * event,
                    const K2piReco& k2pir ,
                    const KaonTrack& kt,
                    bool mc );

            void end_processing();

        private:
            void process_event();
            void print_event( std::ostream& os );

            const fne::Event * event_;
            const K2piReco& k2pir_;
            const KaonTrack& kt_;

            K2piVars vars_;

            TFile& tfile_;
            TTree * ttree_;
            bool mc_;


            REG_DEC_SUB( K2piFilter);
    };

    //--------------------------------------------------


    template<>
        Subscriber * create_subscriber<K2piFilter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
