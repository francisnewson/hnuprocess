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

    class Km2BasePlots
    {
        public:
        virtual void Fill( const Km2RecoEvent& km2re, double wgt  ) = 0;
        virtual void Write() = 0;
        virtual ~Km2BasePlots(){}
    };

    //Class to do actual plotting
    class Km2Plots : public Km2BasePlots
    {
        public:
            Km2Plots();
            void Fill( const Km2RecoEvent& km2re, double wgt  );
            void Write();
            ~Km2Plots(){}

        private:
            TH1D * h_pk_;
            TH1D * h_wgt_;

            TH1D * h_m2m_kmu_;
            TH1D * h_m2m_kpi_;
            TH1D * h_m2m_pimu_;

            TH2D * h_p_m2m_kmu_;
            TH2D * h_p_m2m_kmu_inner_;
            TH2D * h_p_m2m_kpi_;
            TH2D * h_p_m2m_pimu_;

            TH1D * h_p_;

            TH1D * h_kick_;

            TH1D * h_cda_;
            TH1D * h_t_;
            TH1D * h_z_;
            TH1D * h_pt_;

            TH1D * h_track_time_;

            TH2D * h_pt_m2m_kmu_;
            TH2D * h_p_t_;
            TH2D * h_p_cda_;
            TH2D * h_p_z_;
            TH2D * h_z_t_;
            TH2D * h_z_cda_;
            TH2D * h_z_phi_;
            TH2D * h_t_cda_;
            TH2D * h_m2_cda_;
            TH2D * h_m2_t_;
            TH2D * h_m2_z_;

            TH2D * h_txty_;

            TH2D * h_pr_coll_;
            TH2D * h_xy_coll_;
            TH2D * h_xy_coll_fine_;
            TH2D * h_xy_DCH1_;
            TH1D * h_r_DCH1_;
            TH2D * h_xy_DCH4_;
            TH2D * h_xy_LKr_;
            TH2D * h_xy_MUV1_;

            HistStore hs_;
    };

    //--------------------------------------------------
    
    class Km2MiniPlots : public Km2BasePlots
    {
        public:
            Km2MiniPlots();
            void Fill( const Km2RecoEvent& km2re, double wgt  );
            void Write();
            ~Km2MiniPlots(){}

        private:
            TH1D * h_p_;
            TH1D * h_pnu_;
            TH1D * h_m2m_kmu_;
            TH1D * h_m2m_kpi_;
            HistStore hs_;
    };


    //--------------------------------------------------

    class Km2Plotter : public Analysis
    {
        public:
            Km2Plotter( const Selection& sel, 
                    TFile& tfile, std::string folder,
                    std::unique_ptr<Km2BasePlots> km2_plots,
                    const Km2Event& km2_event);

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const Km2Event& km2_event_;
            std::unique_ptr<Km2BasePlots> km2_plots_;

            REG_DEC_SUB( Km2Plotter);
    };

    template<>
        Subscriber * create_subscriber<Km2Plotter>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    class Km2PZTPlotter: public Analysis
    {
        public:
            Km2PZTPlotter( const Selection& sel, 
                    TFile& tfile, std::string folder,
                    const Km2Event& km2_event);

            void end_processing();

        private:
            void process_event();

            TFile& tfile_;
            std::string folder_;
            const Km2Event& km2_event_;
            PZTPlots pzt_plots_;

            REG_DEC_SUB( Km2PZTPlotter);
    };

    template<>
        Subscriber * create_subscriber<Km2PZTPlotter>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
