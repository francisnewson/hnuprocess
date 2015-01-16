#include "MuvPmtHits.hh"
#include "NA62Constants.hh"
#include "Selection.hh"
#include "Km2Reco.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"
#include "TNamed.h"
#include "Event.hh"
#include "MuonHit.hh"
#include <string>

namespace fn
{
    MuvPmtPlotter::MuvPmtPlotter( std::string name, int n_strips )
    {
        for ( int i = 0 ; i != n_strips + 1 ; ++ i )
        {
            vhs_.push_back( hs_.MakeTH2D( name + std::to_string( i ),
                        name + std::to_string( i ),
                        100, -200, 200, "X (cm)",
                        100, -200, 200, "Y (cm)" ) );
        }
    }

    void MuvPmtPlotter::Fill( int strip_id, double x, double y, double wgt )
    {
        vhs_[strip_id]->Fill( x, y, wgt );
    }

    void MuvPmtPlotter::Write()
    {
        hs_.Write();
    }

    MuvPmtHits::MuvPmtHits( const Selection& sel, 
            const Km2Event& km2_event, const fne::Event * e,
            TFile& tfile, std::string folder)
        :Analysis( sel), km2e_( km2_event), e_( e ),
        tfile_( tfile ), folder_( folder ),
        muv1_( "muv1_" , 11 ), muv2_( "muv2_", 11 ), 
        muv3_( "muv3_", 8 ), muv_all_ ("muv_all_", 56 )
    {
        hpmt_ids_ = hs_.MakeTH1D( 
                "hpmt_ids", "MUV PMT HIT IDS",
                60, -0.5, 59.5, "PMT ID" );
    }

    void MuvPmtHits::process_event()
    {
        double weight = get_weight();

        //Extrapolate track to all three planes
        Km2RecoEvent km2re = km2e_.get_reco_event();
        const SingleRecoTrack *srt = km2re.get_reco_track();

        TVector3 extrap1 = srt->extrapolate_ds(na62const::zMuv1);
        TVector3 extrap2 = srt->extrapolate_ds(na62const::zMuv2);
        TVector3 extrap3 = srt->extrapolate_ds(na62const::zMuv3);

        auto& emuon_hits = e_->detector.muon_hits;
        int nmuon_hits = e_->detector.nmuonhits;
        for ( int imuon = 0 ; imuon != nmuon_hits ; ++imuon )
        {
            //copy reco track ( and point to it )
            fne::MuonHit * mh = static_cast<fne::MuonHit*>
                ( emuon_hits[imuon] );

            muv_all_.Fill( mh->channel, extrap1.X(), extrap1.Y(), weight );

            //std::cout << mh->channel << " ";

            hpmt_ids_->Fill( mh->channel, weight );

            if ( mh->channel < 23 )
            {
                int strip = mh->channel - 1;
                if ( strip > 10 ) {strip -= 11;}

            //std::cout << strip << std::endl;
                muv1_.Fill( strip, extrap1.X(), extrap1.Y(), weight );
            }
            else if ( mh->channel < 45 )
            {
                int strip = mh->channel - 23;
                if ( strip > 10 ) {strip -= 11;}
            //std::cout << strip << std::endl;
                muv2_.Fill( strip, extrap2.X(), extrap2.Y(), weight );
            }
            else
            {
                int strip = mh->channel - 45;
                if ( strip > 5 ) {strip -= 6;}
            //std::cout << strip << std::endl;
                muv3_.Fill( strip, extrap3.X(), extrap2.Y(), weight );
            }
        }
    }

    void MuvPmtHits::end_processing()
    {
        using boost::filesystem::path;

        cd_p( &tfile_, folder_ );
        hs_.Write();

        cd_p( &tfile_, path{folder_}/"MUVALL" );
        muv_all_.Write();

        cd_p( &tfile_, path{folder_}/"MUV1" );
        muv1_.Write();

        cd_p( &tfile_, path{folder_}/"MUV2" );
        muv2_.Write();

        cd_p( &tfile_, path{folder_}/"MUV3" );
        muv3_.Write();
    }

    REG_DEF_SUB( MuvPmtHits );

    template<>
        Subscriber * create_subscriber<MuvPmtHits>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            const fne::Event * event = rf.get_event_ptr();

            return new MuvPmtHits( *sel, *km2_event, event, tfile, folder);
        }
}
