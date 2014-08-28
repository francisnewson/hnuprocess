#include "K2piPlotter.hh"
#include "yaml_help.hh"
#include "TProfile.h"
#include "k2pi_extract.hh"

namespace fn
{
    REG_DEF_SUB( K2piPlotter);

    K2piPlotter::K2piPlotter( const Selection& sel,
            const fne::Event * e,
            TFile& tfile, std::string folder,
            const K2piReco& k2pi_reco, bool mc)
        :Analysis( sel), e_( e),
        tfile_( tfile), folder_( folder),
        k2pi_reco_( k2pi_reco ),kt_( e, mc ),  mc_( mc ),
        k2pi_plots_( &vars_ , tfile_, folder)
    {
    }

    void K2piPlotter::process_event()
    {
        //Prepare inputs
        const K2piRecoEvent& k2pirc = k2pi_reco_.get_reco_event();
        const SingleTrack& st = k2pi_reco_.get_single_track();
        const SingleRecoTrack& srt = st.get_single_track();
        kt_.new_event();

        //Extract vars_
        k2pi_extract( mc_, get_weight() , e_ , k2pirc, srt, kt_, vars_ );

        //Request plotting
        k2pi_plots_.new_event();
    }

    void K2piPlotter::end_processing()
    {
        k2pi_plots_.end_processing();
    }

    template<>
        Subscriber * create_subscriber<K2piPlotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const fne::Event * event = rf.get_event_ptr();

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_yaml<std::string>( instruct, "folder") ;

            const K2piReco * k2pi_reco = get_k2pi_reco( instruct, rf );

            bool mc = rf.is_mc();

            return new K2piPlotter
                ( *sel, event,  tfile, folder, *k2pi_reco, mc );
        }
}
