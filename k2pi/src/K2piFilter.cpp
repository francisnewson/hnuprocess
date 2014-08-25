#include "K2piFilter.hh"
#include "k2pi_functions.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "root_help.hh"
#include "NA62Constants.hh"

namespace fn
{
    REG_DEF_SUB( K2piFilter);

    K2piFilter::K2piFilter( 
            const Selection& sel, 
            TFile& tfile, std::string tree_name,
            const fne::Event * event,
            const K2piReco& k2pir, bool mc  )
        : Analysis( sel),
        event_( event),
        k2pir_( k2pir ),
        tfile_( tfile ),
        mc_ (mc)

    {
        //Create output tree
        ttree_ = new TTree( tree_name.c_str(), tree_name.c_str() );
        TTree::SetMaxTreeSize( 100000000LL );

        ttree_->Branch( "k2piVars", "fn::K2piVars", &vars_, 64000, 2 );
    }

    void K2piFilter::process_event()
    {
        const K2piRecoEvent& k2pirc = k2pir_.get_reco_event();

        //Fill reconstructed data variables
        vars_.data.neutral_vertex =  k2pirc.get_vertex();
        vars_.data.p4pi0 = k2pirc.get_p4pi0();
        vars_.data.p4pip = k2pirc.get_p4pip();

        ClusterData c1 = k2pirc.get_cluster1();
        ClusterData c2 = k2pirc.get_cluster2();

        vars_.data.E1 = c1.energy;
        vars_.data.E2 = c2.energy;

        vars_.data.pos1 = c1.position;
        vars_.data.pos2 = c2.position;

        vars_.chi2 = k2pirc.get_chi2();

        if ( mc_ ) 
        { 
            //Fill mc variables
            k2pi_mc_parts mcparts = extract_k2pi_particles( event_ );

            const fne::Mc& mc = event_->mc;
            vars_.mc.vertex =  mc.decay.decay_vertex;
            vars_.mc.p4pi0 =  mcparts.pi0->momentum;
            vars_.mc.p4pip =  mcparts.pip->momentum;
            vars_.mc.p4k =  mcparts.k->momentum;

            int nphotons = mcparts.photons.size();

            if ( nphotons > 0 )
            {
                vars_.mc.p4g1 = mcparts.photons[0]->momentum;
                if ( nphotons > 1 )
                {
                    vars_.mc.p4g2 = mcparts.photons[1]->momentum;
                }
            }
        }

        ttree_->Fill();

#if 0
        if ( vars_.data.p4pip.M2() > 0.05 )
        { print_event( std::cerr ); }
#endif

    }

    void K2piFilter::print_event( std::ostream& os )
    {
        const K2piRecoEvent& k2pirc = k2pir_.get_reco_event();

        os << "\nEvent with " 
            << event_->detector.nclusters  << " clusters."
            << std::endl;
        os << "DATA: "
            << "pos1: " << vars_.data.pos1
            << " pos2: " << vars_.data.pos2;

        if ( k2pirc.found_track_cluster() )
        {
            os << " pippos: " 
                << k2pirc.get_track_cluster().position;
        }

        Track t1( vars_.mc.vertex, vars_.mc.p4g1.Vect() );
        Track t2( vars_.mc.vertex, vars_.mc.p4g2.Vect() );

        Track tpip( vars_.mc.vertex, vars_.mc.p4pip.Vect() );

        TVector3 p1 = t1.extrapolate( na62const::zLkr );
        TVector3 p2 = t2.extrapolate( na62const::zLkr );
        TVector3 ppip = tpip.extrapolate( na62const::zLkr );

        os << "\n  MC: "
            << "pos1: " <<  p1 << " pos2: " <<  p2 
            << " pippos: " << ppip << "\n";

        const SingleTrack& st = k2pir_.get_single_track();
        const SingleRecoTrack& srt = st.get_single_track();
        TVector3 psrt = srt.extrapolate_ds( na62const::zLkr );

        os << "SingleTrack: " << psrt << "\n" <<  std::endl;
    }

    void K2piFilter::end_processing()
    {
        tfile_.cd();
        ttree_->Write();
    }

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<K2piFilter>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            const fne::Event * event = rf.get_event_ptr();

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string tree_name = get_yaml<std::string>
                ( instruct, "treename") ;

            const K2piReco * k2pi_reco = get_k2pi_reco( instruct, rf );

            bool mc = rf.is_mc();

            return new K2piFilter
                ( *sel, tfile, tree_name, event, *k2pi_reco, mc );
        }
}
