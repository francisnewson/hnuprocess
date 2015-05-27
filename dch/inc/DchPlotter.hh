#ifndef DCHPLOTTER_HH
#define DCHPLOTTER_HH
#include "HistStore.hh"
#include "FourMomComp.hh"
#include "Analysis.hh"
#include "SingleTrack.hh"
#if 0
/*
 *  ____       _     ____  _       _   _
 * |  _ \  ___| |__ |  _ \| | ___ | |_| |_ ___ _ __
 * | | | |/ __| '_ \| |_) | |/ _ \| __| __/ _ \ '__|
 * | |_| | (__| | | |  __/| | (_) | |_| ||  __/ |
 * |____/ \___|_| |_|_|   |_|\___/ \__|\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class K2piLkrData;
    class K2piDchData;
    class K2piMcData;
    class K2piEventData;

    class DchPlotter
    {
        public:
            DchPlotter( TFile& tf, std::string folder);

            void plot_data( K2piEventData& event_data,
                    K2piLkrData& lkr_data, K2piDchData& dch_data, 
                    double weight, bool mc, K2piMcData * mc_data  );

            void write();

        private:
            TFile& tf_;
            std::string folder_;

            HistStore dths_;
            TH1D * hm2pip_lkr_ ;

            FourMomComp lkr_dch_cmp_;
            FourMomComp uw_lkr_dch_cmp_;
            FourMomComp lkr_mc_cmp_;
            FourMomComp lkr_tk_mc_cmp_;
            FourMomComp mc_dch_cmp_;
            TH1D * hpt_;
            TH1D * hcda_;
            TH1D * hz_;
            TH2D * hm2_tx_;
            TH2D * hm2_ty_;
            TH1D * hdm2_pi0_;
    };

    //--------------------------------------------------

    class K2piEventPlotter
    {
        public:
            K2piEventPlotter( TFile& tf, std::string folder);

            void plot_data( K2piEventData& event_data,
                    K2piLkrData& lkr_data, K2piDchData& dch_data, 
                    double weight, bool mc, K2piMcData * mc_data  );

            void write();

        private:
            TFile& tf_;
            std::string folder_;

            HistStore dths_;
            TH1D * hchi2_ ;
            TH1D * heop_ ;
            TH2D * heop_p_ ;
            TH1D * hphoton_sep_ ;
            TH1D * htrack_cluster_sep_ ;
            TH1D * htrack_track_cluster_sep_ ;
            TH2D * htrack_cluster_E_sep_ ;
            TH1D * hmin_photon_radius_;

            TH1D * hevent_pt_;

            TH2D * hphoton_energy_;
            TH1D * hhigh_photon_energy_;
            TH1D * hlow_photon_energy_;
    };
    
    //--------------------------------------------------
    
    class DchSelection  : public Subscriber
    {
        public:
            template <class F >
                DchSelection( F f )
                :checker_( f ){}

        bool check_data( K2piDchData * dch_data, K2piLkrData * lkr_data );

        private:
        std::function<bool(K2piDchData*, K2piLkrData*)> checker_;
    };

    bool check_dch_selections( std::vector<DchSelection*> selections, 
            K2piDchData * dch_data, K2piLkrData * lkr_data );


    //--------------------------------------------------
    
    class DchAnalysis: public Analysis
    {
        public:
        DchAnalysis( Selection& sel, TFile& tf_, 
                std::string folder_, K2piEventData& k2pi_data,
                std::string lkr_data_source,
                bool is_mc );
        void end_processing();

        void add_dch_selection( DchSelection* dch_selection );

        private:
        void process_event();
        DchPlotter plots_;
        K2piEventPlotter event_plots_;
        K2piEventData& k2pi_data_;
        K2piLkrData * lkr_data_;
        bool is_mc_;

        int n_scatterers_;
        std::vector<DchPlotter> scatter_plots_;
        std::vector<TrackPowerScatterer> scatterers_;
        std::vector<DchSelection*> dch_selections_;
    };
}
#endif
