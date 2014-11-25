#ifndef TRACKPLOTTER_HH
#define TRACKPLOTTER_HH
#include "TVector3.h"
#include "HistStore.hh"
#if 0
/*
 *  _____               _    ____  _       _   _
 * |_   _| __ __ _  ___| | _|  _ \| | ___ | |_| |_ ___ _ __
 *   | || '__/ _` |/ __| |/ / |_) | |/ _ \| __| __/ _ \ '__|
 *   | || | | (_| | (__|   <|  __/| | (_) | |_| ||  __/ |
 *   |_||_|  \__,_|\___|_|\_\_|   |_|\___/ \__|\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class TrackPlotter
    {
        public:
            TrackPlotter( std::string name1, std::string name2);

            void Fill( TVector3 point_1, TVector3 mom_1, 
                    TVector3 point_2, TVector3 mom_2 , 
                    double weight);

            void Write();

        private:
            HistStore hs_;

            TH1D * h_dp_;
            TH1D * h_dpop2_;
            TH1D * h_dtx_;
            TH1D * h_dty_;
    };
}
#endif
