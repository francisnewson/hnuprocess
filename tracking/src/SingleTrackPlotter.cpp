#include "SingleTrackPlotter.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "tracking_selections.hh"

namespace fn
{
    REG_DEF_SUB( SingleTrackPlotter);

    SingleTrackPlotter::SingleTrackPlotter( const Selection& sel , 
            TFile& tfile, std::string folder,
            const SingleTrack& st )
        :Analysis( sel ), tfile_( tfile ), folder_( folder ), st_( st ),
        scattering_plots_( "Unscattered", "Scattered" )
    {}

    void SingleTrackPlotter::process_event()
    {
        const auto & srt = st_.get_single_track();

        TVector3 point = srt.get_vertex();
        TVector3 p_unscat = srt.get_unscattered_3mom();
        TVector3 p_final = srt.get_us_mom();

        scattering_plots_.Fill( point, p_unscat, point, p_final, get_weight() );
    }

    void SingleTrackPlotter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        scattering_plots_.Write();
    }

    template<>
        Subscriber * create_subscriber<SingleTrackPlotter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
        
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            SingleTrack * st = get_single_track( instruct, rf );

            return new SingleTrackPlotter( *sel, tfile, folder, *st );
        }
}
