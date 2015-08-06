#ifndef FLEXISCALING_HH
#define FLEXISCALING_HH
#if 0
/*
 *  _____ _           _ ____            _ _
 * |  ___| | _____  _(_) ___|  ___ __ _| (_)_ __   __ _
 * | |_  | |/ _ \ \/ / \___ \ / __/ _` | | | '_ \ / _` |
 * |  _| | |  __/>  <| |___) | (_| (_| | | | | | | (_| |
 * |_|   |_|\___/_/\_\_|____/ \___\__,_|_|_|_| |_|\__, |
 *                                                |___/
 *
 */
#endif
#include "yaml_help.hh"
#include "TH1D.h"
#include <memory>
#include "MultiScaling.hh"

namespace fn
{
    class FlexHaloScale
    {
        public:
            virtual void compute_scaling(){}
            virtual double  get_halo_scale() = 0;
            virtual double  get_halo_scale_error() = 0;
    };

    class FlexKaonFlux
    {
        public:
            virtual void compute_scaling(){}
            virtual double  get_fiducial_flux() = 0;
            virtual double  get_fiducial_flux_error() = 0;
            virtual double  get_halo_scale() = 0;
            virtual double  get_halo_scale_error() = 0;
    };

    class FlexPeakFlux
    {
        public:
            virtual void compute_scaling(){}
            virtual double  get_fiducial_flux() = 0;
            virtual double  get_fiducial_flux_error() = 0;
    };

    struct sumdef
    {
        std::string filename;
        std::string pre;
        std::string post;
        double min;
        double max;
    };

}

namespace YAML
{
    template<>
        struct convert <fn::sumdef>
        {
            static Node encode ( const fn::sumdef & sd );
            static bool decode( const Node& node, fn::sumdef& sd );
        };
}

namespace fn
{

    double get_chan_integral( std::string chan, const sumdef& sum );

    //--------------------------------------------------

    class MonoHaloScale : public FlexHaloScale
    {
        public:
            MonoHaloScale( const YAML::Node instruct );
            void compute_scaling();
            double  get_halo_scale();
            double  get_halo_scale_error();

        private:
            double halo_scale_;
            double halo_scale_error_;

            std::string halo_chan_;
            sumdef halo_sumdef_;

            std::string data_chan_;
            sumdef data_sumdef_;
    };

    //--------------------------------------------------

    class StackPeakFlux : public FlexPeakFlux
    {
        public:
            StackPeakFlux( const YAML::Node& instruct,
                    bool do_halo, FlexHaloScale * halo_scaler );
            void compute_scaling();
            double  get_fiducial_flux();
            double  get_fiducial_flux_error();

        private:

            //halo
            bool do_halo_;
            std::string halo_chan_;
            sumdef halo_sumdef_;
            FlexHaloScale * halo_scale_;

            //data
            std::string data_chan_;
            sumdef data_sumdef_;

            //stack
            struct stack_chan
            { std::string chan; std::string type; std::string fidname; };

            std::vector<stack_chan> stack_chans_;
            sumdef stack_sumdef_;
            std::string norm_chan_;

            std::map<std::string, double> fid_weights_;
            std::map<std::string, double> brs_;

            //result
            double k_flux_;
            double k_flux_error_;
    };

    //--------------------------------------------------

    class ComboKaonFlux : public FlexKaonFlux
    {
        public:
            ComboKaonFlux( const YAML::Node & instruct );
            void compute_scaling();
            double  get_fiducial_flux();
            double  get_fiducial_flux_error();
            double  get_halo_scale();
            double  get_halo_scale_error();

        private:
            std::unique_ptr<FlexHaloScale> halo_scale_;
            std::unique_ptr<FlexPeakFlux> peak_flux_;
    };

    //--------------------------------------------------

    class FlexiScaling : public SampleScaler
    {
        public:
            FlexiScaling( const YAML::Node& instruct );

            void compute_scaling();
            double scale_hist( TH1& h, const YAML::Node& instruct ) const;

            double get_halo_scale() const;
            double get_halo_scale_error() const;

            double get_peak_scale() const { return 1.0; }
            double get_peak_scale_error() const  { return 1.0; }

            double get_fiducial_flux() const;
            double get_fiducial_flux_error() const;

        private:

            std::unique_ptr<FlexKaonFlux> kaon_flux_;
            std::unique_ptr<FlexHaloScale> halo_scale_;

            std::map<std::string, double> fid_weights_;
            std::map<std::string, double> brs_;
    };

    //--------------------------------------------------
}
#endif
