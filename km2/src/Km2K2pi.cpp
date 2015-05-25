#include "Km2K2pi.hh"
#include "Km2Reco.hh"
#include "TFile.h"
#include "k2pi_reco_functions.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"

namespace fn
{
    double get_z_pipe( Track& t, double rpipe )
    {
        double dxdz = t.get_dxdz();
        double dydz = t.get_dydz();
        double d = dxdz + dydz;

        double dxdz2 = dxdz*dxdz;
        double dydz2 = dydz*dydz;
        double d2 = dxdz2 + dydz2;

        TVector3 pt = t.get_point();
        double x0 = pt.X();
        double y0 = pt.Y();
        double p02 = x0*x0 + y0*y0;

        double result =  - d /d2 + std::sqrt( 
                ( rpipe*rpipe - p02 )/ d2 + ( d*d )/(d2*d2) );

        return result + pt.Z();
    }

    Km2K2piClusters::Km2K2piClusters( std::string prefix, std::string name , HistStore& hs_,
            const ClusterCorrector& cluster_corrector, bool mc )
        :cluster_corrector_(cluster_corrector), mc_( mc )
    {
        h_cluster_t_z_ = hs_.MakeTH2D( Form( "h_%s_cluster_t_z", prefix.c_str()), 
                Form(  "Cluster t Z ( %s )", name.c_str() ), 
                1000, 0, 0.15, "t",  120, -2000, 10000, "z" );

        h_cluster_r_ = hs_.MakeTH1D( Form( "h_%s_cluster_r", prefix.c_str() ), 
                Form( "Cluster radius ( %s )", name.c_str() ),
                1000, 0, 300, "radius (cm)" );

        h_cluster_E_ = hs_.MakeTH1D( Form( "h_%s_cluster_E", prefix.c_str() ), 
                Form( "Cluster energy ( %s )", name.c_str() ),
                1000, 0, 100, "Energy (GeV)" );

        h_cluster_r_E_ = hs_.MakeTH2D( Form( "h_%s_cluster_r_E", prefix.c_str() ), 
                Form( "Cluster radius and energy ( %s )", name.c_str() ),
                100, 0, 300, "radius (cm)", 100, 0, 100, "Energy" );

        h_cluster_ds_E_ = hs_.MakeTH2D( Form( "h_%s_cluster_ds_E", prefix.c_str() ),  
                Form( "Track cluster separation and energy ( %s )", name.c_str() ),
                100, 0, 300, "ds (cm)", 100, 0, 100, "Energy" );

        h_cluster_zpipe_t_ = hs_.MakeTH2D( Form( "h_%s_cluster_zpipe_t", prefix.c_str() ),  
                Form( "Pipe interaction ( %s )", name.c_str() ),
                150, 0, 15000, "zpipe (cm)", 100, 0, 0.05, "t)" );

        h_cluster_ds_ds_ = hs_.MakeTH2D( Form( "h_%s_cluster_ds_ds", prefix.c_str() ),  
                Form( "dS for 2 clusters ( %s )", name.c_str() ),
                150, 0, 300, "ds (cm)", 150, 0, 300, "ds (cm)" );

        h_cluster_E_E_ = hs_.MakeTH2D( Form( "h_%s_cluster_E_E", prefix.c_str() ),  
                Form( "E for 2 clusters ( %s )", name.c_str() ),
                100, 0, 100, "E (cm)", 100, 0, 100, "E (cm)" );

        h_cluster_r_r_ = hs_.MakeTH2D( Form( "h_%s_cluster_r_r", prefix.c_str() ),  
                Form( "r for 2 clusters ( %s )", name.c_str() ),
                150, 0, 300, "r (cm)", 150, 0, 300, "r (cm)" );
    }

    void Km2K2piClusters::fill( const Km2RecoEvent& km2re, Km2RecoClusters::const_iterator start,
            Km2RecoClusters::const_iterator finish, double wgt )
    {
        double zvertex = km2re.get_zvertex();
        auto vertex = km2re.get_vertex();

        const auto srt = km2re.get_reco_track();

        std::vector<photon_properties> cluster_props;
        photon_properties props;

        for ( auto itclus =  start ; itclus != finish  ; ++itclus )
        {
            double x = (*itclus)->x;
            double y = (*itclus)->y;
            double r = std::hypot( x, y );


            CorrCluster cc( **itclus,  cluster_corrector_, mc_ );
            PhotonProjCorrCluster photon_cluster{cc};
            double photon_angle = (photon_cluster.get_pos() - vertex ).Theta();
            double energy = photon_cluster.get_energy();

            TVector3 track_lkr = srt->extrapolate_ds( photon_cluster.get_pos().Z() );
            double ds = (track_lkr - photon_cluster.get_pos() ).Mag();


            Track photon_track{ vertex, photon_cluster.get_pos() - vertex };

            double z_pipe = get_z_pipe( photon_track, 15.2 ); 

            h_cluster_t_z_->Fill( photon_angle, zvertex, wgt );
            h_cluster_r_->Fill( r, wgt );

            h_cluster_r_E_->Fill( r, energy, wgt );
            h_cluster_ds_E_->Fill( ds, energy, wgt );
            h_cluster_E_->Fill( energy, wgt );

            h_cluster_zpipe_t_->Fill( z_pipe, photon_angle, wgt );

            props.r = r;
            props.ds = ds;
            props.E = energy;
            props.t = photon_angle;
            props.z = zvertex;

            cluster_props.push_back( props );
        }

        if ( cluster_props.size() >=2 )
        {

            sort( begin(cluster_props), end( cluster_props ), 
                    []( const photon_properties& lhs, const photon_properties& rhs )
                    { return lhs.E < rhs.E ; } );

            h_cluster_ds_ds_->Fill(cluster_props[0].ds, cluster_props[1].ds, wgt );
            h_cluster_E_E_->Fill(cluster_props[0].E, cluster_props[1].E, wgt );
            h_cluster_r_r_->Fill(cluster_props[0].r, cluster_props[1].r, wgt );
        }

    }

    REG_DEF_SUB( Km2K2pi);

    Km2K2pi::Km2K2pi( const Selection& sel, bool mc,
            const Km2Event& km2e,  const fne::Event * e, 
            const  Km2Clusters& km2_clusters,
            const ClusterCorrector& cluster_corrector,
            TFile& tfile, std::string folder )
        :Analysis( sel ), mc_( mc ), km2e_( km2e ), e_( e ),
        km2_clusters_( km2_clusters ), cluster_corrector_( cluster_corrector ),
        tfile_( tfile ), folder_( folder ),
        bad_clusters_( "bad", "Bad", hs_, cluster_corrector_, mc ),
        all_clusters_( "all", "All", hs_, cluster_corrector_, mc )

    {
        h_mc_photon_t_z_ = hs_.MakeTH2D( "h_mc_photon_t_z", "MC Photon t Z", 
                1000, 0, 0.15, "t",  120, -2000, 10000, "z" );

        h_mc_photon_r_ = hs_.MakeTH1D( "h_mc_photon_r", "MC Photon radius",
                1000, 0, 300, "radius (cm)" );

        h_mc_photon_r_E_ = hs_.MakeTH2D( "h_mc_photon_r_E", "MC Photon radius and energy",
                100, 0, 300, "radius (cm)", 100, 0, 100, "Energy" );

        h_mc_photon_ds_E_ = hs_.MakeTH2D( "h_mc_photon_ds_e", 
                "mc track cluster separation and energy",
                100, 0, 300, "ds (cm)", 100, 0, 100, "energy" );

        h_mc_photon_ds_vs_ds_ = hs_.MakeTH2D( "h_mc_photon_ds_vs_ds", 
                "mc track cluster separation correlation",
                150, 0, 300, "ds (cm)", 150, 0, 300, "ds (cm)" );

        h_mc_photon_r_vs_r_ = hs_.MakeTH2D( "h_mc_photon_r_vs_r", 
                "mc track cluster radius correlation",
                150, 0, 300, "r (cm)", 150, 0, 300, "r (cm)" );

        h_mc_photon_phi_vs_phi_ = hs_.MakeTH2D( "h_mc_photon_phi_vs_phi", 
                "Photon azimuthal positions",
                100, 0, 2 * 3.141592, "phi (rad)",
                100, 0, 2 * 3.141592, "phi (rad)" );

        h_mc_photon_dphi_p_ = hs_.MakeTH2D( "h_mc_photon_dphi_p_", 
                "Photon azimuthal positions vs pion momentum",
                100, 0, 100 , "p (GeV)",
                100, 0, 6, "phi (rad)" );

        h_mc_photon_E_vs_E_ = hs_.MakeTH2D( "h_mc_photon_E_vs_E", 
                "mc track cluster energy correlation",
                100, 0, 100, "energy (GeV)", 100, 0, 100, "energy (GeV)" );

        h_mc_photon_n_ = hs_.MakeTH1D( "h_mc_photon_n", "MC Photon count",
                5, -0.5, 4.5, "n photons" );

        h_mc_photon_dphi_ = hs_.MakeTH1D( "h_mc_photon_dphi", "MC Photon delta azimuth",
                100, 0,  4, "dphi" );

    }

    void Km2K2pi::process_event()
    {
        const Km2RecoEvent& km2re  = km2e_.get_reco_event();
        const Km2RecoClusters& km2rc = km2_clusters_.get_reco_clusters();
        double wgt = get_weight();

        bad_clusters_.fill( km2re, km2rc.bad_begin(), km2rc.bad_end(), wgt );
        all_clusters_.fill( km2re, km2rc.all_begin(), km2rc.all_end(), wgt );

        if ( mc_ )
        { plot_mc( *e_, wgt ); }
    }

    void Km2K2pi::plot_mc( const fne::Event& e, double wgt )
    {

        boost::optional<k2pi_mc_parts> opt_mc_parts = extract_k2pi_particles( &e );

        if ( !opt_mc_parts ){ return; }

        k2pi_mc_parts& mc_parts = *opt_mc_parts;

        if ( ! ( mc_parts.pip ) ){ return; }

        const fne::McParticle * pion = mc_parts.pip;
        TVector3 pion_vertex = pion->production_vertex;
        TVector3 pion_mom = pion->momentum.Vect();
        Track pion_track{ pion_vertex, pion_mom };
        TVector3 pion_impact = pion_track.extrapolate( na62const::zLkr );

        std::vector<photon_properties> photon_props;
        photon_properties props;

        h_mc_photon_n_->Fill( mc_parts.photons.size() , wgt );

        for ( const auto& photon : mc_parts.photons )
        {
            const auto vertex = photon->production_vertex;
            const auto momentum = photon->momentum;
            double energy = momentum.E();

            Track photon_track{ vertex, momentum.Vect() };
            TVector3 photon_impact = photon_track.extrapolate( na62const::zLkr );
            double ds = ( pion_impact - photon_impact ).Mag() ;

            props.E = momentum.E();
            props.ds = ( pion_impact - photon_impact ).Mag() ;
            props.r = photon_impact.Perp();
            props.z = vertex.Z();
            props.t =  momentum.Theta();
            props.phi = photon_impact.Phi();

            h_mc_photon_t_z_->Fill( props.t , props.z, wgt );
            h_mc_photon_r_->Fill( props.r , wgt );
            h_mc_photon_r_E_->Fill(props.r, props.E,  wgt );
            h_mc_photon_ds_E_->Fill( props.ds, props.E, wgt );

            photon_props.push_back(props);
        }

        if ( photon_props.size() >= 2 )
        {
            std::reverse( begin(photon_props), end( photon_props) );
            h_mc_photon_E_vs_E_->Fill( photon_props[0].E, photon_props[1].E, wgt );
            h_mc_photon_r_vs_r_->Fill( photon_props[0].r, photon_props[1].r, wgt );
            h_mc_photon_ds_vs_ds_->Fill( photon_props[0].ds, photon_props[1].ds, wgt );
            h_mc_photon_phi_vs_phi_->Fill( photon_props[0].phi, photon_props[1].phi, wgt );

            double dphi = fabs(photon_props[0].phi- photon_props[1].phi );
            if (dphi > na62const::pi )
            {
                dphi = 2 * na62const::pi - dphi;
            }

            h_mc_photon_dphi_->Fill( dphi, wgt );
            h_mc_photon_dphi_p_->Fill( pion->momentum.P(), dphi, wgt );
            h_mc_photon_dphi_->Fill( dphi, wgt );
        }
    }

    void Km2K2pi::end_processing()
    {
        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<Km2K2pi>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            const fne::Event * e = rf.get_event_ptr();

            bool mc  = rf.is_mc();

            const Km2Clusters* km2c  = get_km2_clusters( instruct, rf );
            const ClusterCorrector * cluster_corrector = get_cluster_corrector( instruct, rf );

            return new Km2K2pi( *sel, mc, *km2_event, e, *km2c, *cluster_corrector, tfile, folder );
        }
}
