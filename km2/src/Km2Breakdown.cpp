#include "Km2Breakdown.hh"
#include "Km2Reco.hh"
#include "SingleTrack.hh"
#include "yaml_help.hh"
#include "stl_help.hh"
namespace fn
{
    void PlotTrackQuality::init( Km2Breakdown& km2b)
    {
        h =  km2b.get_hist_store().MakeTH1D(
                "hTrackQuality", "TrackQuality", 100, 0.0, 10.0, "quality");
    }

    void PlotTrackQuality::fill( Km2Breakdown& km2b, double wgt )
    { h->Fill( km2b.get_single_reco_track().get_quality() ); }

    const SingleRecoTrack& Km2Breakdown::get_single_reco_track()
    {
        return *km2re_->get_reco_track();
    }

    REG_DEF_SUB( Km2Breakdown );

    Km2Breakdown::Km2Breakdown( const Selection& sel, const Km2Event& km2e,
            TFile& tf, std::string folder)
        :Analysis( sel ), km2e_( km2e ), tf_(tf), folder_( folder )
    {
        register_plotter<PlotTrackQuality>( "track_quality" );
    }

    void Km2Breakdown::add_selection( const Selection * s, std::vector<std::string>& plot_types )
    {
        std::vector<MiniPlot*> plots;
        for ( auto &plot_type : plot_types )
        {
            plot_store_.emplace_back( std::unique_ptr<MiniPlot>{ 
                    mini_plot_map_.at( plot_type)( *this ) } );

            plots.push_back( plot_store_.back().get() );
        }
        selection_plots_.push_back( std::make_pair( s, plots ) );
    }

    void Km2Breakdown::process_event()
    {
        km2re_ = &km2e_.get_reco_event();

        CompositeSelection selection_so_far;

        for ( auto& sel_plot : selection_plots_ )
        {
            for ( auto& plot : sel_plot.second )
            {
                plot->fill( *this, selection_so_far.get_weight() );
            }

            if (sel_plot.first->check() )
            {
                selection_so_far.AddChild( sel_plot.first );
            }
            else
            {
                continue;
            }
        }
    }

    template<>
        Subscriber * create_subscriber<Km2Breakdown>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Km2Event * km2e = get_km2_event( instruct, rf, "km2event" );

            const Selection * base = rf.get_selection(
                    get_yaml<std::string>( instruct, "base" ) );


            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            auto result  = make_unique<Km2Breakdown>( *base, *km2e, tfile, folder );

            const YAML::Node& selection_list = instruct["selection"];
            for ( const auto& selmap  : selection_list )
            {
                const Selection * sel 
                    = rf.get_selection( get_yaml<std::string>( selmap, "cut") );

                    std::vector<std::string> plots 
                    = selmap["plot"].as<std::vector<std::string>>();

                result->add_selection(  sel, plots );
            }
            return result.release();
        }
}
