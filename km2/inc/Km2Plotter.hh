#ifndef KM2PLOTTER_HH
#define KM2PLOTTER_HH
#include "Analysis.hh"
#include "Km2Reco.hh"
#include "HistStore.hh"


#if 0
/*
 *  _  __          ____  ____  _       _   _            
 * | |/ /_ __ ___ |___ \|  _ \| | ___ | |_| |_ ___ _ __ 
 * | ' /| '_ ` _ \  __) | |_) | |/ _ \| __| __/ _ \ '__|
 * | . \| | | | | |/ __/|  __/| | (_) | |_| ||  __/ |   
 * |_|\_\_| |_| |_|_____|_|   |_|\___/ \__|\__\___|_|   
 *                                                      
 * 
 */
#endif

class TFile;
class TH1D;

namespace fn
{
    class HistStore;

    class Km2Plotter : public Analysis
    {
        public:
            Km2Plotter( const Selection& sel, 
                    TFile& tfile, std::string folder,
                    const Km2Event& km2_event);

            void end_processing();

        private:
            void process_event();

            TH1D * h_m2miss_;
            TH1D * h_m2pimiss_;

            TH2D * h_m2pivsk_;
            TH2D * h_pm2pi_;
            TH2D * h_pm2k_;

            TH2D * h_pz_;
            TH1D * h_cda_;
            HistStore hs_;

            TFile& tfile_;
            std::string folder_;
            const Km2Event& km2_event_;

            REG_DEC_SUB( Km2Plotter);
    };

    template<>
        Subscriber * create_subscriber<Km2Plotter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
