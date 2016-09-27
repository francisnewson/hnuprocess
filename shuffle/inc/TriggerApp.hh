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
            TriggerApp(){}
            virtual void set_num_denom( std::string num_name, std::string denom_name );
            virtual void set_sels( std::vector<std::string> sels );
            virtual void init();

            virtual void rebin( int r );

            virtual const TH1D&  get_h_passed(){ return *h_passed_ ; }
            virtual const TH1D&  get_h_all(){ return *h_all_ ; }

            virtual std::pair<double,double> 
                get_eff_err( double sigmin, double sigmax ) const;
            virtual double get_eff( double sigmin, double sigmax );
            virtual double get_err( double sigmin, double sigmax );

            virtual bool correct_hist( TH1& h );

        private:
            std::vector<std::string> sels_;
            TFile* tf_;
            std::unique_ptr<TH1D> h_passed_;
            std::unique_ptr<TH1D> h_all_;
            std::string num_name_;
            std::string denom_name_;
    };

    std::pair<double,double> eff_err( double passed, double all );

    class UniformTrigger  : public TriggerApp
    {
        public:
        UniformTrigger( double eff, double err ): eff_( eff ), err_( err ){}
        double get_eff( double sigmin, double sigmax ){ return eff_;}
        double get_err( double sigmin, double sigmax ){ return err_;}
        std::pair<double,double> 
            get_eff_err( double sigmin, double sigmax ) const
            { return std::pair<double,double>{ eff_, err_ } ;}

        private:
        double eff_;
        double err_;
    };
}
#endif
