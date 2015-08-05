#ifndef MERGING_HH
#define MERGING_HH
#include <memory>
#include "OwningStack.hh"
#include "Limiter.hh"
#include "TH1D.h"
#include "TFile.h"

#if 0
/*
 *                           _
 *  _ __ ___   ___ _ __ __ _(_)_ __   __ _
 * | '_ ` _ \ / _ \ '__/ _` | | '_ \ / _` |
 * | | | | | |  __/ | | (_| | | | | | (_| |
 * |_| |_| |_|\___|_|  \__, |_|_| |_|\__, |
 *                     |___/         |___/
 *
 */
#endif
namespace fn
{
    std::pair<std::unique_ptr<TH1>, std::unique_ptr<OwningStack>>
        extract_data_stack( TFile& tfin, std::vector<std::string> paths );

    std::unique_ptr<TH1D> get_cumulative_hist( TH1& h, double centre );

    //--------------------------------------------------

    class HistErrors
    {
        public:
            HistErrors(){}
            std::unique_ptr<TH1D> compute_errors( TH1D& h );

            void set_scatter_contrib( const ScatterContrib& sc );
            void set_trigger( const TriggerApp& trig );
            void set_halo_errors( const HaloErrors& he );

            double get_err( TH1D& bg, double sig_min, double sig_max );

        private:
            boost::optional<const ScatterContrib*> scat_contrib_;
            boost::optional<const TriggerApp*> trig_;
            boost::optional<const HaloErrors*> halo_errors_;
    };
}
#endif

