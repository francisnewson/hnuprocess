#ifndef SINGLETRACKPLOTTER_HH
#define SINGLETRACKPLOTTER_HH
#include "Analysis.hh"
#include "SingleTrack.hh"
#include "TrackPlotter.hh"
#if 0
/*
 *  ____  _             _     _____               _    ____  _       _   _
 * / ___|(_)_ __   __ _| | __|_   _| __ __ _  ___| | _|  _ \| | ___ | |_| |_ ___ _ __
 * \___ \| | '_ \ / _` | |/ _ \| || '__/ _` |/ __| |/ / |_) | |/ _ \| __| __/ _ \ '__|
 *  ___) | | | | | (_| | |  __/| || | | (_| | (__|   <|  __/| | (_) | |_| ||  __/ |
 * |____/|_|_| |_|\__, |_|\___||_||_|  \__,_|\___|_|\_\_|   |_|\___/ \__|\__\___|_|
 *                |___/
 *
 */
#endif
namespace fn
{
    class SingleTrackPlotter : public Analysis
    {
        public:
            SingleTrackPlotter( const Selection& sel , 
                    TFile& tfile, std::string folder,
                    const SingleTrack& st );

        private:
            void process_event();
            void end_processing();
            TFile& tfile_;
            std::string folder_;
            const SingleTrack& st_;
            TrackPlotter scattering_plots_;
            REG_DEC_SUB( SingleTrackPlotter);
    };

    template<>
        Subscriber * create_subscriber<SingleTrackPlotter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
