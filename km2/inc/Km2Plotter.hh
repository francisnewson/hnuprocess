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

    class PZTPlots
    {
        public:
            PZTPlots();
            void Fill( const Km2RecoEvent& km2re, double wgt  );
            void Write();

        private:
            std::vector<TH2D*> hists_;
            HistStore hs_;
    };

    //Class to do actual plotting
    class Km2Plots
    {
        public:
            Km2Plots();
            void Fill( const Km2RecoEvent& km2re, double wgt  );
            void Write();

        private:
            TH1D * h_pk_;

            TH1D * h_m2m_kmu_;
            TH1D * h_m2m_kpi_;
            TH1D * h_m2m_pimu_;

            TH2D * h_p_m2m_kmu_;
            TH2D * h_p_m2m_kpi_;
            TH2D * h_p_m2m_pimu_;

            TH1D * h_p_;

            TH1D * h_kick_;

            TH1D * h_cda_;
            TH1D * h_t_;
            TH1D * h_z_;
            TH1D * h_pt_;

            TH2D * h_p_t_;
            TH2D * h_m2_t_;
            TH2D * h_pz_;

            TH2D * h_xy_coll_;
            TH2D * h_xy_DCH1_;
            TH2D * h_xy_DCH4_;
            TH2D * h_xy_LKr_;
            TH2D * h_xy_MUV1_;

            HistStore hs_;

            PZTPlots pzt_plots_;
    };

    //--------------------------------------------------

    class Km2Plotter : public Analysis
    {
        public:
            Km2Plotter( const Selection& sel, 
                    TFile& tfile, std::string folder,
                    const Km2Event& km2_event);

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const Km2Event& km2_event_;
            Km2Plots km2_plots_;

            REG_DEC_SUB( Km2Plotter);
    };

    template<>
        Subscriber * create_subscriber<Km2Plotter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
