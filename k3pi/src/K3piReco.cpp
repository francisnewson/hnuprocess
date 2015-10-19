#include "K3piReco.hh"
#include "Event.hh"
#include "yaml_help.hh"
#include "RecoFactory.hh"
#include "RecoCluster.hh"
#include "RecoVertex.hh"
#include "RecoTrack.hh"
#include "CorrCluster.hh"
#include "NA62Constants.hh"
#include "KaonTrack.hh"
#include <algorithm>
#include <cassert>
#include "tracking_functions.hh"

namespace fn
{

    void K3piRecoEvent::update( int charge, double chi2,
            double min_dch1_sep, double max_dy_dch4, double max_eop, 
            TLorentzVector p4_total, TVector3 vertex, TVector3 kaon_mom )
    {
        charge_ = charge;
        chi2_ = chi2;
        min_dch1_sep_ = min_dch1_sep;
        max_dy_dch4_ = max_dy_dch4;
        max_eop_ = max_eop;
        p4_total_ = p4_total;
        vertex_ = vertex;
        kaon_mom_ = kaon_mom;
    }
            int K3piRecoEvent::get_charge() const { return charge_; }
            double K3piRecoEvent::get_chi2() const{ return chi2_; }
            double K3piRecoEvent::get_min_dch1_sep() const { return min_dch1_sep_ ; }
            double K3piRecoEvent::get_max_dy_dch4_sep() const { return max_dy_dch4_; }
            double K3piRecoEvent::get_max_eop() const { return max_eop_; }
            double K3piRecoEvent::get_pt() const { return p4_total_.Vect().Perp( kaon_mom_ ); }
            double K3piRecoEvent::get_mom() const{ return p4_total_.P(); }
            double K3piRecoEvent::get_z_vertex() const{ return vertex_.Z(); }
            TVector3 K3piRecoEvent::get_kaon_mom() const{ return kaon_mom_; }

            double K3piRecoEvent::get_invariant_mass2() const
            {
                return p4_total_.M2();
            }

    //--------------------------------------------------

    REG_DEF_SUB( K3piReco );

    K3piReco::K3piReco( const fne::Event * e, int charge,
            const KaonTrack& kt, const ClusterCorrector& cc )
        :e_( e ), kt_( kt ),cc_( cc), req_charge_( charge )
    {
    }

    //Calculation is delgated to descendents
    void K3piReco::new_event()
    {
        dirty_ = true;
    }

    bool K3piReco::reconstructed_k3pi() const
    {
        if ( dirty_ )
        {
            found_ = process_event();
            dirty_ = false;
        }
        assert( !dirty_ );
        return found_;
    }

    const K3piRecoEvent& K3piReco::get_k3pi_reco_event() const
    {
        if ( dirty_ )
        {
            found_ =   process_event();
            dirty_ = false;
        }
        if ( !found_ )
        {
            throw Xcept<EventDoesNotContain>( "K3piEvent");
        }
        assert( !dirty_ );
        assert( found_);
        return re_;
    }

    bool K3piReco::process_event() const
    {
        std::cerr << "processing k3pi" << std::endl;
        std::cerr << e_->detector.nvertexs << std::endl;

        //require 1 verte8
        if ( e_->detector.nvertexs != 1 ){ return false; }

        std::cerr << "found 1 vtx" << std::endl;

        fne::RecoVertex * rv = static_cast<fne::RecoVertex*>
            ( e_->detector.vertexs.At( 0 ) );

        //require 3 tracks
        if ( rv->nvertextracks != 3 ) {return false; }

        std::cerr << "found 3 trkcs" << std::endl;

        //check charge
        int charge = rv->charge;
        if ( ( req_charge_) != 0 && (req_charge_ != charge) ){ return false; }

        std::cerr << "good charge" << std::endl;

        //extract track momenta and dch impact points
        std::vector<TLorentzVector> momenta;
        std::vector<TVector3> dch1_positions;
        std::vector<fne::RecoTrack*> tracks;

        for ( unsigned int it = 0 ; it != 3 ; ++it )
        {
            fne::RecoVertexTrack * rvt = 
                static_cast<fne::RecoVertexTrack*>( rv->vertex_tracks.At(it) );
            int track_id = rvt->iTrack;

            fne::RecoTrack * rt = static_cast<fne::RecoTrack*>( e_->detector.tracks.At( track_id ) );
            assert( rt->id == track_id );
            tracks.push_back( rt );

            double alpha = e_->conditions.alpha;
            double beta = e_->conditions.beta;
            double corr_mom = p_corr_ab( rt->p, rt->q, alpha, beta );

            TVector3 pion_3mom{ rvt->bdxdz, rvt->bdydz, 1 };
            pion_3mom.SetMag( corr_mom );
            double pion_energy = std::hypot( corr_mom, na62const::mPi);

            momenta.push_back( TLorentzVector{ pion_3mom, pion_energy } );

            Track us_track{ {rt->bx, rt->by, na62const::bz_tracking},
                { rt->bdxdz, rt->bdydz, 1 } };

            dch1_positions.push_back( us_track.extrapolate( na62const::zDch1 ) );
        }

        assert( momenta.size() == 3 );
        assert( dch1_positions.size() == 3 );

        //if ( rv->chi2 > 40 ){ return ; }
        double chi2 = rv->chi2;

        //Find min track separation at DCH1
        double min_dch1_sep = std::numeric_limits<double>::max();

        for ( unsigned int opos =  0 ; opos != 3 ; ++opos )
        {
            for ( unsigned int ipos = opos+1  ; ipos != 3 ; ++ipos )
            {
                TVector3 sep = dch1_positions[opos] - dch1_positions[ipos] ;
                if ( sep.Mag() < min_dch1_sep )
                { min_dch1_sep = sep.Mag() ; }
            }
        }

        //find max track deflection in y at DCH4
        double max_dy_dch4 = 0;
        for( const auto& rt : tracks )
        {
            double dy = y_shift_dch4( *rt );

            if ( dy > max_dy_dch4 )
            {
                max_dy_dch4 = dy;
            }
        }

        //if ( max_dy_dch4 > 0.6 ){ return ; }


        TLorentzVector total_4mom = std::accumulate(
                begin( momenta ), end( momenta), TLorentzVector{} );

        double invariant_mass2 = total_4mom.M2();
        double transverse_mom = total_4mom.Vect().Pt( kt_.get_kaon_3mom() );

        //if	( pow(transverse_mom,2) > 0.001 ) { return; }


        //clusters for eop
        double max_eop = 0;
        int nclusters = e_->detector.nclusters;
        for ( int ic = 0 ; ic != nclusters ; ++ic )
        {
            fne::RecoCluster * rc = static_cast<fne::RecoCluster*>
                ( e_->detector.clusters.At( 0 ) );

            int track_id = rc->iTrack;

            for ( auto& reco_track : tracks )
            {
                if ( reco_track->id == track_id )
                {
                    CorrCluster corr_cluster{ *rc, cc_, false };
                    double eop = corr_cluster.get_energy() / reco_track->p;
                    if ( eop > max_eop )
                    {
                        max_eop = eop;
                    }
                }
            }
        }
        re_.update( charge, chi2, min_dch1_sep, max_dy_dch4, max_eop, total_4mom, 
                {rv->x, rv->y, rv->z}, kt_.get_kaon_3mom() );

        return true;
    }

    void K3piReco::end_processing() {} 

    template<>
        Subscriber * create_subscriber<K3piReco>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const  fne::Event * e = rf.get_event_ptr();

            const KaonTrack * kt = get_kaon_track( instruct, rf );

            const ClusterCorrector * cc = get_cluster_corrector( instruct, rf );

            int charge = get_yaml_default<int>( instruct, "charge", 0 );

            return new K3piReco{ e, charge,  *kt, *cc};
        }  

    //Calculate dch4 y shift
    double y_shift_dch4( fne::RecoTrack& rt )
    {
        TVector3 bpoint( rt.bx, rt.by,na62const::bz_tracking );
        TVector3 bdir( rt.bdxdz, rt.bdydz, 1 );
        Track btrack{ bpoint, bdir };

        TVector3 point( rt.x, rt.y, na62const::z_tracking );
        TVector3 dir( rt.dxdz, rt.dydz, 1 );
        Track track{ point, dir };

        TVector3 bdch4 = btrack.extrapolate( na62const::zDch4 );
        TVector3 dch4 = track.extrapolate( na62const::zDch4 );

        double dch_sep = fabs( bdch4.Y() -dch4.Y() );
        return dch_sep;
    }

    template  <>
        K3piReco * get_sub<K3piReco>( YAML::Node& instruct, RecoFactory& rf)
        {
            return get_sub<K3piReco>( instruct, rf, "k3pi_reco" );
        }


    //--------------------------------------------------

    processing_track  correct_track( 
            const fne::RecoTrack * rt, const fne::Event * e, const KaonTrack& kt )
    {
        processing_track result;

        result.rt_ = *rt;
        result.rt = &result.rt_;
        result.orig_rt = rt;

        const double& alpha = e->conditions.alpha;
        const double& beta = e->conditions.beta;
        const double& dch_toffst = e->conditions.dch_toffst;

        result.corr_mom = p_corr_ab( result.rt->p, result.rt->q, alpha, beta );
        result.unscattered_mom  = result.corr_mom;
        result.adjusted_time = result.rt->time - dch_toffst;

        const Track& kaon_track = kt.get_kaon_track();
        Track result_track = get_bz_track( *result.rt );
        result.vert = compute_cda(result_track, kaon_track );

        //UNFINISHED FUNCTION!!!!!!


        return result;
    }
}
