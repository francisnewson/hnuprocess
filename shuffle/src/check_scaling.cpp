#include "check_scaling.hh"
namespace fn
{
    Scan::Scan( std::vector<polygon_type> areas )
    {
        ac_ = make_unique<AreaCut>( areas );

        h_m2m_kmu_ = makeTH1D( "h_m2m_kmu", "m^{2}_{miss}",  1000, -0.7, 0.3, 
                "m^{2}_{miss} GeV^{2}/c^{4}" );

        h_t_z_ = makeTH2D( "h_t_z", "t_z",  
                150, 0, 0.015,  "t",
                120, -2000, 10000,  "z" );
    }

    void Scan::process_event( Km2EventData & km2e )
    {
        bool allowed =  ac_->allowed( { km2e.z, km2e.t} );

        if ( allowed 
                && km2e.muv_wgt > 0.5 
                && km2e.m2m < -0.1  
                && km2e.cda > 0.0 
                && km2e.cda < 4.0 
                && km2e.p > 10 )
        {
            h_m2m_kmu_->Fill( km2e.m2m, km2e.event_wgt ) ;
            h_t_z_->Fill( km2e.t, km2e.z, km2e.event_wgt );
        }
    }

    //--------------------------------------------------

    Plotter::Plotter()
    {
        h_m2m_kmu_ = makeTH1D( "h_m2m_kmu", "m^{2}_{miss}",  1000, -0.7, 0.3, 
                "m^{2}_{miss} GeV^{2}/c^{4}" );
    }

    void Plotter::process_event( Km2EventData & km2e )
    {
        h_m2m_kmu_->Fill( km2e.m2m, km2e.event_wgt ) ;
    }

    //--------------------------------------------------

    Scanner::Scanner( Km2Cut& sel, std::vector<std::pair<Km2Cut*, Km2Listener*>>& scans )
        :sel_( sel), scans_( scans ){}

    void Scanner::process_event( Km2EventData & km2e )
    {
        if ( sel_.check( km2e ) )
        {
            for ( const auto& scan : scans_ )
            {
                if (scan.first->check( km2e ))
                {
                    scan.second->process_event( km2e );
                }
            }

        }

    }

    //--------------------------------------------------

    polygon_type get_polygon( std::vector<point_type> points )
    {
        points.push_back( points.at( 0 ) );
        polygon_type this_polygon;
        this_polygon.outer() = fn::polygon_type::ring_type( begin( points ), end( points ) );
        return this_polygon;
    }

    void shift_area_first( std::vector<point_type>& points, double  delta )
    {
        for (  auto& point : points ) { point.x( point.x() + delta ) ; }
    }

    void shift_area_second( std::vector<point_type>& points, double  delta )
    {
        for (  auto& point : points ) { point.y( point.y() + delta ) ; }
    }

    void scan_region( TTree& tt, std::vector<Km2Listener*> listeners )
    {
        Km2EventData  * km2e = new Km2EventData;
        tt.SetBranchAddress( "Km2EventData", &km2e );
        Long64_t tt_entries = tt.GetEntries();
        std::cout << "Looping over " << tt_entries << " entries.\n";

        ez::ezETAProgressBar prog( tt_entries );
        prog.start();

        for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
        {
            ++prog;
            tt.GetEntry(ient);

                for ( const auto& listener : listeners )
                {
                    listener->process_event( *km2e );
                }
        }
    }
}
