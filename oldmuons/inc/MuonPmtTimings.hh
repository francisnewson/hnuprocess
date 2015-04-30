#ifndef MUONPMTTIMINGS_HH
#define MUONPMTTIMINGS_HH
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  __  __                   ____            _
 * |  \/  |_   _  ___  _ __ |  _ \ _ __ ___ | |_
 * | |\/| | | | |/ _ \| '_ \| |_) | '_ ` _ \| __|
 * | |  | | |_| | (_) | | | |  __/| | | | | | |_
 * |_|__|_|\__,_|\___/|_| |_|_|   |_| |_| |_|\__|
 * |_   _(_)_ __ ___ (_)_ __   __ _ ___
 *   | | | | '_ ` _ \| | '_ \ / _` / __|
 *   | | | | | | | | | | | | | (_| \__ \
 *   |_| |_|_| |_| |_|_|_| |_|\__, |___/
 *                            |___/
 *
 *
 *
 */
#endif
namespace fne
{
    class Event;
}

namespace fn
{
    class Km2Event;

    class MuonPmtTimings : public Analysis
    {
        public:
        MuonPmtTimings( const Selection& sel, 
                const Km2Event& km2_event,
                const fne::Event * e,
                TFile& tfile, std::string folder);

        void new_run();
        void end_run();
        void end_processing();

        private:
        void process_event();

        const Km2Event& km2e_;
        const fne::Event * e_;

        TFile& tfile_;
        std::string folder_;

        TH2D * h_chantimes_;
        HistStore hs_;


        REG_DEC_SUB( MuonPmtTimings );
    };


    template<>
        Subscriber * create_subscriber<MuonPmtTimings>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
