#ifndef BURSTCOUNT_HH
#define BURSTCOUNT_HH
#include "Analysis.hh"
#include "Event.hh"
#include <map>
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
    class BurstCount : public Analysis
    {
        public:
            BurstCount( const Selection& sel,
                    TFile& tfile, std::string folder,
                    const fne::Event * e );

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const fne::Event * e_;
            std::map<int, int> bursts_;

            REG_DEC_SUB( BurstCount);
    };

    template<>
        Subscriber * create_subscriber<BurstCount>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
