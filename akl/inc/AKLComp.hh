#ifndef AKLCOMP_HH
#define AKLCOMP_HH
#include "Analysis.hh"
#include "Event.hh"
#include "TFile.h"
#include <iosfwd>
#if 0
/*
 *     _    _  ___     ____
 *    / \  | |/ / |   / ___|___  _ __ ___  _ __
 *   / _ \ | ' /| |  | |   / _ \| '_ ` _ \| '_ \
 *  / ___ \| . \| |__| |__| (_) | | | | | | |_) |
 * /_/   \_\_|\_\_____\____\___/|_| |_| |_| .__/
 *                                        |_|
 *
 */
#endif
namespace fn
{

    typedef std::pair<bool, bool> AKLPocket;

    class AKLStatus
    {
        public:
            AKLStatus(){}
            void reset();
            void add_hit( int pocket, int layer );

            bool pocket_or( int ipocket );
            bool pocket_and( int ipocket );

            bool any_pocket_or();
            bool any_pocket_and();

        private:
        std::array<AKLPocket,7> pockets_;
    };

    class AKLComp : public Analysis
    {
        public:
            AKLComp ( const Selection& sel,
                    const fne::Event * e,
                    TFile& tfile, std::string folder,
                    std::ostream& os );

            void new_run();
            void end_processing(){};

        private:
            void process_event();
            const fne::Event * e_;
            std::ostream& os_;

            TFile& tfile_;
            std::string folder_;

            AKLStatus akl_status_;

            REG_DEC_SUB( AKLComp );
    };

    template<>
        Subscriber * create_subscriber<AKLComp>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
