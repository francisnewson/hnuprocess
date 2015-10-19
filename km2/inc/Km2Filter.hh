#ifndef KM2FILTER_HH
#define KM2FILTER_HH
#include "Event.hh"
#include "Km2Reco.hh"
#include "Selection.hh"
#include "Analysis.hh"
#include "TTree.h"
#include "Km2EventData.hh"

#if 0
/*
 *  _  __          ____  _____ _ _ _
 * | |/ /_ __ ___ |___ \|  ___(_) | |_ ___ _ __
 * | ' /| '_ ` _ \  __) | |_  | | | __/ _ \ '__|
 * | . \| | | | | |/ __/|  _| | | | ||  __/ |
 * |_|\_\_| |_| |_|_____|_|   |_|_|\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class Km2Filter : public Analysis
    {
        public:
            Km2Filter( const Selection& sel,
                    const Km2Event& km2_event,
                    const Selection& muv_sel,
                    TFile& tf,  std::string folder,
                    std::string tree_name
                    );

            void end_processing();
        private:
            void process_event();
            const Km2Event& km2_event_;
            const Selection& muv_sel_;

            TFile& tfile_;
            std::string folder_;

            Km2EventData  event_data_;
            TTree * ttree_;

            REG_DEC_SUB( Km2Filter );
    };



}
#endif
