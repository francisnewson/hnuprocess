#ifndef TIMINGOFFSETS_HH
#define TIMINGOFFSETS_HH
#include "Analysis.hh"
#include "HistStore.hh"
#if 0
/*
 *  _____ _           _              ___   __  __          _
 * |_   _(_)_ __ ___ (_)_ __   __ _ / _ \ / _|/ _|___  ___| |_ ___
 *   | | | | '_ ` _ \| | '_ \ / _` | | | | |_| |_/ __|/ _ \ __/ __|
 *   | | | | | | | | | | | | | (_| | |_| |  _|  _\__ \  __/ |_\__ \
 *   |_| |_|_| |_| |_|_|_| |_|\__, |\___/|_| |_| |___/\___|\__|___/
 *                            |___/
 *
 */
#endif
namespace fne
{
    class Event;
};

namespace fn
{
    class TimingOffsets : public Analysis
    {
        public:
            TimingOffsets( const Selection& sel, const fne::Event * e ,
                    TFile& tfile, std::string folder);
            void end_processing();

        private:
            void process_event();
            const fne::Event * e_;

            HistStore hs_;
            TFile& tfile_;
            std::string folder_;

            TH1D * h_track_time_;
            TH1D * h_cluster_time_;
            TH1D * h_track_cluster_time_;


            REG_DEC_SUB( TimingOffsets);
    };

    template<>
        Subscriber * create_subscriber<TimingOffsets>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
