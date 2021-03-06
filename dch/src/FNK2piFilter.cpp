#include "FNK2piFilter.hh"

#include "K2piClusters.hh"
#include "K2piInterface.hh"
#include "CorrCluster.hh"
#include "k2pi_fitting.hh"
#include "k2pi_reco_functions.hh"

#include "KaonTrack.hh"
#include "SingleTrack.hh"
#include "SingleMuon.hh"

#include "tracking_selections.hh"
#include "cluster_selections.hh"
#include "muon_selections.hh"

#include "NA62Constants.hh"
#include "RecoFactory.hh"

#include "yaml_help.hh"

#include "Math/Factory.h"
#include "Math/Functor.h"
#include "TTree.h"
#include "TFile.h"
#include "Xcept.hh"

namespace fn
{

    REG_DEF_SUB( FNK2piExtractor );

    FNK2piExtractor::FNK2piExtractor( 
            const Selection& weighter,
            const fne::Event * event, const SingleTrack & st , 
            const K2piClusters& k2pi_clusters, 
            const ClusterCorrector& cluster_corrector, const KaonTrack& kt,
            const SingleMuon& sm, const Selection& muon_eff,
            bool mc )
        :weighter_( weighter ),e_( event ), st_( st ), k2pi_clusters_( k2pi_clusters),
        cluster_corrector_( cluster_corrector),  kt_( kt ), 
        sm_(sm), muon_eff_(muon_eff),  mc_( mc )
    {}

    const K2piEventData * FNK2piExtractor::get_k2pi_event_ptr()
    {
        return &k2pi_event_data_;
    }

    void FNK2piExtractor::new_event()
    {
        dirty_ = true;
    }

    void FNK2piExtractor::update()
    {
        if ( dirty_ )
        {
            process_event();
            dirty_ = false;
        }
    }

    void FNK2piExtractor::process_event()
    {
        const SingleRecoTrack& srt = st_.get_single_track();
        const K2piRecoClusters& k2pirc = k2pi_clusters_.get_reco_clusters();

        //Extract raw info
        extract_raw_lkr(k2pirc, cluster_corrector_, kt_, k2pi_event_data_.raw_lkr );
        extract_raw_dch( srt, k2pi_event_data_.raw_dch );
        extract_muons( sm_, srt, muon_eff_, k2pi_event_data_.muv);

        if ( mc_ )
        {
            extract_k2pi_mc ( *e_ , k2pi_event_data_.mc );
        }

#if 0
        std::cout << "FNK2piEx:" 
            << k2pi_event_data_.raw_lkr.X1 <<  " " 
            << k2pi_event_data_.raw_lkr.Y1 <<  " " 
            << k2pi_event_data_.raw_lkr.E1 <<  " " 
            << k2pi_event_data_.raw_lkr.X2 <<  " " 
            << k2pi_event_data_.raw_lkr.Y2 <<  " " 
            << k2pi_event_data_.raw_lkr.E2 <<  " " 
            << std::endl;
#endif

        //Do Lkr Fit
        double chi2 = fit_lkr( k2pi_event_data_.raw_lkr, cluster_corrector_,
                k2pi_event_data_.fit_lkr, k2pi_event_data_.fit_lkr_err, 
                k2pi_event_data_.fit_lkr_fit_err, mc_ );
        k2pi_event_data_.lkr_fit_chi2 = chi2;

        //Extract header
        k2pi_event_data_.run = e_->header.run;
        k2pi_event_data_.burst_time = e_->header.burst_time;
        k2pi_event_data_.compact_number = e_->header.compact_number;
        k2pi_event_data_.weight = weighter_.get_weight();

        //Extract track cluster
        if( k2pirc.found_track_cluster() )
        {
            k2pi_event_data_.found_track_cluster = true;
            fne::RecoCluster track_cluster = k2pirc.track_cluster();
            k2pi_event_data_.TCE = track_cluster.energy;
            k2pi_event_data_.TCX = track_cluster.x;
            k2pi_event_data_.TCY = track_cluster.y;
        }
        else
        {
            k2pi_event_data_.found_track_cluster = false;
            k2pi_event_data_.TCE = -1.0;
            k2pi_event_data_.TCX = -200;
            k2pi_event_data_.TCY = -200;
        }
    }

    template<>
        Subscriber * create_subscriber<FNK2piExtractor>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "weighter" ) );
            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack* st = get_single_track( instruct, rf );
            const K2piClusters* k2pic = get_k2pi_clusters( instruct, rf );
            const KaonTrack* kt = get_kaon_track( instruct, rf );
            const Selection * muon_eff = rf.get_selection(
                    get_yaml<std::string>( instruct, "muon_eff" ) );
            const SingleMuon * sm = get_single_muon( instruct, rf );

            const ClusterCorrector * cluster_corrector = 
                get_cluster_corrector( instruct, rf );

            bool mc = rf.is_mc();

            return new FNK2piExtractor( *sel, event, *st,
                    *k2pic, *cluster_corrector,  *kt, *sm, *muon_eff,  mc );
        }

    //--------------------------------------------------

    void extract_raw_lkr( const K2piRecoClusters& k2pirc, 
            const ClusterCorrector& cc, const KaonTrack& kt , K2piLkrData& dest )
    {
        WK2piLkrInterface lkr( dest );

        //Clusters ( no projection correction at this stage )
        CorrCluster c1 {k2pirc.cluster1(), cc, k2pirc.is_mc() };
        CorrCluster c2 {k2pirc.cluster2(), cc, k2pirc.is_mc() };

        //Fill with calibrated enerygies
        lkr.E1() = c1.get_energy();
        lkr.E2() = c2.get_energy();

        //Fill with calibrated positions
        lkr.posC1_X() = c1.get_pos().X();
        lkr.posC1_Y() = c1.get_pos().Y();

        lkr.posC2_X() = c2.get_pos().X();
        lkr.posC2_Y() = c2.get_pos().Y();

        lkr.pK_X() = kt.get_kaon_3mom().X();
        lkr.pK_Y() = kt.get_kaon_3mom().Y();
        lkr.pK_Z() = kt.get_kaon_3mom().Z();

        lkr.pos0K_X() = kt.get_kaon_point().X();
        lkr.pos0K_Y() = kt.get_kaon_point().Y();
    }

    void extract_raw_dch( const SingleRecoTrack& srt, 
            K2piDchData& dest )
    {
        WK2piDchInterface dch ( dest );
        dch.p() = srt.get_mom();

        TVector3 mom = srt.get_3mom();
        dch.dxdz()  = mom.X() / mom.Z();
        dch.dydz()  = mom.Y() / mom.Z();

        TVector3 point = srt.extrapolate_bf( na62const::bz_tracking );
        dch.x0() = point.X();
        dch.y0() = point.Y();

        TVector3 ds_mom = srt.get_ds_mom();
        dch.ds_dxdz()  = ds_mom.X() / ds_mom.Z();
        dch.ds_dydz()  = ds_mom.Y() / ds_mom.Z();

        TVector3 ds_point = srt.extrapolate_ds( na62const::z_tracking );
        dch.ds_x0() = ds_point.X();
        dch.ds_y0() = ds_point.Y();
    }

    void extract_k2pi_mc( const fne::Event& e, K2piMcData& dest  )
    {
        K2piMcInterface mc( dest );
        boost::optional<k2pi_mc_parts> particles = extract_k2pi_particles( &e );
        if ( particles )
        {
        mc.p4k() = particles->k->momentum;
        mc.p4pip() = particles->pip->momentum;
        mc.p4pi0() = particles->pi0->momentum;
        mc.p4g1() = particles->photons[0]->momentum;
        mc.p4g2() = particles->photons[1]->momentum;
        mc.vertex() = particles->k->decay_vertex;
        }
    }

    void extract_muons( const SingleMuon& sm, const SingleRecoTrack& srt,
            const Selection & muon_eff, K2piMuvData& k2pi_muv )
    {
        if (!sm.found() )
        {
            k2pi_muv.found_muon = false;
            k2pi_muv.x = -1000;
            k2pi_muv.y = -1000;
            k2pi_muv.dx = -1000;
            k2pi_muv.dy = -1000;
            k2pi_muv.eff = -10;
        }
        else
        {
            std::pair<double,double> muon_track_separation
                = get_muon_track_separation( sm, srt );

            k2pi_muv.found_muon = true;
            k2pi_muv.x = sm.x();
            k2pi_muv.y = sm.y();

            k2pi_muv.dx = muon_track_separation.first;
            k2pi_muv.dy = muon_track_separation.second;

            k2pi_muv.eff = muon_eff.get_weight();
        }
    }

    double fit_lkr( const K2piLkrData& raw,
            const ClusterCorrector& cluster_corrector, K2piLkrData& fit, K2piLkrData& err,
            K2piLkrData& fit_err, bool mc )
    {
        //create fit object
        FNK2piFit fit_object( cluster_corrector, mc  );
        fit_object.load_raw_data( raw );
        fit_object.prepare_errors();

        ROOT::Math::Functor fit_functor( fit_object, 11 );
        auto minimizer = 
            ROOT::Math::Factory::CreateMinimizer( "Minuit", "MIGRAD" );

        minimizer->SetMaxFunctionCalls( 1000 );
        minimizer->SetMaxIterations( 100 );
        minimizer->SetTolerance( 0.01 );
        minimizer->SetFunction( fit_functor );

        fit_object.set_variables( minimizer );
        minimizer->Minimize();
        double chi2 = fit_object( minimizer->X() );

        WK2piLkrInterface out_fit ( fit );
        copy( fit_object.fit_, out_fit );

        WK2piLkrInterface out_err ( err );
        copy( fit_object.errors_, out_err );

        k2pi_params fit_errors;
        std::copy( minimizer->Errors(), minimizer->Errors() + 11, begin(fit_errors.par_) );

        WK2piLkrInterface out_fit_err ( fit_err );
        copy( fit_errors, out_fit_err );


        return chi2;
    }

    //--------------------------------------------------

    FNK2piExtractor * get_fnk2pi_extractor
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            FNK2piExtractor * k2pi_ex = 0;
            try
            {
                YAML::Node yk2piex = instruct["inputs"]["k2pix"];
                if ( !yk2piex )
                { throw Xcept<MissingNode>( "k2pix" ); }

                k2pi_ex = dynamic_cast<FNK2piExtractor*>
                    ( rf.get_subscriber( yk2piex.as<std::string>() ) );

                if( ! k2pi_ex )
                { throw Xcept<BadCast>( "K2PIX" ) ; }

            }
            catch( ... )
            {
                std::cerr << "Trying to get k2pi extractor (" __FILE__ ")\n" ;
                throw;
            }
            return k2pi_ex;
        }

    REG_DEF_SUB( FNK2piFilter );

    FNK2piFilter::FNK2piFilter( 
            const Selection& sel, 
            TFile& tfile, std::string tree_name,
            FNK2piExtractor& extractor, bool mc )
        :Analysis( sel), tfile_( tfile ), extractor_( extractor ),
        mc_( mc )
    {
        k2pi_event_ = extractor_.get_k2pi_event_ptr();

        tfile_.cd();
        ttree_ = new TTree( tree_name.c_str(), tree_name.c_str() );
        TTree::SetMaxTreeSize( 10000000000LL );
        ttree_->Branch( "K2piEventData", "fn::K2piEventData",
                &k2pi_event_ , 64000, 2 );

        BOOST_LOG_SEV( get_log(), always_print )
            << "MC is " << ( mc_ ? "true" : "false" );

        k2pi_user_info * ui = new k2pi_user_info();
        ui->is_mc = mc_;
        ui->test_string = "fred";
        ttree_->GetUserInfo()->Add( ui );
    }

    void FNK2piFilter::process_event()
    {
        extractor_.update();
        ttree_->Fill();
    }


    void FNK2piFilter::end_processing()
    {
        tfile_.cd();
        ttree_->Write();
    }

    template<>
        Subscriber * create_subscriber<FNK2piFilter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            FNK2piExtractor * extractor = get_fnk2pi_extractor( instruct, rf );
            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            bool is_mc = rf.is_mc();

            return new FNK2piFilter( *sel, tfile, "T",  *extractor, is_mc );
        }
}
