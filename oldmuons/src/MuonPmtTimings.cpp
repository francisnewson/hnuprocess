#include "MuonPmtTimings.hh"
#include "Km2Reco.hh"
#include "yaml_help.hh"
#include "MuonHit.hh"

namespace fn
{
            REG_DEF_SUB( MuonPmtTimings );

    MuonPmtTimings::MuonPmtTimings( const Selection& sel, 
            const Km2Event& km2_event,
            const fne::Event * e,
            TFile& tfile, std::string folder)
        :Analysis( sel), km2e_( km2_event), e_( e ),
        tfile_( tfile ), folder_( folder ), h_chantimes_(0)
    {

    }

    void MuonPmtTimings::new_run()
    {
        h_chantimes_ = hs_.MakeTH2D(
                Form( "h_chan_time_%d", e_->header.run ), 
                Form("Time vs Chan  run %d", e_->header.run ),
                60, -0.5, 59.5, "Channel",
                500, -100, 100, "Time" );
    }

    void MuonPmtTimings::end_run()
    {
        h_chantimes_ = 0;
    }

    void MuonPmtTimings::process_event()
    {

        if ( ! h_chantimes_ )
        {
            throw std::runtime_error( 
                    "process_event in " __FILE__ 
                    " with no chantimes_ set" );
        }

        //Extrapolate track to all three planes
        Km2RecoEvent km2re = km2e_.get_reco_event();
        const SingleRecoTrack *srt = km2re.get_reco_track();
        double track_time = srt->get_time();

        auto& emuon_hits = e_->detector.muon_hits;
        int nmuon_hits = e_->detector.nmuonhits;

        for ( int imuon = 0 ; imuon != nmuon_hits ; ++imuon )
        {
            //get pointer to muon hit
            fne::MuonHit * mh = static_cast<fne::MuonHit*>
                ( emuon_hits[imuon] );

            double rel_time = mh->time -track_time;

            h_chantimes_->Fill( mh->channel, rel_time );
        }

    }

    void MuonPmtTimings::end_processing()
    {
        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<MuonPmtTimings>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            const fne::Event * event = rf.get_event_ptr();

            return new MuonPmtTimings( *sel, *km2_event, event, tfile, folder);
        }
}
