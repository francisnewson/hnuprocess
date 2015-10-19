#include "CdaComp.hh"
#include "yaml_help.hh"
#include "tracking_selections.hh"

namespace fn
{           
    REG_DEF_SUB( CdaComp );

    CdaComp::CdaComp( const Selection& sel, const SingleTrack& st,
            const fne::Event * e, bool mc,
            std::map<std::string, const KaonTrack*> kaon_tracks,
            TFile& tf, std::string folder )
        :Analysis( sel ), st_( st ), e_( e ), mc_( mc ), tf_( tf ), folder_( folder )
    {
        for ( const auto& kt : kaon_tracks )
        {
            auto * h = hs_.MakeTH1D( "hcda_" + kt.first, Form( "CDA for %s", kt.first.c_str() ),
                    1000, -10, 10, "CDA (cm)" );

            auto * hdxdz = hs_.MakeTH1D( "hdxdz_" + kt.first, Form( "DXDZ for %s", kt.first.c_str() ),
                    1000, -5e-4, 5e-4, "dxdz" );

            auto * h_xoff = hs_.MakeTH1D( "hxoff_" + kt.first, Form( "XOFF for %s", kt.first.c_str() ),
                    1000, -2, 2, "xoff (cm)" );


            h->SetDirectory(0);

            kts_.push_back( CdaPlotter{ kt.first, h, hdxdz, h_xoff,  kt.second} );
        }

        h_mccda_ = hs_.MakeTH1D( "hmccda" , "CDA for MC", 1000, -10, 10, "CDA (cm)" );
        h_mcdxdz_ = hs_.MakeTH1D( "hmcdxdz",  "MC DXDZ", 1000, -5e-4, 5e-4, "dxdz" );
        h_mcxoff_ = hs_.MakeTH1D( "hmcxoff",  "MC XOFF", 1000, -2, 2, "xoff" );
    }

    void CdaComp::process_event()
    {
        const auto& srt =  st_.get_single_track();
        //auto track =  Track{ srt.get_vertex(), srt.get_3mom() };
        auto track =  srt.get_bz_track();

        for ( auto cda_plot :  kts_ )
        {
            double cda = compute_cda( track, cda_plot.kt_->get_kaon_track() ).cda;
            cda_plot.h_cda_->Fill(  cda, get_weight() );

            TVector3 kmom = cda_plot.kt_->get_kaon_3mom();
            cda_plot.h_dxdz_->Fill( kmom.X() / kmom.Z() );

            const auto& ktrack = *cda_plot.kt_;
            double xoff = ktrack.extrapolate_z( 0 ).X();
            cda_plot.h_xoff_->Fill( xoff );
        }

        const auto& mc_kaon4mom = e_->mc.decay.kaon_momentum;
        const auto& mc_vertex = e_->mc.decay.decay_vertex;
        auto mcktrack = Track{ mc_vertex, mc_kaon4mom.Vect() };
        double mc_cda = compute_cda( track, mcktrack ).cda;
        double mc_xoff = mcktrack.extrapolate(0).X();
        h_mccda_->Fill( mc_cda );
        h_mcdxdz_->Fill( mc_kaon4mom.X() / mc_kaon4mom.Z() );
        h_mcxoff_->Fill( mc_xoff );
    }

    void CdaComp::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<CdaComp>
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile(
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            std::map<std::string, std::string> track_requests = 
                get_yaml<std::map<std::string, std::string> > (instruct,  "kaon_tracks" );

            const SingleTrack * st = get_single_track( instruct, rf );

            const fne::Event * e = rf.get_event_ptr();
            bool mc = rf.is_mc();

            std::map<std::string, const KaonTrack*> kaon_tracks;

            for ( const  auto& track_request : track_requests )
            {
                const KaonTrack  * kt = dynamic_cast<KaonTrack*>
                    (rf.get_subscriber( track_request.second ));

                if ( ! kt )
                {
                    std::cerr << "CdaComp: can't find KaonTrack called " +
                        track_request.second << std::endl;
                    throw std::runtime_error( 
                            "CdaComp: can't find KaonTrack called " +
                            track_request.second );
                }

                kaon_tracks.insert( std::make_pair( track_request.first, kt ) );
            }

            return new CdaComp( *sel, *st,e, mc, kaon_tracks, tfile, folder );
        }
}
