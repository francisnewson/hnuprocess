#ifndef KM2SUB_HH
#define KM2SUB_HH
#if 0
/*
 *  _  __          ____  ____        _
 * | |/ /_ __ ___ |___ \/ ___| _   _| |__
 * | ' /| '_ ` _ \  __) \___ \| | | | '_ \
 * | . \| | | | | |/ __/ ___) | |_| | |_) |
 * |_|\_\_| |_| |_|_____|____/ \__,_|_.__/
 *
 *
 */
#endif
#include <memory>
#include "TFile.h"
#include "yaml_help.hh"
#include "HistExtractor.hh"
#include "TH1D.h"

namespace fn
{
    class Km2Sub
    {
        public:
            Km2Sub( TFile& tfout, TFile& tflog, std::string config_file );

            void process_all_plots();

        private:
            void process_plot( std::string plot_name, std::string pol );
            std::unique_ptr<TH1> get_sum( std::string chan, std::string pol );
            void scale_hist( TH1& h, std::string chan, std::string pol );

            YAML::Node config_node_;

            TFile &tfout_;
            TFile &tflog_;

            std::unique_ptr<TFile> tfin_;
            std::unique_ptr<RootTFileWrapper> rtfw_;
            std::unique_ptr<ChannelHistExtractor> ce_;

            std::map<std::string, std::string> names_;
            std::map<std::string, double> brs_;
            std::map<std::string, double> fid_weights_;



    };
}
#endif
