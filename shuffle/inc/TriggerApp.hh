#ifndef TRIGGERAPP_HH
#define TRIGGERAPP_HH
#if 0
/*
 *  _____     _                        _
 * |_   _| __(_) __ _  __ _  ___ _ __ / \   _ __  _ __
 *   | || '__| |/ _` |/ _` |/ _ \ '__/ _ \ | '_ \| '_ \
 *   | || |  | | (_| | (_| |  __/ | / ___ \| |_) | |_) |
 *   |_||_|  |_|\__, |\__, |\___|_|/_/   \_\ .__/| .__/
 *              |___/ |___/                |_|   |_|
 *
 */
#endif
#include <vector>
#include <memory>
#include <string>
#include "TFile.h"
#include "TH1D.h"

namespace fn
{
    class TriggerApp
    {
        public:
            TriggerApp( TFile& tf );
            void set_sels( std::vector<std::string> sels );
            void init();
            void rebin( int r );

            const TH1D&  get_h_passed(){ return *h_passed_ ; }
            const TH1D&  get_h_all(){ return *h_all_ ; }

            std::pair<double,double> get_eff_err( double sigmin, double sigmax );
            double get_eff( double sigmin, double sigmax );
            double get_err( double sigmin, double sigmax );

            bool correct_hist( TH1& h );

        private:
            std::vector<std::string> sels_;
            TFile& tf_;
            std::unique_ptr<TH1D> h_passed_;
            std::unique_ptr<TH1D> h_all_;
    };

    std::pair<double,double> eff_err( double passed, double all );
}
#endif
