#include "BurstCount.hh"
#include "TFile.h"
#include "HistStore.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "TROOT.h"
#include <TApplication.h>
#include "TClass.h"
#include "K2piEventData.hh"

namespace fn
{


    BurstCount::BurstCount( const Selection& sel,
            TFile& tfile, std::string folder, std::string codename,
            const fne::Event * e )
        :Analysis( sel ), tfile_( tfile), folder_( folder ), e_( e),
        burst_tree_(0)
    {
        SaveDirectory sd;
        cd_p( &tfile_, folder_ );
        burst_tree_ = new TTree ( codename.c_str() , "Burst info");
        TTree::SetMaxTreeSize(10000000000LL);
        burst_tree_->Branch( "run", &burst_info_.run, "run/L" );
        burst_tree_->Branch( "burst_time", &burst_info_.burst_time, "burst_time/L" );
        burst_tree_->Branch( "events", &burst_info_.events, "events/L" );
        burst_tree_->Branch( "weight", &burst_info_.weight, "weight/D" );
    }

    void BurstCount::new_burst()
    {
        burst_info_.run = e_->header.run;
        burst_info_.burst_time = e_->header.burst_time;
        burst_info_.events = 0;
        burst_info_.weight = 0;
    }

    void BurstCount::process_event()
    {
        burst_info_.events++;
        burst_info_.weight +=  get_weight();
    }

    void BurstCount::end_burst()
    {
        burst_tree_->Fill();
    }

    void BurstCount::end_processing()
    {
        end_burst();

#if 0
        for ( auto bc : bursts_ )
        {
            std::cout << bc.first << " " << bc.second << std::endl;
        }
#endif

        cd_p( &tfile_, folder_ );
        //burst_tree_->Print();
        //burst_tree_->Scan();
        burst_tree_->Write();
        //burst_tree_.SetDirectory(0);
    }

    REG_DEF_SUB( BurstCount);

    template<>
        Subscriber * create_subscriber<BurstCount>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            std::string codename = get_yaml<std::string>( instruct, "codename" );

            const fne::Event * e = rf.get_event_ptr();

            return new BurstCount( *sel, tfile, folder, codename, e );
        }

    //--------------------------------------------------

    K2piBurstCount::K2piBurstCount( const Selection& sel,
            TFile& tfile, std::string folder,
            const K2piEventData& e )
        :Analysis( sel ), tfile_( tfile), folder_( folder ), e_( e ),
        burst_tree_(  new TTree( "bursts", "Burst info") )
    {
        TTree::SetMaxTreeSize(10000000000LL);
        burst_tree_->Branch( "run", &burst_info_.run, "run/L" );
        burst_tree_->Branch( "burst_time", &burst_info_.burst_time, "burst_time/L" );
        burst_tree_->Branch( "events", &burst_info_.events, "events/L" );
        burst_tree_->Branch( "weight", &burst_info_.weight, "weight/D" );
        new_burst();
    }

    void K2piBurstCount::new_burst()
    {
        burst_info_.run = e_.run;
        burst_info_.burst_time = e_.burst_time;
        burst_info_.events = 0;
        burst_info_.weight = 0;
    }

    void K2piBurstCount::process_event()
    {
        burst_info_.events++;
        burst_info_.weight +=  e_.weight * get_weight();
    }

    void K2piBurstCount::end_burst()
    {
        burst_tree_->Fill();
    }

    void K2piBurstCount::end_processing()
    {
        end_burst();
        cd_p( &tfile_, folder_ );
        std::cout << "Printing burst_tree" << std::endl;
        //burst_tree_->Print();
        burst_tree_->Write();
    }
}
