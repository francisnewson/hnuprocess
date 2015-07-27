#ifndef MULTISCALING_HH
#define MULTISCALING_HH
#include "yaml_help.hh"
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>
#if 0
/*
 *  __  __       _ _   _ ____            _ _
 * |  \/  |_   _| | |_(_) ___|  ___ __ _| (_)_ __   __ _
 * | |\/| | | | | | __| \___ \ / __/ _` | | | '_ \ / _` |
 * | |  | | |_| | | |_| |___) | (_| (_| | | | | | | (_| |
 * |_|  |_|\__,_|_|\__|_|____/ \___\__,_|_|_|_| |_|\__, |
 *                                                 |___/
 *
 */
#endif
class TH1;

namespace fn
{
    struct scale_result
    {
        double halo_scale;
        double halo_scale_error;
        double peak_scale;
        double peak_scale_error;
    };

    std::ostream& operator<<( std::ostream& os , const scale_result& sr );

    class ScaleStrategy
    {
        public:
            double get_halo_scale() const
            { return sr.halo_scale; }

            double get_halo_scale_error() const
            { return sr.halo_scale_error; }

            double get_peak_scale() const
            { return sr.peak_scale; }

            double get_peak_scale_error() const
            { return sr.peak_scale_error; }

            void update_scaling();

        private:
            virtual scale_result compute_scaling() = 0;
            scale_result sr;
    };

    class M2ScaleStrategy : public ScaleStrategy
    {
        public:
            M2ScaleStrategy(
                    const YAML::Node & channel_node,
                    const YAML::Node & strat_node
                    );

            scale_result compute_scaling();

        private:
            const YAML::Node& channel_node_;
            const YAML::Node& strat_node_;

            std::vector<std::string> halo_channels;
            std::vector<std::string> data_channels;
            std::vector<std::string> peak_channels;

            boost::filesystem::path input_file;
            std::string post_path;
            double m2_min_halo;
            double m2_max_halo;
            double m2_min_peak;
            double m2_max_peak;

            bool do_halo_;
            bool do_peak_;
    };

    class DummyScaleStrategy : public ScaleStrategy
    {
        public:
            DummyScaleStrategy(){}

        private:
            scale_result compute_scaling()
            { return scale_result{ 1,0, 1,0 }; }
    };

    //--------------------------------------------------

    class MultiScaling
    {
        public:
            MultiScaling( 
                    const YAML::Node& scaling_config, 
                    const std::map<std::string, double>& fiducial_weights,
                    const std::map<std::string, double>& brs);

            void scale_hist( TH1& h, const YAML::Node& instruct ) const;
            void compute_scaling() ;

            double get_halo_scale() const;
            double get_halo_scale_error() const;
            double get_peak_scale() const;
            double get_peak_scale_error() const;

            double get_fiducial_flux() const;
            double get_fiducial_flux_error() const;

        private:
            std::unique_ptr<ScaleStrategy> mc_strat_;
            std::unique_ptr<ScaleStrategy> halo_strat_;

            const YAML::Node& scaling_config_;

            const std::map<std::string, double>& fiducial_weights_;
            const std::map<std::string, double>& brs_;
            std::vector<std::string> peak_channels_;


            double peak_fid_weight_;
            double peak_br_;
            std::string peak_name_;
    };
}
#endif
