#ifndef LIMITER_HH
#define LIMITER_HH
#if 0
/*
 *  _     _           _ _
 * | |   (_)_ __ ___ (_) |_ ___ _ __
 * | |   | | '_ ` _ \| | __/ _ \ '__|
 * | |___| | | | | | | | ||  __/ |
 * |_____|_|_| |_| |_|_|\__\___|_|
 *
 *
 */
#endif

#include <map>
#include <memory>
#include <string>
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include "TriggerApp.hh"

namespace fn
{
    class HnuLimParams
    {
        public:
            HnuLimParams( TFile& tf, 
                    const std::map<std::string, double>& fid_weights,
                    const std::vector<std::string>& regions,
                    int mass, bool fancyfit = false );

            int get_mass() const;
            std::string get_chan() const;

            const TH1D& get_signal_hist() const;

            double get_full_acceptance() const;
            double get_width_acceptance() const;
            double get_width_acceptance_err() const;

            double get_mean() const;
            double get_width() const;

            double get_width_min() const;
            double get_width_max() const;

            void write();

        private:
            void build_totals();
            void compute_shape();

            //inputs
            TFile& tf_;
            const std::map<std::string, double>& fid_weights_;
            const std::vector<std::string>& regions_;
            int mass_;
            bool fancy_fit_;


            //outputs
            std::unique_ptr<TH1D> h_sig_;

            double fid_total_;
            double sig_total_;
            std::string chan_;

            std::unique_ptr<TF1> fit_;
            std::unique_ptr<TF1> fita_;
            std::unique_ptr<TF1> fitb_;

            double mean_;
            double width_;

            double width_total_;
            double width_acc_;
            double width_acc_err_;

    };

    class HnuLimResult
    {
        public:
            double trig_eff;
            double trig_err;
            double background;
            double background_err;
            double acc_sig_ul;
            double orig_sig_ul;
            double rolke_orig_sig_ul;
            double ul_br;
            double ul_u2;

            std::map<std::string, double> error_budget;
    };

    //--------------------------------------------------

    class ScatterContrib
    {
        public:
            ScatterContrib( TFile& tf_noscat, TFile& tf_scat );
            double get_rel_scatter_err( double sig_min, double sig_max ) const;

        private:
            TFile& tf_noscat_;
            TFile& tf_scat_;
            std::unique_ptr<TH1>h_noscat_;
            std::unique_ptr<TH1> h_scat_;
    };

    //--------------------------------------------------

    class Limiter
    {
        public:
            Limiter ( TFile& bg_file, 
                    std::vector<std::string> regions );

            HnuLimResult get_limit( const HnuLimParams& params );
            std::pair<double,double> get_signal_range( TH1D& h );

            void set_bg_channels( std::vector<std::string> bgchans );
            void set_scatter_contrib( const ScatterContrib& sc );
            void set_trigger( const TriggerApp& trig );

            void set_halo_log_file( TFile& tf );

        private:
            std::unique_ptr<TH1> get_bg_hist
                ( std::string chan, std::string pol,  std::string region );

            double get_background( double sig_min, double sig_max );
            std::pair<double,double>
                get_trig_eff( double sig_min, double sig_max );

            std::pair<double,double> get_km2_flux_and_err();

            double compute_halo_scale_err( double sig_min, double sig_max );
            double compute_halo_val_err( double sig_min, double sig_max );

            double n_sigma_range_;

            TFile& bg_file_;

            std::vector<std::string> bg_channels_;
            std::vector<std::string> pols_;
            std::vector<std::string> regions_;
            std::vector<std::string> km2_flux_paths_;

            boost::optional<const ScatterContrib*> scat_contrib_;
            boost::optional<const TriggerApp*> trig_;

            TFile * halo_log_file_;
    };

    //--------------------------------------------------

    double br_to_mix(double BR, double mh);


}
#endif
