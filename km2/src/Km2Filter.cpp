#include "Km2Filter.hh"
#include "root_help.hh"
#include "NA62Constants.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{
    REG_DEF_SUB( Km2Filter );

    Km2Filter::Km2Filter( const Selection& sel,
            const Km2Event& km2_event,
            const Selection& muv_sel,
            TFile& tf,  std::string folder,
            std::string tree_name)
        :Analysis( sel ), km2_event_( km2_event ),
        muv_sel_( muv_sel ),
        tfile_( tf ), folder_( folder )
    {
        SaveDirectory sd{};

        cd_p( &tfile_, folder_ );
        ttree_ = new TTree(  tree_name.c_str(),
                tree_name.c_str() );
        TTree::SetMaxTreeSize( 100000000000LL );
        ttree_->Branch( "Km2EventData", "fn::Km2EventData",
                &event_data_, 6400, 2 );
    }

    void Km2Filter::process_event()
    {
        const Km2RecoEvent& km2re = km2_event_.get_reco_event();
        event_data_.vertex = km2re.get_vertex();
        event_data_.mu_mom = km2re.get_p4_mu().Vect();

        event_data_.kpoint = km2re.get_kaon_track()->get_kaon_track().get_point();
        event_data_.k_mom = km2re.get_kaon_track()->get_kaon_mom() * 
            km2re.get_kaon_track()->get_kaon_track().get_direction().Unit();

        event_data_.p = km2re.get_muon_mom();
        event_data_.z = km2re.get_zvertex();
        event_data_.t = km2re.get_opening_angle();
        event_data_.cda = km2re.get_cda();
        event_data_.m2m = km2re.get_m2m_kmu();

        event_data_.event_wgt = get_weight();
        event_data_.muv_wgt = muv_sel_.get_weight();

        event_data_.tx = km2re.get_tx();
        event_data_.ty = km2re.get_ty();

        const SingleRecoTrack * srt = km2re.get_reco_track();
        TVector3 v_coll = srt->extrapolate_bf( na62const::zFinalCollimator );

        event_data_.xcoll = v_coll.X();
        event_data_.ycoll = v_coll.Y();

        TVector3 v_dch4 = srt->extrapolate_ds( na62const::zFinalCollimator );
        event_data_.xdch4 = v_dch4.X();
        event_data_.ydch4 = v_dch4.Y();

        ttree_->Fill();
    }

    void Km2Filter::end_processing()
    {
        cd_p( &tfile_, folder_ );
        ttree_->Write();
    }


    template<>
        Subscriber * create_subscriber<Km2Filter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const Selection * muv_sel = rf.get_selection(
                    get_yaml<std::string>( instruct, "muv_sel" ) );

            std::string folder = get_folder( instruct, rf );

            std::string tree_name = get_yaml<std::string>( instruct, "tree_name" );

            const Km2Event * km2_event = get_km2_event( instruct, rf );

            return new Km2Filter( *sel, *km2_event, *muv_sel, tfile, folder, tree_name );
        }
}
