#ifndef KM2BREAKDOWN_HH
#define KM2BREAKDOWN_HH
#include "Analysis.hh"
#include "HistStore.hh"
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
    class SingleRecoTrack;

    class MiniPlot
    {
        public:
            virtual void fill( Km2Breakdown& km2b, double wgt ) = 0;
            virtual void set_name( std::string name ) = 0;
            virtual std::string get_name() = 0;
    };

    template<typename T> MiniPlot * createInstance( Km2Breakdown& km2b)
    { T * result = new T{}; result->init( km2b ); return result; }

    template<typename T> TH1D * hinit( Km2Breakdown& km2b ){ return 0; }
    template<typename T> double hfill( Km2Breakdown& km2b ){ return 0; }

    template <typename T>
        class Mini1DPlot : public MiniPlot
        {
            private:
                TH1D * h;

            public:
                void init(  Km2Breakdown& km2b)
                { h = hinit<T>(km2b ); }

                void fill(  Km2Breakdown& km2b,  double wgt )
                { h->Fill( hfill<T>( km2b), wgt ) ; }

                void set_name( std::string name )
                { h->SetName( name.c_str() ); }

                std::string get_name()
                { return h->GetName(); }
        };

    template<typename T> TH2D * h2init( Km2Breakdown& km2b ){ return 0; }
    template<typename T> std::pair<double,double> h2fill( Km2Breakdown& km2b )
    { return {0,0}; }

    template <typename T>
        class Mini2DPlot : public MiniPlot
        {
            private:
                TH2D * h;

            public:
                void init(  Km2Breakdown& km2b)
                { h = h2init<T>(km2b ); }

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

    //TRACK QUALITY
    struct mp_track_quality{};
    template <> TH1D * hinit<mp_track_quality>( Km2Breakdown& km2b );
    template <> double hfill<mp_track_quality>( Km2Breakdown& km2b );

    //CDA
    struct mp_cda{};
    template <> TH1D * hinit<mp_cda>( Km2Breakdown& km2b );
    template <> double hfill<mp_cda>( Km2Breakdown& km2b );

    //DCH1
    struct mp_r_dch1{};
    template <> TH1D * hinit<mp_r_dch1>( Km2Breakdown& km2b );
    template <> double hfill<mp_r_dch1>( Km2Breakdown& km2b );

    //DCH4
    struct mp_r_dch4{};
    template <> TH1D * hinit<mp_r_dch4>( Km2Breakdown& km2b );
    template <> double hfill<mp_r_dch4>( Km2Breakdown& km2b );

    //LKR
    struct mp_xy_lkr{};
    template <> TH2D * h2init<mp_xy_lkr>( Km2Breakdown& km2b );
    template <> std::pair<double,double> h2fill<mp_xy_lkr>( Km2Breakdown& km2b );

    //DCHT
    struct mp_dcht{};
    template <> TH1D * hinit<mp_dcht>( Km2Breakdown& km2b );
    template <> double hfill<mp_dcht>( Km2Breakdown& km2b );

    //CLUSTER DISTANCE
    struct mp_cluster_distance{};
    template <> TH1D * hinit<mp_dcht>( Km2Breakdown& km2b );
    template <> double hfill<mp_dcht>( Km2Breakdown& km2b );

    //--------------------------------------------------

    class Km2Breakdown : public Analysis
    {
        public:
            Km2Breakdown( const Selection& sel, const Km2Event& km2e, 
                    const Selection& good_track,
                    TFile& tf, std::string folder  );

            HistStore& get_hist_store(){ return hs_; }
            const SingleRecoTrack& get_single_reco_track();

            template <typename T>
            void register_plotter( std::string ) ;
            void add_selection( const Selection * s, 
                    std::vector<std::string>& plot_types, std::string prefix = "" );

            void end_processing();

        private:
            typedef std::map<std::string, MiniPlot*(*)(Km2Breakdown& km2b)> map_type;
            map_type  mini_plot_map_;

            const Km2Event& km2e_;
            const Km2RecoEvent * km2re_;
            const Selection& good_track_;

            void process_event();
            TFile& tf_;
            std::string folder_;

            std::vector<std::pair<const Selection*, std::vector<MiniPlot*>> > selection_plots_;
            std::vector<std::unique_ptr<MiniPlot>> plot_store_;

            HistStore hs_;

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
