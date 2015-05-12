#include "ChopPlot.hh"
#include "Km2Reco.hh"
#include "SingleTrack.hh"
#include "stl_help.hh"
#include "root_help.hh"
#include "HistStore.hh"
#include "tracking_selections.hh"
#include "yaml_help.hh"
#include "RecoFactory.hh"

namespace fn
{
    M2MPlotter::M2MPlotter()
        :hm2m_( "hm2m", "Missing m^{2}", 1000, -0.7, 0.3 )
    {
        hm2m_.GetXaxis()->SetTitle( "m^{2}_{miss}" );
    }

    void M2MPlotter::fill( const Km2RecoEvent& km2re, double wgt )
    {
        hm2m_.Fill( km2re.get_m2m_kmu(), wgt );
    }

    void M2MPlotter::write()
    {
        hm2m_.Write();
    }

    //--------------------------------------------------

    REG_DEF_SUB( M2MChopPlot );

    M2MChopPlot::M2MChopPlot( const Selection& sel, const Km2Event& km2e, const SingleTrack& st, 
            TFile& tf , std::string folder )
        :Analysis( sel ), km2e_( km2e ), st_( st ), tf_( tf ),folder_( folder )
    {
        std::vector<std::pair<double,double>> mom_ranges{
            {0, 10}, {10,20}, {20,30}, {30,40}, {40,50}, {50, 60}, {60, 70} 
        };

        for ( auto& mr : mom_ranges )
        {
            std::string name{ Form("p_%03.0f_%03.0f", mr.first,  mr.second ) };

            mom_analyses_.push_back(  std::unique_ptr<ana>(  new ana{   
                        name,
                        make_unique<TrackMomentum>( st_, mr.first, mr.second ),
                        make_unique<M2MPlotter>()
                        } ) );
        }
    }

    void M2MChopPlot::process_event() 
    {
        const Km2RecoEvent& km2re = km2e_.get_reco_event();

        for( auto& an : mom_analyses_ )
        {
            an->sel->new_event();

            if ( an->sel->check())
            {
                double wgt = get_weight() * an->sel->get_weight();
                an->an->fill( km2re, wgt );
            }
        }
    }

    void M2MChopPlot::end_processing()
    {
        for( auto& an: mom_analyses_)
        {
            boost::filesystem::path my_folder = boost::filesystem::path{folder_}/an->name;
            cd_p( &tf_, my_folder );
            an->an->write();
        }
    }

    template<>
        Subscriber * create_subscriber<M2MChopPlot>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );
            const SingleTrack* single_track = get_single_track( instruct, rf );

            return new M2MChopPlot( *sel, *km2_event, *single_track, tfile, folder );
        }


}
