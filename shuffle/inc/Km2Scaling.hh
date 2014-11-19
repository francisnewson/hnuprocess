#ifndef KM2SCALING_HH
#define KM2SCALING_HH
#include "yaml_help.hh"
#include "TH1.h"
#if 0
/*
 *  _  __          ____  ____            _ _
 * | |/ /_ __ ___ |___ \/ ___|  ___ __ _| (_)_ __   __ _
 * | ' /| '_ ` _ \  __) \___ \ / __/ _` | | | '_ \ / _` |
 * | . \| | | | | |/ __/ ___) | (_| (_| | | | | | | (_| |
 * |_|\_\_| |_| |_|_____|____/ \___\__,_|_|_|_| |_|\__, |
 *                                                 |___/
 *
 */
#endif
namespace fn
{
    class Km2Scaling
    {
        public:
            Km2Scaling( const YAML::Node& scaling_config, 
                    const std::map<std::string, double>& fiducial_weights,
                    const std::map<std::string, double>& brs);

            void compute_scaling();

            double get_halo_scale() const
            { return halo_scale_ ; }

            double get_halo_scale_error() const
            { return halo_scale_error_; }

            double get_km2_scale() const
            { return km2_scale_; }

            double get_km2_scale_error() const 
            { return km2_scale_error_; }

            double get_km2_fid_weight() const
            { return km2_fid_weight_; }

            void scale_hist( TH1& h, const YAML::Node& instruct ) const;

        private:
            const YAML::Node& scaling_config_;

            double halo_scale_;
            double halo_scale_error_;

            double km2_scale_;
            double km2_scale_error_;

            double km2_fid_weight_;
            double km2_br_ ;

            const std::map<std::string, double>& fiducial_weights_;
            const std::map<std::string, double>& brs_;
    };
}
#endif

