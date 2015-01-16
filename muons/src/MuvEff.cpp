#include "MuvEff.hh"
#include "Selection.hh"
#include "Km2Reco.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"
#include "TNamed.h"
#include <string>

namespace fn
{
    REG_DEF_SUB( MuvEff );

    //--------------------------------------------------

    MuvGeom::MuvGeom( std::set<int> status, double muv_plane)
        :status_( status ), muv_plane_( muv_plane)
    {
        std::string s_status ="";
        auto n = status.begin();
        s_status += std::to_string(*n );
        ++n;
        while ( n != status.end() )
        {
            s_status += "_" + std::to_string( *n );
            ++n;
        }

        h_muv_hits_ = hs_.MakeTH2D( Form( "h_muv_hits_%s", s_status.c_str()), 
                Form( "MUV hits with status %s",s_status.c_str() ),
                150, -150 , 150, "X cm", 150, -150 , 150, "Y cm" );

        h_muv_track_hits_ = hs_.MakeTH2D( Form( "h_muv_track_hits_%s", s_status.c_str()), 
                Form( "MUV hits with status %s extrapolated from track", s_status.c_str() ),
                150, -150 , 150, "X cm", 150, -150 , 150, "Y cm" );
    }

    void MuvGeom::Fill( const fne::RecoMuon& rm, const SingleRecoTrack& srt, double wgt )
    {
        //std::cout << rm.plane[0] << "  " << rm.plane[1]  << std::endl;

        if ( status_.find(rm.status) == status_.end() ) { return; }

        TVector3 extrap = srt.extrapolate_ds( muv_plane_);
        h_muv_hits_->Fill( rm.x, rm.y, wgt );
        h_muv_track_hits_->Fill( extrap.X(), extrap.Y(), wgt );
        //std::cout << "Fill extrap x: " << extrap.X() << std::endl;
    }

    void MuvGeom::Write()
    {
        hs_.Write();
        TNamed s_muv_plane( "muv_plane",  Form( "%f", muv_plane_ ) );
        s_muv_plane.Write();
    }

    //--------------------------------------------------

    MuvEff::MuvEff( const Selection& sel, const Selection& muv_selection,
            const Km2Event& km2_event, const fne::Event * e,
            double muv_plane,
            TFile& tfile, std::string folder)
        :Analysis( sel), muv_selection_( muv_selection),
        km2e_( km2_event), e_( e ), muv_plane_( muv_plane ),
        tfile_( tfile), folder_( folder),
        //Statuses 1 to 4 use muv_plane_parameter
        muvhits_1_( {1}, muv_plane ), muvhits_2_( {2}, muv_plane ),
        muvhits_3_( {3}, muv_plane ), muvhits_4_( {4}, muv_plane ),

        //Statues for MUV1 and MUV2 use approprate z
        muvhits_muv1_( {1,2,4}, na62const::zMuv1 ),
        muvhits_muv2_( { 1,2,3 }, na62const::zMuv2 ),

        eff_mom_( "eff_mom", "Eff( muon momentum)", 100, 0, 100 ),
        eff_m2m_( "eff_m2m", "Eff( muon m^{2}_{miss})", 2000, -0.2, 0.2 ),
        eff_z_( "eff_z", "Eff(z vertex)", 120, -2000, 10000 ),
        eff_rmuv1_( "eff_rmuv1", "Eff(r muv 1)", 200, 0, 200 ),
        eff_x_( "eff_x", "Eff(x at MUV Plane)", 200, -200 , 200 ),
        eff_y_( "eff_y", "Eff(y at MUV Plane)", 200, -200 , 200 )
        {
            eff_mom_.SetDirectory( 0 );
            eff_m2m_.SetDirectory( 0 );
            eff_z_.SetDirectory( 0 );
            eff_rmuv1_.SetDirectory( 0 );
            eff_x_.SetDirectory( 0 );
            eff_y_.SetDirectory( 0 );
        }

    void MuvEff::process_event()
    {
        Km2RecoEvent km2re = km2e_.get_reco_event();
        double weight = get_weight();

        const SingleRecoTrack *srt = km2re.get_reco_track();

        all_plots_.Fill( km2re, weight );

        bool pass = muv_selection_.check() ;

        if  (pass)
        {
            pass_plots_.Fill( km2re, weight );
        }
        else
        {
            fail_plots_.Fill( km2re, weight );
        }

        TVector3 extrap = srt->extrapolate_ds( muv_plane_);
        double r_muv1 = extrap.Perp();
        double x_muv1 = extrap.X();
        double y_muv1 = extrap.Y();

        //std::cout << "extrap x: " << x_muv1 << std::endl;

        eff_mom_.Fill( pass, km2re.get_muon_mom() );
        eff_m2m_.Fill( pass, km2re.get_m2miss() );
        eff_z_.Fill( pass, km2re.get_zvertex() );
        eff_rmuv1_.Fill( pass, r_muv1 );
        eff_x_.Fill( pass, x_muv1 );
        eff_y_.Fill( pass, y_muv1 );

        const TClonesArray& muons = e_->detector.muons;
        Int_t nmuons = e_->detector.nmuons;

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

            muvhits_1_.Fill( *rm , *srt, weight );
            muvhits_2_.Fill( *rm , *srt, weight );
            muvhits_3_.Fill( *rm , *srt, weight );
            muvhits_4_.Fill( *rm , *srt, weight );
            muvhits_muv1_.Fill( *rm , *srt, weight );
            muvhits_muv2_.Fill( *rm , *srt, weight );
        }
    }

    void MuvEff::end_processing()
    {
        cd_p( &tfile_, folder_ + "/all");
        all_plots_.Write();

        cd_p( &tfile_, folder_ + "/pass");
        pass_plots_.Write();

        cd_p( &tfile_, folder_ + "/fail");
        fail_plots_.Write();

        cd_p( &tfile_, folder_ );
        eff_mom_.Write();
        eff_m2m_.Write();
        eff_z_.Write();
        eff_rmuv1_.Write();
        eff_x_.Write();
        eff_y_.Write();

        cd_p( &tfile_, folder_ );

        muvhits_1_.Write();
        muvhits_2_.Write();
        muvhits_3_.Write();
        muvhits_4_.Write();
        muvhits_muv1_.Write();
        muvhits_muv2_.Write();

    }

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<MuvEff>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv_sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "muv_selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            const fne::Event * event = rf.get_event_ptr();

            double muv_plane = get_yaml<double>( instruct, "muv_plane" );

            return new MuvEff( *sel, *muv_sel, *km2_event, event, muv_plane,  tfile, folder);
        }


}
