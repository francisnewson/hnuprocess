#ifndef TRIGGEREFFS_HH
#define TRIGGEREFFS_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include "Selection.hh"
#include "TH1D.h"

#if 0
/*
 *  _____     _                       _____  __  __
 * |_   _| __(_) __ _  __ _  ___ _ __| ____|/ _|/ _|___
 *   | || '__| |/ _` |/ _` |/ _ \ '__|  _| | |_| |_/ __|
 *   | || |  | | (_| | (_| |  __/ |  | |___|  _|  _\__ \
 *   |_||_|  |_|\__, |\__, |\___|_|  |_____|_| |_| |___/
 *              |___/ |___/
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

    class TriggerEffs : public Analysis
    {
        public:
            TriggerEffs( const Selection& base, const Selection& source, 
                    const Km2Event& km2e, TFile& tf, std::string folder);

            void end_processing();

        private:
            void process_event();

            HistStore hs_;
            TH1D * h_all_;
            TH1D * h_passed_;

            TH2D * h_all_p_t_;
            TH2D * h_passed_p_t_;

            const Selection& source_;
            const Km2Event& km2e_;

            TFile& tf_;
            std::string folder_;

            REG_DEC_SUB( TriggerEffs );
    };

    template<>
        Subscriber * create_subscriber<TriggerEffs>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Q11TTrigger : public CachedSelection
    {
        public:
            Q11TTrigger( const fne::Event * e, bool mc );

        private:
            bool do_check() const;

            const fne::Event * e_;
            bool mc_;

            REG_DEC_SUB( Q11TTrigger );
    };

    template<>
        Subscriber * create_subscriber<Q11TTrigger>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif

