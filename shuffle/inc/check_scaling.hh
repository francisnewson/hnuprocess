#ifndef CHECK_SCALING_HH
#define CHECK_SCALING_HH
#if 0
/*
 *       _               _                      _ _
 *   ___| |__   ___  ___| | __    ___  ___ __ _| (_)_ __   __ _
 *  / __| '_ \ / _ \/ __| |/ /   / __|/ __/ _` | | | '_ \ / _` |
 * | (__| | | |  __/ (__|   <    \__ \ (_| (_| | | | | | | (_| |
 *  \___|_| |_|\___|\___|_|\_\___|___/\___\__,_|_|_|_| |_|\__, |
 *                          |_____|                       |___/
 *
 */
#endif
#include "TFile.h"
#include "TTree.h"
#include "root_help.hh"
#include "stl_help.hh"
#include "AreaCut.hh"
#include "HistStore.hh"
#include "Km2EventData.hh"
#include "ezETAProgressBar.hh"
namespace fn
{

    class Km2Listener
    {
        public:
            virtual void process_event( Km2EventData& km2 ) = 0;
            virtual ~Km2Listener(){};
    };

    class Km2Cut
    {
        public:
            virtual bool check( Km2EventData& km2 ) = 0;
    };

    //--------------------------------------------------

    class Km2Lambda : public Km2Cut
    {
        public:
            template <typename f>
                Km2Lambda( f cut )
                :cut_( cut )
                {
                }

            bool check( Km2EventData& km2e ){ return cut_( km2e ); }

        private:
            std::function<bool(Km2EventData&)> cut_;
    };

    //--------------------------------------------------

    class Scan : public Km2Listener
    {
        public:
            Scan( std::vector<polygon_type> areas );
            void process_event( Km2EventData & km2e );
            void write() { h_m2m_kmu_->Write(); }

            void write( std::string name ) {
                h_m2m_kmu_->Write( name.c_str() );
                h_t_z_->Write( (name + "_tz" ).c_str() );
            }

            double integral()
            {
                return h_m2m_kmu_->Integral();
            }

        private:
            std::unique_ptr<AreaCut> ac_;
            std::unique_ptr<TH1D> h_m2m_kmu_;
            std::unique_ptr<TH2D> h_t_z_;
    };

    //--------------------------------------------------

    class Plotter : public Km2Listener
    {
        public:
            Plotter();
            void process_event( Km2EventData & km2e );
            double integral()
            {
                return h_m2m_kmu_->Integral();
            }

            void write( std::string name ) {
                h_m2m_kmu_->Write( Form( "%s_hm2m", name.c_str() ) );
            }

            TH1D * operator->(){ return h_m2m_kmu_.get() ;}
            TH1D * get_hist(){ return h_m2m_kmu_.get() ;}

        private:
            std::unique_ptr<TH1D> h_m2m_kmu_;
    };

    //--------------------------------------------------

    class Scanner : public Km2Listener
    {
        public:
            Scanner( Km2Cut& sel, std::vector<std::pair<Km2Cut*, Km2Listener*>>& scans );
            void process_event( Km2EventData & km2e );

        private:
            Km2Cut& sel_;
            std::vector<std::pair<Km2Cut*, Km2Listener*>>& scans_;
    };

    //--------------------------------------------------

    void scan_region( TTree& tt, std::vector<Km2Listener*> listeners );

    //--------------------------------------------------

    polygon_type get_polygon( std::vector<point_type> points );

    void shift_area_first( std::vector<point_type>& points, double  delta );

    void shift_area_second( std::vector<point_type>& points, double  delta );

    //--------------------------------------------------

    template <class f >
        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> generate_region_scan_cuts
        ( std::vector<point_type> raw_area, std::vector<double> deltas,
          f shift, double multiplier = 1 )
        {
            std::vector<std::pair<int,std::unique_ptr<Km2Cut>>> scan_cuts;

            for ( const auto delta : deltas )
            {
                auto  this_area = raw_area;
                shift( this_area, delta * multiplier  );
                polygon_type this_polygon = get_polygon( this_area );
                std::vector<polygon_type> polygons{ this_polygon };
                AreaCut ac ( polygons);

                auto this_cut = make_unique<Km2Lambda>( [ac]( Km2EventData & km2e )
                        { return ac.allowed( { km2e.z, km2e.t } ); } );

                scan_cuts.push_back(
                        std::make_pair(  static_cast<int>( delta ) , std::move( this_cut ) ) );
            }

            return scan_cuts;
        }

    void populate_scans(
            std::vector<std::pair<int, std::unique_ptr<Plotter>>>& plotters,
            std::vector<std::pair<Km2Cut*, Km2Listener*>>& scans,
            std::vector<std::pair<int, std::unique_ptr<Km2Cut>>>& reg_cuts
            )
    {
        for ( const auto& reg_cut : reg_cuts )
        {
            int delta = reg_cut.first;
            Km2Cut * cut = reg_cut.second.get();
            auto plotter = make_unique<Plotter>();
            scans.push_back( std::make_pair( cut, plotter.get() ) );
            plotters.push_back( std::make_pair( delta,  std::move( plotter ) ) ) ;
        }
    }
}
#endif
