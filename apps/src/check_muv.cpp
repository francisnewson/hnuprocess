#include "easy_app.hh"
#include "root_help.hh"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
#include <iostream>
#include <iomanip>

struct plot_instruct
{
    std::string name;
    int rebin;
};

struct chan_info
{
    boost::filesystem::path pre;
    boost::filesystem::path post;
};

struct chan_group
{
    std::string name;
    std::vector<chan_info> chans;
};

TGraphAsymmErrors substract_graph(const TGraphAsymmErrors& eff_data ,
       const TGraphAsymmErrors&  eff_mc )
{
    assert( eff_data.GetN() == eff_mc.GetN() );
    Int_t n = eff_data.GetN();

    std::vector<Double_t> x;
    std::vector<Double_t> y;
    std::vector<Double_t> xlow;
    std::vector<Double_t> xhigh;
    std::vector<Double_t> ylow;
    std::vector<Double_t> yhigh;

    for ( Int_t ibin =  0 ; ibin != n ; ++ibin )
    {
        auto dt_err = eff_data.GetErrorY( ibin );
        Double_t dt_x;
        Double_t dt_y;
        eff_data.GetPoint(ibin, dt_x, dt_y );

        auto mc_err = eff_data.GetErrorY( ibin );
        Double_t mc_x;
        Double_t mc_y;
        eff_mc.GetPoint( ibin, mc_x, mc_y );

        assert( mc_x == dt_x );

        auto diff_x  = dt_x;
        auto diff_y = dt_y - mc_y;

        auto x_low = eff_data.GetErrorXlow(ibin);
        auto x_high = eff_data.GetErrorXhigh(ibin);

        auto yerr = std::hypot( dt_err, mc_err );

        x.push_back( diff_x );
        xlow.push_back( x_low );
        xhigh.push_back( x_high  );
        y.push_back( diff_y );
        ylow.push_back( yerr );
        yhigh.push_back( yerr );
    }

    return TGraphAsymmErrors( n, x.data(), y.data(),
            xlow.data(), xhigh.data(), ylow.data(), yhigh.data() );
}


int main( int argc , char * argv[] )
{
    path input_file = "tdata/staging/muv_check.root" ;
    auto tfin = get_tfile( input_file );

    path output_file = "output/muv_check.root";
    auto tfout = get_tfile( output_file, "RECREATE" );

    std::vector<plot_instruct> plot_names
    {
        { "h_m2m", 1 },
            { "h_p", 10 },
            { "h_t", 10 },
            { "h_z", 1 },
            { "h_rmuv", 10 },
            { "h_phi", 10 }
    };

    path sel_folder{ "muv_eff_check" };

    std::vector<std::string> channels
    {
        "p5.data.q11t.v3.neg",
            "p5.data.q11t.v3.pos",
            "p5.km2.v2.neg",
            "p5.km2.v2.pos"
    };

    std::vector<chan_group> chan_groups
    {
        { "data", { { "pos", "hdata"}, {"neg", "hdata" } } },
            { "mc", { { "pos", "hbg"}, {"neg", "hbg" } } },
    };


    for ( const auto & plot_name : plot_names )
    {
        std::map<std::string, TGraphAsymmErrors> results;

        std::cerr << "Doing : " << plot_name.name << std::endl;

        for ( const auto & cg : chan_groups )
        {
            std::vector<std::unique_ptr<TH1D>> hs_pass;
            std::vector<std::unique_ptr<TH1D>> hs_all;

            for  ( const auto& chan : cg.chans )
            {
                auto p_pass = chan.pre / sel_folder / "pass" / plot_name.name/ chan.post;
                auto p_all = chan.pre / sel_folder / "all" / plot_name.name/ chan.post;

                auto h_pass = extract_hist<TH1D>( *tfin, p_pass );
                auto h_all = extract_hist<TH1D>( *tfin, p_all );

                std::cerr << p_pass << h_pass->GetNbinsX() << std::endl;
                std::cerr << p_all << h_all->GetNbinsX() << std::endl;

                h_pass->Rebin( plot_name.rebin );
                h_all->Rebin( plot_name.rebin );

                hs_pass.push_back( std::move( h_pass ) );
                hs_all.push_back( std::move( h_all ) );
            }

            auto h_pass = sum_hists( begin( hs_pass ), end( hs_pass ) );
            auto h_all = sum_hists( begin( hs_all ), end( hs_all ) );

            h_pass->Sumw2();
            h_all->Sumw2();

            TGraphAsymmErrors teff( h_pass.get(), h_all.get() );
            results.insert( std::make_pair(cg.name , teff) );

            cd_p( tfout.get() ,  cg.name / sel_folder  );
            teff.Write( plot_name.name.c_str() );
        }


        const TGraphAsymmErrors& eff_data = results.at( "data" );
        const TGraphAsymmErrors& eff_mc = results.at( "mc" );
        TGraphAsymmErrors eff_diff = substract_graph(eff_data , eff_mc );

        cd_p( tfout.get() ,  path{ "diffs" } );
        eff_diff.Write( plot_name.name.c_str()  );

    }
}
