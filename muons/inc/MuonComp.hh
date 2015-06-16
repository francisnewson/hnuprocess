#ifndef MUONCOMP_HH
#define MUONCOMP_HH
#include "Analysis.hh"

#if 0
/*
 *  __  __                    ____
 * |  \/  |_   _  ___  _ __  / ___|___  _ __ ___  _ __
 * | |\/| | | | |/ _ \| '_ \| |   / _ \| '_ ` _ \| '_ \
 * | |  | | |_| | (_) | | | | |__| (_) | | | | | | |_) |
 * |_|  |_|\__,_|\___/|_| |_|\____\___/|_| |_| |_| .__/
 *                                               |_|
 *
 */
#endif
namespace fne
{
    class Event;
}
namespace fn
{
    class SingleTrack;
    class SingleMuon;

    class MuonComp : public Analysis
    {
        public:
            MuonComp( const Selection& sel,
                    const fne::Event * e,
                    const SingleTrack& st,
                     SingleMuon& sm1, SingleMuon& sm2 );

            void end_processing(){}

        private:
            void process_event();

            const fne::Event * e_;
            const SingleTrack&  st_;
            SingleMuon& sm1_;
            SingleMuon& sm2_;

            REG_DEC_SUB( MuonComp );
    };

    template<>
        Subscriber * create_subscriber<MuonComp>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
