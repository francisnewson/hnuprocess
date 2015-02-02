#include "easy_app.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
#include "TFile.h"
#include "TKey.h"
#include "TROOT.h"
#include <TApplication.h>
#include "TClass.h"
#include "HistExtractor.hh"
#include <iterator>
#include <algorithm>
#include <numeric>

struct exval
{
    double val;
    double err;
};


exval poisson_exval( double  val )
{ return exval{ val, std::sqrt( val ) }; }

double rel_err( exval ev )
{ return ev.err / ev.val ; }

exval exval_ratio( const exval num, const exval denom )
{
    double res = num.val / denom.val;
    return exval {  res,  
        res * ( rel_err( num ) + rel_err( denom) ) };
}

exval  exval_product( const exval lhs, const exval rhs )
{
    double res = lhs.val * rhs.val;
    return  exval {  res,  
        res * ( rel_err( lhs ) + rel_err( rhs) ) };
}

exval  exval_add( const exval lhs, const exval rhs )
{
    return exval { lhs.val + rhs.val, std::hypot( lhs.err, rhs.err ) };
}

exval exval_sub( const exval lhs, const exval rhs )
{
    return exval { lhs.val - rhs.val, std::hypot( lhs.err, rhs.err ) };
}

exval exval_scale( const exval ev, double scale )
{
    return exval { scale * ev.val , scale*ev.err };
}

exval operator+( const exval lhs, const exval rhs){ return exval_add( lhs, rhs );  }
exval operator-( const exval lhs, const exval rhs){ return exval_sub( lhs, rhs );  }
exval operator*( const exval lhs, const exval rhs){ return exval_product( lhs, rhs );  }
exval operator/( const exval lhs, const exval rhs){ return exval_ratio( lhs, rhs );  }

//--------------------------------------------------

int main( int argc, char * argv[] )
{

    //**************************************************
    //Introduction
    //**************************************************
    splash( "input/art/scaling_splash.txt", std::cerr );
    echo_launch ( argc, argv, std::cerr );

    {
        std::ofstream sflaunch( "slaunch.log", std::ofstream::app );
        write_launch ( argc, argv, sflaunch );
    }

    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "input,i", po::value<path>()->required(),  "Specify input root file")
        ( "prefix,p", po::value<path>()->required(),  "Specify output prefix")
        ;

    po::options_description desc("Allowed options");
    desc.add( general );


    //**************************************************
    //Parse options
    //**************************************************

    //parse
    po::variables_map vm;
    po::store( po::parse_command_line(argc, argv, desc), vm);

    if ( vm.count( "help" ) )
    {
        std::cerr << desc << std::endl;
        std::cerr << "Exiting because help was requested" << std::endl;
        exit( 1 );
    }

    vm.notify();

    path input_filename = vm["input"].as<path>() ;
    path output_prefix = vm["prefix"].as<path>() ;

    //open input file
    TFile tfin{ input_filename.string().c_str() };
    RootTFileWrapper rtfwin{ tfin };
    ChannelHistExtractor hefin{ rtfwin };

    //mappings
    path neg_data_channel = "p5.data.q11t.v3.neg";
    path neg_km2_channel = "p5.km2.v2.neg";
    path neg_halo_channel = "p6.data.km.q11t.v2.neg";

    path pos_data_channel = "p5.data.q11t.v3.pos";
    path pos_km2_channel = "p5.km2.v2.pos";
    path pos_halo_channel = "p6.data.km.q11t.v2.pos";

    //fix cda vs mass
    hefin.set_post_path( "km2_fix_cda/h_m2m_kmu");

    auto h_neg_data = hefin.get_hist( neg_data_channel );
    auto h_neg_km2  = hefin.get_hist( neg_km2_channel );
    auto h_neg_halo  = hefin.get_hist( neg_halo_channel );

    auto h_pos_data = hefin.get_hist( pos_data_channel );
    auto h_pos_km2  = hefin.get_hist( pos_km2_channel );
    auto h_pos_halo  = hefin.get_hist( pos_halo_channel );

    std::vector<double> mod_mass( 100 );
    std::iota( mod_mass.begin(), mod_mass.end(), 0 );
    for ( auto & m : mod_mass ){ m *= 0.05 / 100;  }

    std::ofstream ofm( output_prefix.string() + "m2m.dat" );

    std::size_t w  = 15;

    double nh_pos_data = integral( *h_pos_data, -0.3, -0.1 );
    double nh_pos_halo  = integral( *h_pos_halo, -0.3, -0.1 );

    exval pos_halo_ratio = exval_ratio( 
            poisson_exval( nh_pos_data),
            poisson_exval( nh_pos_halo ) );

    double nh_neg_data = integral( *h_neg_data, -0.3, -0.1 );
    double nh_neg_halo  = integral( *h_neg_halo, -0.3, -0.1 );

    exval neg_halo_ratio = exval_ratio( 
            poisson_exval( nh_neg_data),
            poisson_exval( nh_neg_halo ) );

    std::ostream& os( ofm );
    os
        << std::setw(w) << "mod_mass" 
        << std::setw(w) << "n_pos_data" 
        << std::setw(w) << "n_pos_km2" 
        << std::setw(w) << "pos_ratio"
        << std::setw(w) << "pos_ratio_err"
        << std::setw(w) << "n_neg_data" 
        << std::setw(w) << "n_neg_km2" 
        << std::setw(w) << "neg_ratio"
        << std::setw(w) << "neg_ratio_err"
        << std::endl;

    for ( auto & m : mod_mass )
    {
        double n_pos_data = integral( *h_pos_data, -m, m );
        double  n_pos_km2  = integral( *h_pos_km2, -m, m );
        double  n_pos_halo  = integral( *h_pos_halo, -m, m );

        double n_neg_data = integral( *h_neg_data, -m, m );
        double  n_neg_km2  = integral( *h_neg_km2, -m, m );
        double  n_neg_halo  = integral( *h_neg_halo, -m, m );

        exval pos_ratio =
            ( poisson_exval( n_pos_data) - 
              pos_halo_ratio*poisson_exval( n_pos_halo )  )
            / poisson_exval( n_pos_km2 ) ;

        exval neg_ratio = exval_ratio( 
                exval_sub( poisson_exval( n_neg_data),
                    exval_product( neg_halo_ratio, poisson_exval( n_neg_halo ) ) ),
                poisson_exval( n_neg_km2 ) );

        std::size_t prec = os.precision( 6 );

        os
            << std::setw(w) << m 
            << std::setw(w) << n_pos_data 
            << std::setw(w) << n_pos_km2 
            << std::setw(w) << pos_ratio.val
            << std::setw(w) << pos_ratio.err
            << std::setw(w) << n_neg_data 
            << std::setw(w) << n_neg_km2 
            << std::setw(w) << neg_ratio.val
            << std::setw(w) << neg_ratio.err
            << std::endl;

        os.precision( prec );
    }

}
