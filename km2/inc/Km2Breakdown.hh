#ifndef KM2BREAKDOWN_HH
#define KM2BREAKDOWN_HH
#include "Analysis.hh"
#include "HistStore.hh"
#include <boost/optional.hpp>
#if 0
/*
 *  _  __          ____  ____                 _       _
 * | |/ /_ __ ___ |___ \| __ ) _ __ ___  __ _| | ____| | _____      ___ __
 * | ' /| '_ ` _ \  __) |  _ \| '__/ _ \/ _` | |/ / _` |/ _ \ \ /\ / / '_ \
 * | . \| | | | | |/ __/| |_) | | |  __/ (_| |   < (_| | (_) \ V  V /| | | |
 * |_|\_\_| |_| |_|_____|____/|_|  \___|\__,_|_|\_\__,_|\___/ \_/\_/ |_| |_|
 *
 *
 */
#endif
namespace fn
{
    class Km2Breakdown;
    class Km2Event;
    class Km2RecoEvent;
    class Km2RecoClusters;
    class Km2Clusters;
    class ClusterCorrector;
    class SingleRecoTrack;
    class SingleMuon;

    class MiniPlot
    {
        public:
            virtual void fill( Km2Breakdown& km2b, double wgt ) = 0;
            virtual void set_name( std::string name ) = 0;
            virtual std::string get_name() = 0;
    };

    template<typename T> MiniPlot * createInstance( HistStore& hs)
    { T * result = new T{}; result->init( hs ); return result; }

    template<typename T> TH1D * hinit( HistStore& hs ){ return 0; }
    template<typename T> double hfill( Km2Breakdown& km2b ){ return 0; }

    template <typename T>
        class Mini1DPlot : public MiniPlot
    {
        private:
            TH1D * h;

        public:
            void init( HistStore& hs )
            { h = hinit<T>(hs ); }

            void fill(  Km2Breakdown& km2b,  double wgt )
            { h->Fill( hfill<T>( km2b), wgt ) ; }

            void set_name( std::string name )
            { h->SetName( name.c_str() ); }

            std::string get_name()
            { return h->GetName(); }
    };

    //--------------------------------------------------
    
    template<typename T> boost::optional<double> hoptfill( Km2Breakdown& km2b )
    { return boost::optional<double>{} ; }

    template <typename T>
        class Mini1DPlotOpt : public MiniPlot
    {
        private:
            TH1D * h;

        public:
            void init( HistStore& hs )
            { h = hinit<T>(hs ); }

            void fill(  Km2Breakdown& km2b,  double wgt )
            { 
                boost::optional<double> result = hoptfill<T>( km2b );
                if ( result )
                {
                h->Fill( *result, wgt ) ;
                }
            }

            void set_name( std::string name )
            { h->SetName( name.c_str() ); }

            std::string get_name()
            { return h->GetName(); }
    };

    //--------------------------------------------------

    template<typename T> TH2D * h2init( HistStore& hs ){ return 0; }
    template<typename T> std::pair<double,double> h2fill( Km2Breakdown& km2b )
    { return {0,0}; }

    template <typename T>
        class Mini2DPlot : public MiniPlot
    {
        private:
            TH2D * h;

        public:
            void init( HistStore& hs )
            { h = h2init<T>(hs ); }

            void fill(  Km2Breakdown& km2b,  double wgt )
            {
                std::pair<double,double>  result = h2fill<T>( km2b );
                h->Fill( result.first, result.second, wgt ) ;
            }

            void set_name( std::string name )
            { h->SetName( name.c_str() ); }

            std::string get_name()
            { return h->GetName(); }
    };

    //--------------------------------------------------

    template<typename T> 
        boost::optional<std::pair<double,double>> h2optfill( Km2Breakdown& km2b )
    { return boost::optional<std::pair<double,double>>{}  ; }

    template <typename T>
        class Mini2DPlotOpt : public MiniPlot
    {
        private:
            TH2D * h;

        public:
            void init( HistStore& hs )
            { h = h2init<T>(hs ); }

            void fill(  Km2Breakdown& km2b,  double wgt )
            {
                boost::optional<std::pair<double,double>>  result = h2optfill<T>( km2b );
                if ( result )
                {
                h->Fill( result->first, result->second, wgt ) ;
                }
            }

            void set_name( std::string name )
            { h->SetName( name.c_str() ); }

            std::string get_name()
            { return h->GetName(); }
    };


    //--------------------------------------------------
    
    //M2M 
    struct mp_m2m{};
    template <> TH1D * hinit<mp_m2m>( HistStore& hs );
    template <> double hfill<mp_m2m>( Km2Breakdown& km2b );

    //TRACK MOM
    struct mp_mom{};
    template <> TH1D * hinit<mp_mom>( HistStore& hs );
    template <> double hfill<mp_mom>( Km2Breakdown& km2b );

    //TRACK QUALITY
    struct mp_track_quality{};
    template <> TH1D * hinit<mp_track_quality>( HistStore& hs );
    template <> double hfill<mp_track_quality>( Km2Breakdown& km2b );

    //CDA
    struct mp_cda{};
    template <> TH1D * hinit<mp_cda>( HistStore& hs );
    template <> double hfill<mp_cda>( Km2Breakdown& km2b );

    //DCH1
    struct mp_r_dch1{};
    template <> TH1D * hinit<mp_r_dch1>( HistStore& hs );
    template <> double hfill<mp_r_dch1>( Km2Breakdown& km2b );

    //DCH4
    struct mp_r_dch4{};
    template <> TH1D * hinit<mp_r_dch4>( HistStore& hs );
    template <> double hfill<mp_r_dch4>( Km2Breakdown& km2b );

    //LKR
    struct mp_xy_lkr{};
    template <> TH2D * h2init<mp_xy_lkr>( HistStore& hs );
    template <> std::pair<double,double> h2fill<mp_xy_lkr>( Km2Breakdown& km2b );

    //MUV
    struct mp_xy_muv{};
    template <> TH2D * h2init<mp_xy_muv>( HistStore& hs );
    template <> std::pair<double,double> h2fill<mp_xy_muv>( Km2Breakdown& km2b );

    //DCHT
    struct mp_dcht{};
    template <> TH1D * hinit<mp_dcht>( HistStore& hs );
    template <> double hfill<mp_dcht>( Km2Breakdown& km2b );

    //CLUSTER DISTANCE
    class ClusterMiniPlot : public MiniPlot
    {
        public:
            void init( HistStore& hs );
            void fill( Km2Breakdown& km2b, double wgt );
            void set_name( std::string name );
            std::string get_name();

        private:
            TH1D * h_ds_;
            TH1D * h_E_;
            TH2D * h_ds_E_;
            TH2D * h_ds_t_;
    };

    //ZT
    struct mp_zt{};
    template <> TH2D * h2init<mp_zt>( HistStore& hs );
    template <> std::pair<double,double> h2fill<mp_zt>( Km2Breakdown& km2b );

    //PT
    struct mp_pt{};
    template <> TH2D * h2init<mp_pt>( HistStore& hs );
    template <> std::pair<double,double> h2fill<mp_pt>( Km2Breakdown& km2b );

    //TPHI
    struct mp_tphi{};
    template <> TH2D * h2init<mp_tphi>( HistStore& hs );
    template <> std::pair<double,double> h2fill<mp_tphi>( Km2Breakdown& km2b );

    //MUON
    struct mp_muon{};
    template <> TH2D * h2init<mp_muon>( HistStore& hs );
    template <> boost::optional<std::pair<double,double>> h2optfill<mp_muon>( Km2Breakdown& km2b );

    struct mp_muonr{};
    template <> TH1D * hinit<mp_muonr>( HistStore& hs );
    template <>boost::optional<double> hoptfill<mp_muonr>( Km2Breakdown& km2b );

    //--------------------------------------------------

    typedef std::map<std::string, MiniPlot*(*)(HistStore& hs)> MiniPlotMap;

    class SelPlot
    {
        public:
            SelPlot( TFile& tf, std::string folder, MiniPlotMap& mpm  );
            void AddMiniPlot( std::string plot_type );
            void Fill( Km2Breakdown& km2b, double pre_wgt, bool passed, double post_wgt );
            void Write();

        private:
            TFile& tf_;
            std::string folder_;
            MiniPlotMap& mpm_;

            std::vector<std::unique_ptr<MiniPlot>> plot_store_;

            HistStore pre_hs_;
            std::vector<MiniPlot*> pre_;

            HistStore post_hs_;
            std::vector<MiniPlot*> post_;
    };


    class Km2Breakdown : public Analysis
    {
        public:
            Km2Breakdown( const Selection& sel, const Km2Event& km2e, 
                    const Selection& good_track, Km2Clusters& km2c,
                    const SingleMuon& sm,
                    TFile& tf, std::string folder  );

            const SingleRecoTrack& get_single_reco_track();
            const Km2RecoClusters& get_reco_clusters();
            const Km2RecoEvent& get_reco_event();
            const SingleMuon& get_single_muon();

            template <typename T>
                void register_plotter( std::string ) ;
            void add_selection( const Selection * s, 
                    std::vector<std::string>& plot_types, std::string prefix = "" );

            void end_processing();

        private:
            typedef std::map<std::string, MiniPlot*(*)(HistStore& hs)> map_type;
            map_type  mini_plot_map_;

            const Km2Event& km2e_;
            const Km2RecoEvent * km2re_;
            const Selection& good_track_;

            const Km2Clusters& km2c_;
            const Km2RecoClusters * km2rc_;

            const SingleMuon&  sm_;

            void process_event();
            TFile& tf_;
            std::string folder_;

            std::vector<std::pair<const Selection*, SelPlot* >> sel_plots_;
            std::vector<std::unique_ptr<SelPlot>> sel_plot_store_;

            REG_DEC_SUB( Km2Breakdown );
    };

    template <typename T>
        void Km2Breakdown::register_plotter( std::string name ) 
        {
            mini_plot_map_.insert( std::make_pair( name, createInstance<T> ) );
        }

    template<>
        Subscriber * create_subscriber<Km2Breakdown>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
