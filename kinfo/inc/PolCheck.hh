#ifndef POLCHECK_HH
#define POLCHECK_HH

#include "Analysis.hh"
#include "HistStore.hh"
#include "Event.hh"
#include "KaonTrack.hh"
#include "SingleTrack.hh"
#include "TH2D.h"

#if 0
/*
 *  ____       _  ____ _               _
 * |  _ \ ___ | |/ ___| |__   ___  ___| | __
 * | |_) / _ \| | |   | '_ \ / _ \/ __| |/ /
 * |  __/ (_) | | |___| | | |  __/ (__|   <
 * |_|   \___/|_|\____|_| |_|\___|\___|_|\_\
 *
 *
 */
#endif
namespace fn
{
    class PolCheck : public Analysis
    {
        public:
            PolCheck( const Selection& sel, 
                    TFile& tf, std::string folder,
                    std::ostream& fout,
                    const fne::Event * e, bool mc,
                    SingleTrack& st );

            void new_run();
            void end_run();

            void end_processing();

        private:
            void process_event();

            TH2D * h_momkick;
            TH2D * h_field;
            HistStore hs_;

            double field_sum_;
            double kick_sum_;
            int current_run_;
            int run_count_;

            TFile& tfile_;
            std::string folder_;

            std::ostream& fout_;
            
            const fne::Event * e_;
            SingleTrack& st_;

            std::map<int, double> run_average_field_;
            std::map<int, double> run_average_kick_;

            REG_DEC_SUB( PolCheck) ;
    };

    template<>
        Subscriber * create_subscriber<PolCheck>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
