#ifndef CHOPPLOT_HH
#define CHOPPLOT_HH
#include "Analysis.hh"
#include "Selection.hh"
#include "TH1D.h"
#include "TFile.h"

#if 0
/*
 *   ____ _                 ____  _       _
 *  / ___| |__   ___  _ __ |  _ \| | ___ | |_
 * | |   | '_ \ / _ \| '_ \| |_) | |/ _ \| __|
 * | |___| | | | (_) | |_) |  __/| | (_) | |_
 *  \____|_| |_|\___/| .__/|_|   |_|\___/ \__|
 *                   |_|
 *
 */
#endif

namespace fn
{
    class Km2Event;
    class Km2RecoEvent;
    class SingleTrack;

    class M2MPlotter
    {
        public:
            M2MPlotter();
            void fill(const Km2RecoEvent& km2re,  double wgt );
            void write();

        private:
            TH1D  hm2m_;
    };

    //--------------------------------------------------

    class M2MChopPlot : public Analysis
    {
        public:
            M2MChopPlot( const Selection& sel, 
                    const Km2Event& km2e, const SingleTrack& st,
                    TFile& tf, std::string folder);

            void end_processing();

        private:
            struct ana
            {
                std::string name;
                std::unique_ptr<Selection> sel;
                std::unique_ptr<M2MPlotter> an;
            };

            void process_event();

            std::vector<std::unique_ptr<ana>> mom_analyses_;

            const Km2Event& km2e_;
            const SingleTrack& st_;

            TFile& tf_;
            std::string folder_;

            REG_DEC_SUB( M2MChopPlot );
    };

    template<>
        Subscriber * create_subscriber<M2MChopPlot>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif

