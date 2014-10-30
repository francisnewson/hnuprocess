#ifndef KM2STATS_HH
#define KM2STATS_HH
#include "Analysis.hh"
#include "Km2Reco.hh"
#include "HistStore.hh"
#include "RunningStats.hh"
#include <iosfwd>
#if 0
/*
 *  _  __          ____  ____  _        _
 * | |/ /_ __ ___ |___ \/ ___|| |_ __ _| |_ ___
 * | ' /| '_ ` _ \  __) \___ \| __/ _` | __/ __|
 * | . \| | | | | |/ __/ ___) | || (_| | |_\__ \
 * |_|\_\_| |_| |_|_____|____/ \__\__,_|\__|___/
 *
 *
 */
#endif
namespace fn
{
    class Km2Stats : public Analysis
    {
        public:
        Km2Stats( const Selection& sel, std::string channel,
                TFile& tfile, std::string folder,
                std::ostream& os,
                const Km2Event& km2_event);

        void end_processing();

        private:
        void process_event();

        std::string channel_;

        TH1D * h_m2miss_;
        HistStore hs_;

        TFile& tfile_;
        std::string folder_;


        std::ostream& os_;

        RunningStats rs_;

        const Km2Event& km2_event_;


        REG_DEC_SUB( Km2Stats);
    };

    template<>
        Subscriber * create_subscriber<Km2Stats>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
