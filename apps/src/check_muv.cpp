#include "easy_app.hh"
#include "root_help.hh"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"

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

            cd_p( tfout.get() ,  cg.name / sel_folder  );
            teff.Write( plot_name.name.c_str() );
        }
    }
}
