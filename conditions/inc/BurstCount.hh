#ifndef BURSTCOUNT_HH
#define BURSTCOUNT_HH
#include "Analysis.hh"
#include "Event.hh"
#include "TTree.h"
#if 0
/*
 *  ____                 _    ____                  _
 * | __ ) _   _ _ __ ___| |_ / ___|___  _   _ _ __ | |_
 * |  _ \| | | | '__/ __| __| |   / _ \| | | | '_ \| __|
 * | |_) | |_| | |  \__ \ |_| |__| (_) | |_| | | | | |_
 * |____/ \__,_|_|  |___/\__|\____\___/ \__,_|_| |_|\__|
 *
 *
 */
#endif

class TFile;

namespace fn
{
    class K2piEventData;

    struct burst_info
    {
        Long64_t run;
        Long64_t burst_time;
        Long64_t events;
        Double_t weight;
    };

    class BurstCount : public Analysis
    {
        public:
            BurstCount( const Selection& sel,
                    TFile& tfile, std::string folder,
                    std::string codename,
                    const fne::Event * e );

            void new_burst();
            void end_burst();
            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const fne::Event * e_;
            burst_info burst_info_;
            TTree burst_tree_;

            REG_DEC_SUB( BurstCount);
    };

    template<>
        Subscriber * create_subscriber<BurstCount>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class K2piBurstCount : public Analysis
    {
        public:
            K2piBurstCount( const Selection& sel,
                    TFile& tfile, std::string folder,
                    const K2piEventData& e );

            void new_burst();
            void end_burst();
            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const K2piEventData& e_;
            burst_info burst_info_;
            TTree burst_tree_;
    };
}
#endif
