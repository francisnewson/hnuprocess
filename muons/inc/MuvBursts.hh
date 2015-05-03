#ifndef MUVBURSTS_HH
#define MUVBURSTS_HH
#include "Analysis.hh"
#include <memory>
#include "Rtypes.h"
#include "TTree.h"

#if 0
/*
 *  __  __             ____                 _
 * |  \/  |_   ___   _| __ ) _   _ _ __ ___| |_ ___
 * | |\/| | | | \ \ / /  _ \| | | | '__/ __| __/ __|
 * | |  | | |_| |\ V /| |_) | |_| | |  \__ \ |_\__ \
 * |_|  |_|\__,_| \_/ |____/ \__,_|_|  |___/\__|___/
 *
 *
 */
#endif
namespace fne
{
    class Event;
}

class TFile;

namespace fn
{
    struct muv_burst
    {
        Long64_t run;
        Long64_t burst_time;

        double total;
        double pass;
    };


    class MuvBursts : public Analysis
    {
        public:
            MuvBursts( const Selection& sel, const Selection& muon_req,
                    const fne::Event *e , 
                    TFile& tf, std::string folder );

            void new_burst();
            void end_burst();

            void end_processing();

        private:
            void process_event();

            TFile& tf_;
            std::string folder_;

            const fne::Event * e_;
            const Selection& mr_;
            muv_burst mb_;

            std::unique_ptr<TTree> ttree_;

            REG_DEC_SUB( MuvBursts );
    };


    template<>
        Subscriber * create_subscriber<MuvBursts>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
