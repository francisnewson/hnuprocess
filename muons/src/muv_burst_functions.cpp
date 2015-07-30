#include "muv_burst_functions.hh"
#include <iostream>
#include <iomanip>
#include <set>
#include "TCanvas.h"
#include "TGraph.h"
#include "MuvBursts.hh"
#include "TEfficiency.h"
#include "ezETAProgressBar.hh"

using boost::filesystem::path;

namespace fn
{

    CachedTree::CachedTree( TFile& tfin, std::string format_string )
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

    TTree& CachedTree::get_tree()
    {
        return *cached_tree_;
    }

    //--------------------------------------------------

    //produce one plot per run
    void muv_burst_plots( TTree * tt, boost::filesystem::path output_folder )
    {
        //get unique runs
        std::set<Long64_t> runs;

        //set up reading tree
        muv_burst mb;
        tt->SetBranchAddress( "muv_burst", &mb );

        //loop over tree and collect runs
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

            int nselected_rows = static_cast<int>( tt->GetSelectedRows());
            TGraph graph{ nselected_rows, tt->GetV2(), tt->GetV1() };

            graph.SetTitle( Form("Run %lld", run ) );
            graph.Draw();
            path output_file = output_folder/ ("run"+std::to_string( run ) + ".pdf") ;
            c.SaveAs( output_file.string().c_str(), "pdf" );
        }
    }

    //--------------------------------------------------

    void muv_burst_text( TTree * tt, boost::filesystem::path output_file )
    {
        //get unique runs
        muv_burst mb;
        tt->SetBranchAddress( "muv_burst", &mb );

        std::cerr << "Writing burst info to " << output_file << std::endl;
        boost::filesystem::ofstream ofs( output_file );

        ofs << std::setw( 15 ) << "run"
            << std::setw( 15 ) << "burst_time"
            << std::setw( 15 ) << "pass"
            << std::setw( 15 ) << "total"
            << std::setw( 15 ) << "eff"
            << std::setw( 15 ) << "up"
            << std::setw( 15 ) << "down"
            << "\n";

        Long64_t tt_entries = tt->GetEntries();
        for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
        {
            if ( !(ient % 1000) )
            {
                std::cerr << "Processed " << ient << " bursts!" << std::endl;
            }

            tt->GetEntry( ient );
            double eff = mb.pass / mb.total;
            double up = 0;
            double down = 1;

            TEfficiency::FeldmanCousinsInterval( mb.total, mb.pass, 0.68, down, up );

            ofs << std::setw( 15 ) << mb.run 
                << std::setw( 15 ) << mb.burst_time
                << std::setw( 15 ) << mb.pass
                << std::setw( 15 ) << mb.total
                << std::setw( 15 ) << eff
                << std::setw( 15 ) << up
                << std::setw( 15 ) << down
                << "\n" << std::flush;

        }
    }

    //--------------------------------------------------

    void print_bursts( TTree& tt, int run,
            int min_burst, int max_burst, std::ostream& os )
    {
        muv_burst mb;
        tt.SetBranchAddress( "muv_burst", &mb );

        Long64_t tt_entries = tt.GetEntries();
        for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
        {
            tt.GetEntry( ient );

            if ( (mb.run == run ) 
                    && (mb.burst_time >= min_burst )
                    && ( mb.burst_time <= max_burst ) ) 
            {
                double up = TEfficiency::ClopperPearson( mb.total, mb.pass, 0.90, true );
                double dn = TEfficiency::ClopperPearson( mb.total, mb.pass, 0.90, false );
                os 
                    << std::setw(15) << mb.run
                    << std::setw(20) << mb.burst_time
                    << std::setw(10) << mb.pass
                    << std::setw(10) << mb.total
                    << std::setw(10) << double(mb.pass) / double(mb.total)
                    << std::setw(10) << dn
                    << std::setw(10) << up
                    << "\n";
            }
        }
    }

    //--------------------------------------------------

    std::vector<fn::BadBurst::BurstId> find_bad_bursts( TTree&  tt,  double threshold )
    {
        std::vector<fn::BadBurst::BurstId> result;
        muv_burst mb;
        tt.SetBranchAddress( "muv_burst", &mb );

        Long64_t tt_entries = tt.GetEntries();
        for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
        {
            tt.GetEntry( ient );

            if ( mb.total == 0 ) // probably already marked bad by someone else
            {
                continue;
            }

            if ( mb.run == 20438 && mb.burst_time >= 1188939862 && mb.burst_time <= 1188951008 )
            {
                //already covered by bad burst ranges
                continue;
            }

            if ( mb.pass/mb.total < threshold || mb.pass < 10)
            {
                result.push_back( 
                        fn::BadBurst::BurstId{ 
                        static_cast<int>(mb.run),
                        static_cast<int>(mb.burst_time)} );
            }
        }

        return result;
    }

    //find bursts with eff statistically below threshold
    std::vector<fn::BadBurst::BurstId> 
        find_stat_bad_bursts( TTree&  tt,  double threshold, double level )
        {

            if ( level > 1 )
            {
                throw std::runtime_error( "Confidence level > 1 requested!" );
            }

            std::vector<fn::BadBurst::BurstId> result;
            muv_burst mb;
            tt.SetBranchAddress( "muv_burst", &mb );

            Long64_t tt_entries = tt.GetEntries();
            ez::ezETAProgressBar prog( tt_entries );
            prog.start();
            for ( Long64_t ient = 0 ; ient != tt_entries ; ++ient )
            {
                ++prog;
                tt.GetEntry( ient );

                //TEfficiency::FeldmanCousinsInterval( mb.total, mb.pass, level, down, up );
                double up = TEfficiency::ClopperPearson( mb.total, mb.pass, level, true );

                if ( mb.total == 0 ) // probably already marked bad by someone else
                {
                    continue;
                }

                if ( mb.run == 20438 && mb.burst_time >= 1188939862 && mb.burst_time <= 1188951008 )
                {
                    //already covered by bad burst ranges
                    continue;
                }

                if ( up < threshold  || mb.pass < 10 )
                {
                    result.push_back( 
                            fn::BadBurst::BurstId{ 
                            static_cast<int>(mb.run),
                            static_cast<int>(mb.burst_time)} );
                }
            }

            return result;
        }
}
