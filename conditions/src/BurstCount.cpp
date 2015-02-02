#include "BurstCount.hh"
#include "TFile.h"
#include "HistStore.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "TROOT.h"
#include <TApplication.h>
#include "TClass.h"

namespace fn
{

    BurstCount::BurstCount( const Selection& sel,
            TFile& tfile, std::string folder,
            const fne::Event * e )
        :Analysis( sel ), tfile_( tfile), folder_( folder ), e_( e),
        burst_tree_( "bursts", "Burst info")
    {
        TTree::SetMaxTreeSize(10000000000LL);
        burst_tree_.Branch( "run", &burst_info_.run, "run/L" );
        burst_tree_.Branch( "burst_time", &burst_info_.burst_time, "burst_time/L" );
        burst_tree_.Branch( "events", &burst_info_.events, "events/L" );
        }

    void BurstCount::new_burst()
    {
        burst_info_.run = e_->header.run;
        burst_info_.burst_time = e_->header.burst_time;
        burst_info_.events = 0;
    }

    void BurstCount::process_event()
    {
        burst_info_.events++;
    }

    void BurstCount::end_burst()
    {
        burst_tree_.Fill();
    }

    void BurstCount::end_processing()
    {

#if 0
        for ( auto bc : bursts_ )
        {
            std::cout << bc.first << " " << bc.second << std::endl;
        }
#endif

        cd_p( &tfile_, folder_ );
        burst_tree_.Print();
        burst_tree_.Write();
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

            const fne::Event * e = rf.get_event_ptr();

            return new BurstCount( *sel, tfile, folder, e );
        }
}
