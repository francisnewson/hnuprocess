#ifndef HALOSUB_HH
#define HALOSUB_HH
#include "yaml_help.hh"
#include "MultiScaling.hh"
#include <iosfwd>
#include <memory>
#if 0
/*
 *  _   _       _      ____        _
 * | | | | __ _| | ___/ ___| _   _| |__
 * | |_| |/ _` | |/ _ \___ \| | | | '_ \
 * |  _  | (_| | | (_) |__) | |_| | |_) |
 * |_| |_|\__,_|_|\___/____/ \__,_|_.__/
 *
 *
 */
#endif
class TFile;

namespace fn
{

    void print_scaling( MultiScaling& ms, std::ostream& os );

    class HaloSub
    {
        public:
            HaloSub( TFile& tfout, TFile& tflog,  std::string config_file );

            void find_correction();
            void save_log_plots();

            void get_correction_factor();
            void do_subtractions();

        private:
            YAML::Node config_node_;
            std::map<std::string, double> brs_;

            std::unique_ptr<MultiScaling> calib_km2_scaling_;
            std::unique_ptr<MultiScaling> calib_k3pi_scaling_;

            double correction_factor_;
            TFile& tfout_;
            TFile& tflog_;
    };

    void copy_fids( TFile& tfout, const YAML::Node& instruct );
}
#endif
