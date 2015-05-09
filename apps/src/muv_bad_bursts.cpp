#include "easy_app.hh"
#include "TH2D.h"
#include "yaml_help.hh"
#include "TFile.h"
#include "TTree.h"
#include "TEfficiency.h"
#include "root_help.hh"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <set>
#include "MuvBursts.hh"
#include "BadBurst.hh"
#include "TCanvas.h"
#include "TGraph.h"
#include <iomanip>
#include <limits>
#include "TNtuple.h"

using boost::filesystem::path;

class CachedTree
{
    private:
        std::unique_ptr<TFile> cache_file_;
        TTree * cached_tree_;
    public:
        CachedTree( TFile& tfin, std::string format_string )
        {
            //Load pos and neg polarity from files
            TTree * tt_neg = 0;
            TTree * tt_pos = 0;
            tfin.GetObject( Form( format_string.c_str(), "neg"), tt_neg );
            tfin.GetObject( Form( format_string.c_str(), "pos"), tt_pos );

            //Create cache file to hold new merged tree
            cache_file_.reset( new TFile(   "/tmp/fnewson/tcache.root", "RECREATE") );

            //Merge pos and neg trees
            TList * tlist = new TList;
            tlist->Add( tt_neg );
            tlist->Add( tt_pos );
            cached_tree_ = TTree::MergeTrees( tlist );

            std::cout << "Neg bursts " <<       tt_neg->GetEntries() << "\n";
            std::cout << "Pos bursts " <<       tt_pos->GetEntries() << "\n";
            std::cout << "All bursts " << cached_tree_->GetEntries() << "\n";
        }

        TTree& get_tree()
        {
            return *cached_tree_;
        }
};


void muv_burst_plots( TTree * tt, path output_folder )
{
    //get unique runs
    std::set<Long64_t> runs;

    muv_burst mb;
    tt->SetBranchAddress( "muv_burst", &mb );

    Long64_t tt_entries = tt->GetEntries();
    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    {
        tt->GetEntry( ient );
        runs.insert( mb.run );
    }

    //Loop over runs and plot
    for ( auto run : runs )
    {
        TCanvas c{ "c", "c", 800, 200 };
        tt->Draw("pass/total:burst_time", Form("run==%lld", run ), "goff" );
        TGraph graph{ static_cast<int>( tt->GetSelectedRows()), tt->GetV2(), tt->GetV1() };
        graph.SetTitle( Form("Run %lld", run ) );
        graph.Draw();
        path output_file = output_folder/ ("run"+std::to_string( run ) + ".pdf") ;
        c.SaveAs( output_file.string().c_str(), "pdf" );
    }
}

void print_bursts( TTree& tt, int run, int min_burst, int max_burst, std::ostream& os )
{
    muv_burst mb;
    tt.SetBranchAddress( "muv_burst", &mb );

    Long64_t tt_entries = tt.GetEntries();
    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    {
        tt.GetEntry( ient );
        if ( (mb.run == run ) && (mb.burst_time > min_burst ) && ( mb.burst_time < max_burst ) ) 
        {
            os 
                << std::setw(15) << mb.run
                << std::setw(20) << mb.burst_time
                << std::setw(10) << mb.pass
                << std::setw(10) << mb.total
                << std::setw(10) << double(mb.pass) / double(mb.total)
                << "\n";
        }
    }
}

std::vector<fn::BadBurst::BurstId> find_bad_bursts( TTree&  tt,  double threshold )
{
    std::vector<fn::BadBurst::BurstId> result;
    muv_burst mb;
    tt.SetBranchAddress( "muv_burst", &mb );

    Long64_t tt_entries = tt.GetEntries();
    for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
    {
        tt.GetEntry( ient );

        if ( mb.pass/mb.total < threshold )
        {
            result.push_back( 
                    fn::BadBurst::BurstId{ static_cast<int>(mb.run), static_cast<int>(mb.burst_time)} );
        }
    }

    return result;
}


int main( int argc, char * argv[] )
{

    //**************************************************
    //Option logic
    //**************************************************

    //Set up program options
    po::options_description general("general");

    general.add_options()
        ( "help,h", "Display this help message")
        ( "plots,p",  "Make bad burst plots")
        ( "bursts,b", po::value<double>(),  "Specify bad burst threshold")
        ( "input,i", po::value<path>(),  "Specify input root file")
        ( "output,o", po::value<path>(),  "Specify output")
        ( "run_range,r", po::value<int>(),  "Run range")
        ( "start_burst,s", po::value<int>(),  "Start burst")
        ( "end_burst,e", po::value<int>(),  "End burst")
        ( "merge,m",  "merge trees")
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

    if ( !vm.count( "input" ) )
    {
        std::cerr << "Must specify input file!" << std::endl;
        exit(1);
    }

    TFile tfin( vm["input"].as<path>().string().c_str() );

    if ( vm.count("plots") + vm.count("bursts") + vm.count("run_range") > 1 )
    {
        std::cerr << "Can only do plots OR bursts OR run_range!" << std::endl;
        exit(1);
    }

    if ( vm.count("run_range" ) )
    {
        auto run = vm["run_range"].as<int>();
        CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };

        int min_burst = 0;
        int max_burst = std::numeric_limits<int>::max();

        if ( vm.count("min_burst" ) )
        {
            min_burst = vm["min_burst"].as<int>();
        }

        if ( vm.count("max_burst" ) )
        {
            max_burst = vm["min_burst"].as<int>();
        }

        path output_file = "output/burst_list.dat";
        if ( vm.count( "output" ) )
        {
            output_file = vm["output"].as<path>();
        }

        boost::filesystem::ofstream os{output_file};

        print_bursts( ct.get_tree(), run, min_burst, max_burst, os );

        std::cerr << "Output written to " << output_file.string() << std::endl;
    }

    if ( vm.count("plots") )
    {
        path output_folder = "output/muv_bursts/";
        if ( vm.count( "output" ) )
        {
            output_folder = vm["output"].as<path>();
        }

        CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };
        muv_burst_plots( &ct.get_tree(), output_folder );

    }

    if ( vm.count( "merge") )
    {
        path output_file = "output/muv_bursts.root";
        if ( vm.count( "output" ) )
        {
            output_file = vm["output"].as<path>();
        }

        CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };
        TTree& tt = ct.get_tree();
        TFile tfout( output_file.string().c_str(), "RECREATE") ;
        ct.get_tree().CloneTree()->Write("muv_burst" );

        Long64_t run;
        Long64_t burst_time;
        Double_t pass;
        Double_t total;

        TTree my_ttree( "split_muv_bursts", "Muv bursts");
        my_ttree.Branch( "run", &run );
        my_ttree.Branch( "burst_time", &burst_time );
        my_ttree.Branch( "pass", &pass );
        my_ttree.Branch( "total", &total );

        muv_burst mb;
        tt.SetBranchAddress( "muv_burst", &mb );

        Long64_t tt_entries = tt.GetEntries();
        for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
        {
            tt.GetEntry( ient );
            run = mb.run;
            burst_time = mb.burst_time;
            total = mb.total;
            pass = mb.pass;

            my_ttree.Fill();
        }

        my_ttree.Write();

        tfout.Close();
    }

    if ( vm.count( "bursts") )
    {
        double threshold = vm["bursts"].as<double>();

        path output_file = "output/bad_muv_bursts.dat";
        if ( vm.count( "output" ) )
        {
            output_file = vm["output"].as<path>();
        }
        CachedTree ct{ tfin,  "p5.data.q11t.v3.%s/muv_bursts/T" };

        std::vector<fn::BadBurst::BurstId> bad_bursts =
            find_bad_bursts( ct.get_tree(), threshold );


        boost::filesystem::ofstream os{output_file};

        for ( auto bb : bad_bursts )
        {
            os << std::setw(10) << bb.run << std::setw(20) << bb.burst_time << "\n";
        }

        std::cerr << "Output written to " << output_file.string() << std::endl;
    }
}
