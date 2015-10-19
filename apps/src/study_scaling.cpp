#include "easy_app.hh"
#include "TFile.h"
#include "HistStore.hh"
#include "root_help.hh"

int main( int argc, char * argv[] )
{

    auto tfin = get_tfile( "tdata/staging/all.plots.root" );
    auto tfout = get_tfile( "output/study_scaling.root", "RECREATE" );

    for ( auto pol : { "pos", "neg" } )
    {

        auto h_upper_halo = extract_hist<TH1D>( *tfin, std::string("p6.halo.q11t.") + pol + "/ctrl_upper_muv_plots/h_z" );
        auto h_upper_dt = extract_hist<TH1D>( *tfin, std::string("p5.data.q11t.v3.") + pol + "/ctrl_upper_muv_plots/h_z" );

        double hscale = integral( *h_upper_dt, 3000, 5500 )
            / integral( *h_upper_halo, 3000, 5500 );

        auto  up_scale_map = makeTH2D( "h_up_scale_map", "Scale map",
                50,  1050, 6050, "lower",
                50,  1050, 6050, "upper" );

        for ( int up = 2100 ; up != 5500 ; up += 100 )
        {
            for ( int dn = 2000 ; dn < up ; dn += 100 )
            {
                double this_scale = integral( *h_upper_dt, dn, up )
                    / integral( *h_upper_halo, dn, up );

                up_scale_map->Fill( dn, up, this_scale);
            }
        }

        h_upper_halo->Scale( hscale);

        std::cout << "Writing" << std::endl;
        cd_p( tfout.get(), pol );
        up_scale_map->Write("up_scale");
        h_upper_halo->Write("h_up_z_halo");
        h_upper_dt->Write( "h_up_z_dt");

        std::cout << "Done writing" << std::endl;
    }

    for ( auto pol : { "pos", "neg" } )
    {

        std::cout << "Loading" << std::endl;
        auto h_lower_halo = extract_hist<TH1D>( *tfin, std::string("p6.halo.q11t.") + pol + "/ctrl_lower_full_m2m_plots/h_m2m_kmu" );
        auto h_lower_dt = extract_hist<TH1D>( *tfin, std::string("p5.data.q11t.v3.") + pol + "/ctrl_lower_full_m2m_plots/h_m2m_kmu" );

        std::cout << "Scaling" << std::endl;
        double hscale = integral( *h_lower_dt, -0.15, -0.05 )
            / integral( *h_lower_halo, -0.15, -0.05 );

        auto  up_scale_map = makeTH2D( "h_up_scale_map", "Scale map",
                21,  -0.205, +0.005, "lower",
                21,  -0.205, +0.005, "lower" );

        std::cout << "Looping" << std::endl;
        for ( double up = -0.20 ; up < 0.01 ; up += 0.01 )
        {
            for ( double dn = -0.20 ; dn < up ; dn += 0.01 )
            {
                double this_scale = integral( *h_lower_dt, dn, up )
                    / integral( *h_lower_halo, dn, up );

                up_scale_map->Fill( dn, up, this_scale);
            }
        }

        h_lower_halo->Scale( hscale);

        cd_p( tfout.get(), pol );
        up_scale_map->Write("dn_scale");
        h_lower_halo->Rebin(10);
        h_lower_dt->Rebin(10);
        h_lower_halo->Write("h_dn_m2m_halo");
        h_lower_dt->Write( "h_dn_m2m_dt");
    }
}
