#include "K2piFilter.hh"
#include "k2pi_functions.hh"
#include "k2pi_extract.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "root_help.hh"
#include "NA62Constants.hh"
#include <iomanip>
#include <cmath>

namespace fn
{
    REG_DEF_SUB( K2piFilter);

    K2piFilter::K2piFilter( 
            const Selection& sel, 
            TFile& tfile, std::string tree_name,
            const fne::Event * event,
            const K2piReco& k2pir, 
            const KaonTrack& kt,
            bool mc  )
        : Analysis( sel),
        event_( event),
        k2pir_( k2pir ),
        kt_( kt),
        tfile_( tfile ),
        mc_ (mc)
    {
        //Create output tree
        ttree_ = new TTree( tree_name.c_str(), tree_name.c_str() );
        TTree::SetMaxTreeSize( 10000000000LL );

        ttree_->Branch( "k2piVars", "fn::K2piVars", &vars_, 64000, 2 );
    }

    void K2piFilter::process_event()
    {
        const K2piRecoEvent& k2pirc = k2pir_.get_reco_event();
        const SingleTrack& st = k2pir_.get_single_track();
        const SingleRecoTrack& srt = st.get_single_track();

        k2pi_extract( mc_, get_weight(), event_, k2pirc, srt, kt_, vars_ );

        ttree_->Fill();

#if 0
        double dZ = vars_.data.neutral_vertex.Z() - vars_.mc.vertex.Z() ;

        if (  fabs( dZ ) > 200 )
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
            << "pos1: " << vars_.data.pos1 << " " <<  std::setw( 7) << vars_.data.E1
            << " pos2: " << vars_.data.pos2 << " " << std::setw( 7) <<  vars_.data.E2;

        if ( k2pirc.found_track_cluster() )
        {
            os << " pippos: " 
                << k2pirc.get_track_cluster().position;
        }

        Track t1( vars_.mc.vertex, vars_.mc.p4g1.Vect() );
        Track t2( vars_.mc.vertex, vars_.mc.p4g2.Vect() );

        Track tpip( vars_.mc.vertex, vars_.mc.p4pip.Vect() );

        TVector3 p1 = t1.extrapolate( vars_.data.pos1.Z() );
        TVector3 p2 = t2.extrapolate( vars_.data.pos2.Z() );
        TVector3 ppip = tpip.extrapolate( na62const::zLkr );

        os << "\n  MC: "
            << "pos1: " <<  p1  << " " <<  std::setw( 7) << vars_.mc.p4g1.E()
            << " pos2: " <<  p2 << " " <<  std::setw( 7) << vars_.mc.p4g2.E()  
            << " pippos: " << ppip << "\n";

        const SingleTrack& st = k2pir_.get_single_track();
        const SingleRecoTrack& srt = st.get_single_track();
        TVector3 psrt = srt.extrapolate_ds( na62const::zLkr );

        os << "SingleTrack: " << psrt << "\n";
        os <<  "MC Vertex: " << vars_.mc.vertex << " " 
            << "RecoVertex: " << vars_.data.neutral_vertex << "\n";

        os << "MC kaon: " << vars_.mc.p4k << " " << "Data kaon:" << kt_.get_kaon_4mom() << std::endl;

        os << std::endl;
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

            const KaonTrack * kt = get_kaon_track( instruct, rf );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string tree_name = get_yaml<std::string>
                ( instruct, "treename") ;

            const K2piReco * k2pi_reco = get_k2pi_reco( instruct, rf );

            bool mc = rf.is_mc();

            return new K2piFilter
                ( *sel, tfile, tree_name, event, *k2pi_reco, *kt, mc );
        }
}
