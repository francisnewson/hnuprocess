#include "BasicMuonStats.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "MuonHit.hh"
#include "TFile.h"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( BasicMuonStats );

    BasicMuonStats::BasicMuonStats( const Selection& sel, const fne::Event * event, bool filter,
            TFile& tf, std::string folder )
        :Analysis(sel), e_( event ), tf_( tf ), folder_( folder ), filter_( filter )
    {
        h_muv_status_ = hs_.MakeTH1D( "h_muv_status", "MUV status",
                5, -0.5, 4.5, "Status" );

        h_xy_ = hs_.MakeTH2D( "h_muon_position", "Muon Position",
                150, -150, 150, "X",
                150, -150, 150, "Y" );

        h_nmuon_ = hs_.MakeTH1D( "h_nmuon", "Number of muons",
                10, -0.5, 9.5, "N muons" );

        h_nhits_ = hs_.MakeTH1D( "h_nhits", "Number of PMT hits",
                20, -0.5, 19.5, "N hits" );

        h_muons_vs_hits_ = hs_.MakeTH2D( "h_muons_vs_hits", "Muonvs vs Hits",
                10, -0.5, 9.5, "N muons",
                20, -0.5, 19.5, "N hits" );

        h_muons_vs_tracks_ = hs_.MakeTH2D( "h_muons_vs_tracks", "Muonvs vs tracks",
                10, -0.5, 9.5, "N muons",
                10, -0.5, 9.5, "N tracks" );

        h_separation_ =  hs_.MakeTH2D( "h_separation", "Muon separation", 
                150, -150, 150, "X",
                150, -150, 150, "Y" );

        h_sep_x_ = hs_.MakeTH1D( "h_sep_x", "Muon separation x",
                150, -150, 150,  "x (cm)" );

        h_sep_y_ = hs_.MakeTH1D( "h_sep_y", "Muon separation y",
                150, -150, 150,  "y (cm)" );
    }

    void BasicMuonStats::process_event()
    {

        std::vector<const fne::RecoMuon*> my_muons;

        //Collect all muons
        const fne::Detector & det = e_->detector;
        const TClonesArray & muons = det.muons;
        for ( unsigned int imu = 0 ; imu != det.nmuons ; ++ imu )
        {
            const fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imu ) );

            my_muons.push_back( rm );
        }

        //Optionally only accept status 1 or 2
        if ( filter_ )
        {
            my_muons.erase( std::remove_if( begin( my_muons), end( my_muons),
                        []( const fne::RecoMuon * rm ) { return !(rm->status == 1 || rm->status ==2); } ), end( my_muons) );
        }

        auto nmuon = my_muons.size();
        auto nhits = det.nmuonhits;

        int ntracks = det.ntracks;

        h_muons_vs_tracks_->Fill( nmuon, ntracks );

        h_nmuon_->Fill( nmuon );
        h_nhits_->Fill( nhits );
        h_muons_vs_hits_->Fill( nmuon, nhits );

        for (auto rm : my_muons )
        {
            h_xy_->Fill( rm->x, rm->y );
            h_muv_status_->Fill( rm->status );
        }

        //Double loop to get inter-muon separation
        for ( unsigned int imu = 0 ; imu != nmuon ; ++ imu )
        {
            for ( unsigned int jmu = 0 ; jmu != imu ; ++ jmu )
            {
                const fne::RecoMuon& irm = * ( my_muons.at( imu ) );
                const fne::RecoMuon& jrm = * ( my_muons.at( jmu ) );

                double xsep = irm.x - jrm.x;
                double ysep = irm.y - jrm.y;

                h_separation_->Fill(xsep, ysep);
                h_sep_x_->Fill(xsep);
                h_sep_y_->Fill(ysep);
            }
        }
    }

    void BasicMuonStats::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<BasicMuonStats>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();

            const Selection * sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile(
                    get_yaml<std::string>( instruct, "tfile" ));

            std::string folder = get_folder ( instruct, rf );

            bool filter = get_yaml<bool>( instruct, "filter" );

            return new BasicMuonStats( *sel, event,filter, tfile, folder );
        }
}
