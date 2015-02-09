#ifndef FILTER_HH
#define FILTER_HH
#include "Analysis.hh"
#if 0
/*
 *  _____ _ _ _
 * |  ___(_) | |_ ___ _ __
 * | |_  | | | __/ _ \ '__|
 * |  _| | | | ||  __/ |
 * |_|   |_|_|\__\___|_|
 *
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
    class Filter : public Analysis
    {
        public:
            Filter(const Selection& sel, TFile& tf,  const fne::Event * e);

            void end_processing();

        private:
            void process_event();

            std::unique_ptr<TTree> ttree_;
            TFile& tf_;

            const fne::Event * e_;

            REG_DEC_SUB( Filter );
    };

    template<>
        Subscriber * create_subscriber<Filter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
