#include "easy_app.hh"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "root_help.hh"
#include "stl_help.hh"
#include "AreaCut.hh"
#include "HistStore.hh"
#include "Km2EventData.hh"
#include "ezETAProgressBar.hh"
#include "TEfficiency.h"
#include "check_scaling.hh"




//--------------------------------------------------

template <class f >
    std::vector<std::pair<int, std::unique_ptr<Scan>>> generate_scans
( std::vector<point_type> raw_area, std::vector<double> deltas, f shift, double multiplier = 1 )
{
    std::vector<std::pair<int,std::unique_ptr<Scan>>> scans;

    for ( const auto delta : deltas )
    {
        auto  this_area = raw_area;
        shift( this_area, delta * multiplier  );
        polygon_type this_polygon;
        this_polygon.outer() = fn::polygon_type::ring_type( begin( this_area ), end( this_area ) );
        auto this_scan = make_unique<Scan>( std::vector<polygon_type>{this_polygon} );
        scans.push_back( std::make_pair(  static_cast<int>( delta ) , std::move( this_scan ) ) );
    }

    return scans;
}

//--------------------------------------------------

std::vector<std::pair<int, std::unique_ptr<Scan>>> get_lower_scans()
{
    std::vector<point_type> raw_area
    {  {  -950 ,  0.0200 }  , {  -950 ,  0.0110 }  ,
        {  -825 ,  0.0085 }  , {  -325 ,  0.0060 }  ,
        {   500 ,  0.0040 }  , {  1500 ,  0.0030 }  ,
        {  2575 ,  0.0025 }  , {  2900 ,  0.0016 }  ,
        {  3000 ,  0.0000 }  , { 10000 ,  0.0000 }  ,
        { 10000 ,  0.0200 }  , {  -950 ,  0.0200 }  };

    std::vector<double> deltas { -500, -400, -300, -200, -100, 0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };

    std::vector<std::pair<int,std::unique_ptr<Scan>>> scans = generate_scans
        ( raw_area, deltas, shift_area_first );

    return scans;
}

std::vector<std::pair<int, std::unique_ptr<Scan>>> get_upper_scans()
{
    std::vector<point_type> raw_area {
        {  8000,  0.000 } , {  8000,  0.004 } ,
            {  7000,  0.0045 }  ,
            {  5000,  0.006 } , {  3500,  0.008 }  ,
            {  2000,  0.010 } , {     0,  0.010 } ,
            {     0,  0.000 } , {  8000,  0.000 }  } ;

    std::vector<double> deltas { -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };

    std::vector<std::pair<int,std::unique_ptr<Scan>>> scans = generate_scans
        ( raw_area, deltas, shift_area_second, 0.0001 );

    return scans;
}


std::vector<std::pair<int, std::unique_ptr<Scan>>> get_scans( std::string region )
{
    if ( region == "upper" )
    { return get_upper_scans() ;  }
    else if ( region == "lower" )
    { return get_lower_scans() ;  }
    else
    {
        std::cout << "Uknown region : " << region << std::endl;
        throw( std::out_of_range( region ) );
    }
}

//--------------------------------------------------

void run_k2m( TTree& tt, std::vector<Km2Listener*> listeners )
{
    Km2EventData  * km2e = new Km2EventData;
    tt.SetBranchAddress( "Km2EventData", &km2e );

    Long64_t tt_entries = tt.GetEntries();
    ez::ezETAProgressBar prog( tt_entries );
    prog.start();

    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    {
        ++prog;

        tt.GetEntry(ient);
        for ( auto& listener : listeners )
        {
            listener->process_event( *km2e );
        }
    }
}

//--------------------------------------------------

    std::pair<std::unique_ptr<TTree>, std::unique_ptr<TTree> > 
get_dt_hl_trees( TFile& tf, std::string pol, std::string region )
{
    auto dt_tree = std::unique_ptr<TTree>( get_object<TTree>( tf , 
                Form( "p5.data.q11t.v3.%s/km2_trees/km2_%s", pol.c_str(), region.c_str() ) )  );

    auto hl_tree = std::unique_ptr<TTree>( get_object<TTree>( tf ,
                Form("p6.data.km.q11t.v2.%s/km2_trees/km2_%s", pol.c_str(), region.c_str() ) ));

    dt_tree->SetCacheSize(10000000);
    dt_tree->AddBranchToCache("*");

    hl_tree->SetCacheSize(10000000);
    hl_tree->AddBranchToCache("*");

    return std::make_pair( std::move(dt_tree), std::move(hl_tree ));
}

//--------------------------------------------------

void process_scans( TFile& tfout , std::ostream & os, 
        std::vector<std::pair<int, std::unique_ptr<Plotter>>>& dt_scans,
        std::vector<std::pair<int, std::unique_ptr<Plotter>>>& hl_scans,
        std::string name )
{
    cd_p( &tfout, name + "/hdt" );
    for( const auto & scan : dt_scans )
    {
        scan.second->write( std::to_string( scan.first ) );
    }

    cd_p( &tfout, name + "/hhl" );
    for( const auto & scan : hl_scans )
    {
        scan.second->write( std::to_string( scan.first ) );
    }

    os << "scan  " << name << "\n";
    os
        << std::setw(15) << "shift" << std::setw(15) << "dt"
        << std::setw(15) << "hl" << std::setw(15)  << "scale"
        << std::setw(15)  << "err" 
        << std::setw(15)  << "fitscale" 
        << std::setw(15)  << "fitscalerr" 
        << std::setw(15)  << "fitchi2" 
        << "\n";

    for ( const auto& scan : zip( dt_scans, hl_scans ) )
    {
        const std::pair<int, std::unique_ptr<Plotter>>& dtscan = scan.get<0>();
        const std::pair<int, std::unique_ptr<Plotter>>&  hlscan = scan.get<1>();
        double dt_total =  dtscan.second->integral();
        double hl_total =  hlscan.second->integral();
        double scale_factor = dt_total / hl_total;
        double scale_err = scale_factor * ( std::sqrt( 1 / dt_total + 1 / hl_total ) );

        dtscan.second->get_hist()->Rebin(50);
        hlscan.second->get_hist()->Rebin(50);
        dtscan.second->get_hist()->Sumw2();
        hlscan.second->get_hist()->Sumw2();
        dtscan.second->get_hist()->Divide( hlscan.second->get_hist() );
        TF1 fit( "my_fit", "pol0", -1, 1 );
        dtscan.second->get_hist()->Fit( "my_fit" );

        cd_p( &tfout, name + "/hrat" );
        dtscan.second->get_hist()->Write( std::to_string( dtscan.first ).c_str() );

        os
            << std::setw(15) << dtscan.first
            << std::setw(15) << dt_total
            << std::setw(15) << hl_total
            << std::setw(15)  << scale_factor
            << std::setw(15)  << scale_err
            << std::setw(15)  << fit.GetParameter(0)
            << std::setw(15)  << fit.GetParError(0)
            << std::setw(15)  << fit.GetChisquare()
            << "\n";
    }
}


//--------------------------------------------------

void run_scan( TFile& tfin, TFile& tfout, std::string pol, std::string region, std::string name )
{
    auto dt_hl = get_dt_hl_trees( tfin, pol, region );
    auto & dt_tree = dt_hl.first;
    auto & hl_tree = dt_hl.second;

    std::vector<Km2Listener*> dt_listeners;
    std::vector<std::pair<int, std::unique_ptr<Scan>>> dt_scans = get_scans( region);
    for ( const auto& scan : dt_scans )
    {
        dt_listeners.push_back( scan.second.get() );
    }

    run_k2m( *dt_tree,  dt_listeners );

    std::vector<Km2Listener*> hl_listeners;
    std::vector<std::pair<int, std::unique_ptr<Scan>>> hl_scans = get_scans( region );
    for ( const auto& scan : hl_scans )
    {
        hl_listeners.push_back( scan.second.get() );
    }

    run_k2m( *hl_tree,  hl_listeners );

    //process_scans( tfout, dt_scans, hl_scans, name );
}

//--------------------------------------------------

int main()
{

    TFile tfin{ "tdata/filter/all.filter.root" };
    TFile tfout{ "output/scan.root", "RECREATE" };

    //for each region
    for(  auto pol : { "pos", "neg" } )
    {
        std::string region = "upper";
        auto dt_hl = get_dt_hl_trees( tfin, pol, region );
        auto & dt_tree = dt_hl.first;
        auto & hl_tree = dt_hl.second;

        //prepare selections

        //global cut
        auto global_select = []( Km2EventData& km2e )
        {
            return (km2e.muv_wgt > 0.5 
                    && km2e.m2m < -0.07
                    && km2e.m2m > -0.25  
                    && km2e.cda > 0.0 
                    && km2e.cda < 3.5 
                    && km2e.p > 10 );
        }; 

        Km2Lambda global_cut{ global_select };

        //Area cuts
        std::vector<point_type> raw_upper_area {
            {  8000,  0.000 } , {  8000,  0.004 } , {  7000,  0.0045 }  ,
                {  5000,  0.006 } , {  3500,  0.008 } , {  2000,  0.010 } , 
                {     0,  0.010 } , {     0,  0.000 } , {  8000,  0.000 }  } ;

        std::vector<double> deltas
        { -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30 };

        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> reg_cuts
            = generate_region_scan_cuts( raw_upper_area, deltas, shift_area_second, 0.0001 );

        //CDA scan prep
        auto cda_zt_polygon = std::vector<polygon_type>{ get_polygon( raw_upper_area ) };
        AreaCut cda_ac{ cda_zt_polygon };
        auto global_cda_select = [cda_ac]( Km2EventData& km2e )
        {
            return (km2e.muv_wgt > 0.5 
                    && km2e.m2m < -0.07
                    && km2e.m2m > -0.25  
                    && km2e.cda > 0.0 
                    && km2e.p > 10 
                    && cda_ac.allowed( {km2e.z, km2e.t} ) );
        };

        Km2Lambda global_cda_cut{ global_cda_select };

        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> cda_cuts;
        for ( int cda  : { 25, 26, 27, 28, 29, 30,
                31, 32, 33, 34, 35,
                36, 37, 38, 39, 40 } )
        {
            auto cda_cut = [cda]( Km2EventData& km2e ){ return km2e.cda < 0.1 *cda ; };
            cda_cuts.push_back( std::make_pair( cda, make_unique<Km2Lambda>(cda_cut ) ) );
        }

        //RK PZ scan prep
        std::vector<rectangle> rk_pz_rectangles
        {
            { 13, 20, -1000, 9000 } ,
            { 20, 25, -1000, 9000 } ,
            { 25, 30,     0, 9000 } ,
            { 30, 35,   500, 9000 } ,
            { 35, 40,   500, 9000 } ,
            { 40, 45,   500, 9000 } ,
            { 45, 50,  1000, 9000 } ,
            { 50, 55,  1000, 9000 } ,
            { 55, 60,  1500, 9000 } ,
            { 60, 65,  1500, 9000 }
        };

        AreaCut rk_pz_area{ rk_pz_rectangles };
        auto global_rk_select = [rk_pz_area]( Km2EventData& km2e )
        { return rk_pz_area.allowed( {km2e.p, km2e.z } ); };

        Km2Lambda global_rk_cut{ global_rk_select };

        //Data scan
        std::vector<std::pair<int, std::unique_ptr<Plotter>>> data_zt_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_zt_scans;
        populate_scans( data_zt_plotters, data_zt_scans, reg_cuts );
        Scanner data_upper_zt_scanner{ global_cut, data_zt_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> data_cda_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_cda_scans;
        populate_scans( data_cda_plotters, data_cda_scans, cda_cuts );
        Scanner data_upper_cda_scanner{ global_cda_cut, data_cda_scans };

        Plotter data_rk_plotter;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_rk_scans{ { &global_rk_cut, &data_rk_plotter } };
        Scanner data_rk_scanner{ global_rk_cut, data_rk_scans } ;

        std::vector<Km2Listener*> data_listeners(
                {  &data_upper_zt_scanner, &data_upper_cda_scanner, &data_rk_scanner } );

        scan_region( *dt_tree, data_listeners );

        //Halo scan
        std::vector<std::pair<int, std::unique_ptr<Plotter>>> halo_zt_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_zt_scans;
        populate_scans( halo_zt_plotters, halo_zt_scans, reg_cuts );
        Scanner halo_upper_zt_scanner{ global_cut, halo_zt_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> halo_cda_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_cda_scans;
        populate_scans( halo_cda_plotters, halo_cda_scans, cda_cuts );
        Scanner halo_upper_cda_scanner{ global_cda_cut, halo_cda_scans };

        Plotter halo_rk_plotter;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_rk_scans{ { &global_rk_cut, &halo_rk_plotter } };
        Scanner halo_rk_scanner{ global_rk_cut, halo_rk_scans } ;

        std::vector<Km2Listener*> halo_listeners(
                {  &halo_upper_zt_scanner, &halo_upper_cda_scanner, &halo_rk_scanner} );

        scan_region( *hl_tree, halo_listeners );

        //Combine
        std::ofstream of_uzt(Form( "output/scaling/%s_upper_zt.dat", pol));
        process_scans( tfout, of_uzt, data_zt_plotters, halo_zt_plotters, Form("%s_upper_zt", pol ) );

        std::ofstream of_ucda(Form( "output/scaling/%s_upper_cda.dat", pol));
        process_scans( tfout, of_ucda,  data_cda_plotters, halo_cda_plotters, Form("%s_upper_cda", pol ) );

        std::cout << Form( "%s_upper_rk", pol ) << std::endl;
        std::cout
            << std::setw(10) <<  data_rk_plotter.integral() 
            << std::setw(10)  << halo_rk_plotter.integral() 
            << std::setw(10)  << data_rk_plotter.integral()  / halo_rk_plotter.integral() 
            <<  std::endl;
    }

    for(  auto pol : { "pos", "neg" } )
    {
        std::string region = "lower";
        auto dt_hl = get_dt_hl_trees( tfin, pol, region );
        auto & dt_tree = dt_hl.first;
        auto & hl_tree = dt_hl.second;

        //prepare selections

        //global cut
        auto global_select = []( Km2EventData& km2e )
        {
            return (km2e.muv_wgt > 0.5 
                    && km2e.m2m < -0.07
                    && km2e.m2m > -0.25  
                    && km2e.cda > 0.0 
                    && km2e.cda < 3.5 
                    && km2e.p > 10 );
        }; 

        Km2Lambda global_cut{ global_select };

        //Area cuts
        std::vector<point_type> raw_lower_area
        {  {  -950 ,  0.0200 }  , {  -950 ,  0.0110 }  ,
            {  -825 ,  0.0085 }  , {  -325 ,  0.0060 }  ,
            {   500 ,  0.0040 }  , {  1500 ,  0.0030 }  ,
            {  2575 ,  0.0025 }  , {  2900 ,  0.0016 }  ,
            {  3000 ,  0.0000 }  , { 10000 ,  0.0000 }  ,
            { 10000 ,  0.0200 }  , {  -950 ,  0.0200 }  };


        std::vector<double> deltas
        { -500, -400, -300, -200, -100, 0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };

        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> reg_cuts
            = generate_region_scan_cuts( raw_lower_area, deltas, shift_area_first, -1.0 );

        //CDA scan prep
        auto cda_zt_polygon = std::vector<polygon_type>{ get_polygon( raw_lower_area ) };
        AreaCut cda_ac{ cda_zt_polygon };
        auto global_cda_select = [cda_ac]( Km2EventData& km2e )
        {
            return (km2e.muv_wgt > 0.5 
                    && km2e.m2m < -0.07
                    && km2e.m2m > -0.25  
                    && km2e.cda > 0.0 
                    && km2e.p > 10 
                    && cda_ac.allowed( {km2e.z, km2e.t} ) );
        };

        Km2Lambda global_cda_cut{ global_cda_select };

        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> cda_cuts;
        for ( int cda  : { 25, 26, 27, 28, 29, 30,
                31, 32, 33, 34, 35,
                36, 37, 38, 39, 40 } )
        {
            auto cda_cut = [cda]( Km2EventData& km2e ){ return km2e.cda < 0.1 *cda ; };
            cda_cuts.push_back( std::make_pair( cda, make_unique<Km2Lambda>(cda_cut ) ) );
        }

        //M2m scan prep
        auto global_m2m_select = [cda_ac]( Km2EventData& km2e )
        {
            return (km2e.muv_wgt > 0.5 
                    && km2e.cda > 0.0 
                    && km2e.cda < 3.0 
                    && km2e.p > 10 
                    && cda_ac.allowed( {km2e.z, km2e.t} ) );
        };

        Km2Lambda global_m2m_cut{ global_m2m_select };

        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> m2m_cuts;
        for ( int m2m  : { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 } )
        {
            auto m2m_cut = [m2m]( Km2EventData& km2e ){ return km2e.m2m < - 0.01 *m2m ; };
            m2m_cuts.push_back( std::make_pair( m2m, make_unique<Km2Lambda>(m2m_cut ) ) );
        }

        auto global_min_m2m_select = [cda_ac]( Km2EventData& km2e )
        {
            return (km2e.muv_wgt > 0.5 
                    && km2e.m2m < -0.07  
                    && km2e.cda > 0.0 
                    && km2e.cda < 3.0 
                    && km2e.p > 10 
                    && cda_ac.allowed( {km2e.z, km2e.t} ) );
        };

        Km2Lambda global_min_m2m_cut{ global_m2m_select };

        std::vector<std::pair<int, std::unique_ptr<Km2Cut>>> min_m2m_cuts;
        for ( int m2m  : { 20, 25, 30, 35, 40, 45, 50, 55, 60 } )
        {
            auto m2m_cut = [m2m]( Km2EventData& km2e ){ return km2e.m2m > - 0.01 *m2m ; };
            min_m2m_cuts.push_back( std::make_pair( m2m, make_unique<Km2Lambda>(m2m_cut ) ) );
        }

        //RK PZ scan prep
        std::vector<rectangle> rk_pz_rectangles
        {
            { 13, 20, -1000, 9000 } ,
                { 20, 25, -1000, 9000 } ,
                { 25, 30,     0, 9000 } ,
                { 30, 35,   500, 9000 } ,
                { 35, 40,   500, 9000 } ,
                { 40, 45,   500, 9000 } ,
                { 45, 50,  1000, 9000 } ,
                { 50, 55,  1000, 9000 } ,
                { 55, 60,  1500, 9000 } ,
                { 60, 65,  1500, 9000 }
        };

        AreaCut rk_pz_area{ rk_pz_rectangles };
        auto global_rk_select = [rk_pz_area]( Km2EventData& km2e )
        { return rk_pz_area.allowed( {km2e.p, km2e.z } ); };

        Km2Lambda global_rk_cut{ global_rk_select };

        //Data scan
        std::vector<std::pair<int, std::unique_ptr<Plotter>>> data_zt_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_zt_scans;
        populate_scans( data_zt_plotters, data_zt_scans, reg_cuts );
        Scanner data_lower_zt_scanner{ global_cut, data_zt_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> data_cda_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_cda_scans;
        populate_scans( data_cda_plotters, data_cda_scans, cda_cuts );
        Scanner data_lower_cda_scanner{ global_cda_cut, data_cda_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> data_m2m_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_m2m_scans;
        populate_scans( data_m2m_plotters, data_m2m_scans, m2m_cuts );
        Scanner data_lower_m2m_scanner{ global_m2m_cut, data_m2m_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> data_min_m2m_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_min_m2m_scans;
        populate_scans( data_min_m2m_plotters, data_min_m2m_scans, min_m2m_cuts );
        Scanner data_lower_min_m2m_scanner{ global_min_m2m_cut, data_min_m2m_scans };

        Plotter data_rk_plotter;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> data_rk_scans{ { &global_rk_cut, &data_rk_plotter } };
        Scanner data_rk_scanner{ global_rk_cut, data_rk_scans } ;

        std::vector<Km2Listener*> data_listeners(
                {  &data_lower_zt_scanner, &data_lower_cda_scanner,
                &data_lower_m2m_scanner, &data_lower_min_m2m_scanner, &data_rk_scanner} );

        scan_region( *dt_tree, data_listeners );

        //Halo scan
        std::vector<std::pair<int, std::unique_ptr<Plotter>>> halo_zt_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_zt_scans;
        populate_scans( halo_zt_plotters, halo_zt_scans, reg_cuts );
        Scanner halo_lower_zt_scanner{ global_cut, halo_zt_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> halo_cda_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_cda_scans;
        populate_scans( halo_cda_plotters, halo_cda_scans, cda_cuts );
        Scanner halo_lower_cda_scanner{ global_cda_cut, halo_cda_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> halo_m2m_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_m2m_scans;
        populate_scans( halo_m2m_plotters, halo_m2m_scans, m2m_cuts );
        Scanner halo_lower_m2m_scanner{ global_m2m_cut, halo_m2m_scans };

        std::vector<std::pair<int, std::unique_ptr<Plotter>>> halo_min_m2m_plotters;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_min_m2m_scans;
        populate_scans( halo_min_m2m_plotters, halo_min_m2m_scans, min_m2m_cuts );
        Scanner halo_lower_min_m2m_scanner{ global_min_m2m_cut, halo_min_m2m_scans };

        Plotter halo_rk_plotter;
        std::vector<std::pair<Km2Cut*, Km2Listener*>> halo_rk_scans{ { &global_rk_cut, &halo_rk_plotter } };
        Scanner halo_rk_scanner{ global_rk_cut, halo_rk_scans } ;

        std::vector<Km2Listener*> halo_listeners(
                {  &halo_lower_zt_scanner, &halo_lower_cda_scanner,
                &halo_lower_m2m_scanner, &halo_lower_min_m2m_scanner, &halo_rk_scanner} );

        scan_region( *hl_tree, halo_listeners );

        //Combine
        std::ofstream of_lzt(Form( "output/scaling/%s_lower_zt.dat", pol));
        process_scans( tfout, of_lzt, data_zt_plotters, halo_zt_plotters, Form("%s_lower_zt", pol ) );

        std::ofstream of_lcda(Form( "output/scaling/%s_lower_cda.dat", pol));
        process_scans( tfout, of_lcda, data_cda_plotters, halo_cda_plotters, Form("%s_lower_cda", pol ) );

        std::ofstream of_lm2m(Form( "output/scaling/%s_lower_m2m.dat", pol));
        process_scans( tfout, of_lm2m, data_m2m_plotters, halo_m2m_plotters, Form("%s_lower_m2m", pol ) );

        std::ofstream of_mlm2m(Form( "output/scaling/%s_lower_min_m2m.dat", pol));
        process_scans( tfout, of_mlm2m,  data_min_m2m_plotters, halo_min_m2m_plotters,
                Form("%s_lower_min_m2m", pol ) );

        std::cout << Form( "%s_lower_rk", pol ) << std::endl;
        std::cout
            << std::setw(10) <<  data_rk_plotter.integral() 
            << std::setw(10)  << halo_rk_plotter.integral() 
            << std::setw(10)  << data_rk_plotter.integral()  / halo_rk_plotter.integral() 
            <<  std::endl;

    }

    exit(0);


    //auto dt_hl = get_dt_hl_trees( tfin, "pos", "lower" );
    //auto & dt_tree = dt_hl.first;
    //auto & hl_tree = dt_hl.second;


    //Plotter dt_plotter{};
    //{
    //    auto & tt = *dt_tree;
    //    auto & plotter = dt_plotter;

    //    std::vector<Km2Listener*> listeners{ &plotter };

    //    Km2EventData  * km2e = new Km2EventData;
    //    tt.SetBranchAddress( "Km2EventData", &km2e );

    //    Long64_t tt_entries = tt.GetEntries();
    //    ez::ezETAProgressBar prog( tt_entries );
    //    prog.start();

    //    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    //    {
    //        ++prog;

    //        tt.GetEntry(ient);
    //        for ( auto& listener : listeners )
    //        {
    //            if( rk_pz_area.allowed( {km2e->p, km2e->z} ) && km2e->cda < 3.5 )
    //            {
    //                plotter.process_event( *km2e );
    //            }
    //        }
    //    }
    //}

    //Plotter hl_plotter{};
    //{
    //    auto & tt = *hl_tree;
    //    auto & plotter = hl_plotter;

    //    std::vector<Km2Listener*> listeners{ &plotter };

    //    Km2EventData  * km2e = new Km2EventData;
    //    tt.SetBranchAddress( "Km2EventData", &km2e );

    //    Long64_t tt_entries = tt.GetEntries();
    //    ez::ezETAProgressBar prog( tt_entries );
    //    prog.start();

    //    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    //    {
    //        ++prog;

    //        tt.GetEntry(ient);
    //        for ( auto& listener : listeners )
    //        {
    //            if( rk_pz_area.allowed( {km2e->p, km2e->z} ) && km2e->cda < 3.5 )
    //            {
    //                plotter.process_event( *km2e );
    //            }
    //        }
    //    }
    //}

    //std::cout << dt_plotter.integral() / hl_plotter.integral() << std::endl ;


}
