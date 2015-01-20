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
        :Analysis( sel ), tfile_( tfile), folder_( folder ), e_( e){}

    void BurstCount::process_event()
    {
        bursts_[ e_->header.burst_time] ++;
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
        TClass *c = gROOT->FindSTLClass("std::map<int,int>", true);
        std::cout << "Class: " << c << std::endl;
        c->Print();
        int res = gDirectory->WriteObjectAny( &bursts_, c,"bursts");
        std::cout << "Wrote: " << res << std::endl;
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
