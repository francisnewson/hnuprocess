#include "km2_selections.hh"
#include "yaml_help.hh"
#include "NA62Constants.hh"
#include "tracking_selections.hh"

namespace fn
{


    REG_DEF_SUB( Km2NoBadCluster );

    Km2NoBadCluster::Km2NoBadCluster( const Km2Clusters& km2c )
        :km2c_( km2c ){}

    bool Km2NoBadCluster::do_check() const
    {
        const Km2RecoClusters& km2rc  = km2c_.get_reco_clusters();
        return ( km2rc.bad_size() < 1 );
    }

    template<>
        Subscriber * create_subscriber<Km2NoBadCluster>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Km2Clusters* km2c  = get_km2_clusters( instruct, rf );
            return new Km2NoBadCluster( *km2c );
        }

    //--------------------------------------------------

    REG_DEF_SUB( Km2M2Miss);

    Km2M2Miss::Km2M2Miss( const Km2Event& km2_event,
            double min_m2, double max_m2)
        :km2_event_( km2_event ), 
        min_m2_( min_m2), max_m2_( max_m2)
    { }

    bool Km2M2Miss::do_check() const
    {
        const Km2RecoEvent & km2re = km2_event_.get_reco_event();
        double m2miss = km2re.get_m2miss();

        return (m2miss > min_m2_) && (m2miss < max_m2_) ;

    }

    template<>
        Subscriber * create_subscriber<Km2M2Miss>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            double min_m2 = 
                get_yaml<double>( instruct, "min_m2" );

            double max_m2 = 
                get_yaml<double>( instruct, "max_m2" );

            const auto * km2_event = get_km2_event( instruct, rf );

            return new Km2M2Miss( *km2_event, min_m2, max_m2 );
        }

    //--------------------------------------------------

    REG_DEF_SUB( Km2PM2Miss);

    Km2PM2Miss::Km2PM2Miss(  const Km2Event& km2_event,
            std::vector<rectangle> pm2_recs )
        :km2_event_( km2_event), area_cut_( pm2_recs )
    {}

    bool Km2PM2Miss::do_check() const
    {
        const Km2RecoEvent & km2re = km2_event_.get_reco_event();
        double m2miss = km2re.get_m2miss();
        double muon_mom = km2re.get_muon_mom();
        return area_cut_.allowed( { muon_mom, m2miss } );
    }

    template<>
        Subscriber * create_subscriber<Km2PM2Miss>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const auto * km2_event = get_km2_event( instruct, rf );

            std::string shape = get_yaml<std::string>( instruct, "shape" );

            if ( shape == "rectangles" )
            {
                std::vector<rectangle> recs=
                    get_yaml<std::vector<rectangle>>( instruct, "points" );

                return new Km2PM2Miss( *km2_event, recs );
            }
            else
            {
                throw std::runtime_error(
                        "Unknown TrackPZShape" );
            }
        }


    //--------------------------------------------------

    REG_DEF_SUB( Km2HotLkr );

    Km2HotLkr::Km2HotLkr( const  Km2Event& km2_event,
            const Km2Clusters& km2_clusters,
            std::vector<std::pair<int, int> > hot_lkr_cells )
        :km2_event_( km2_event), km2_clusters_( km2_clusters ), hot_lkr_cells_( hot_lkr_cells )
    {}

    bool Km2HotLkr::do_check() const
    {
        std::vector<std::pair<int, int> > cells_to_check;

        const Km2RecoEvent & km2re = km2_event_.get_reco_event();
        const SingleRecoTrack * srt = km2re.get_reco_track();
        TVector3 vLkr = srt->extrapolate_ds( na62const::zLkr );

        cells_to_check.push_back( get_cpd_cell_index( vLkr.X(), vLkr.Y() ) );

        const Km2RecoClusters& km2rc = km2_clusters_.get_reco_clusters();
        Km2RecoClusters::const_iterator begin = km2rc.all_begin();
        Km2RecoClusters::const_iterator end = km2rc.all_end();

        for ( auto iclus = begin ; iclus != end ;  ++iclus )
        {
            TrackProjCorrCluster corr_cluster( **iclus );
            TVector3 vClus = corr_cluster.get_pos();

            if ( ( vLkr - vClus).Mag() < 10 )
            {
                cells_to_check.push_back( get_cpd_cell_index((*iclus)->x, (*iclus)->y  ) );
            }
        }

        for ( auto & check_cell : cells_to_check )
        {
            for ( auto & hot_cell : hot_lkr_cells_ )
            {
                if ( check_cell == hot_cell )
                {
                    return false;
                }
            }
        }

        return true;
    }

    template<>
        Subscriber * create_subscriber<Km2HotLkr>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const auto& km2_event = get_km2_event( instruct, rf );
            const auto& km2_clusters = get_km2_clusters( instruct, rf );

            auto hot_lkr_cells = get_yaml<std::vector<std::pair<int, int> > >
                ( instruct, "hot_cells" );

            return new Km2HotLkr( *km2_event, *km2_clusters, hot_lkr_cells );
        }

    //--------------------------------------------------

    REG_DEF_SUB( Km2TrackClusterEP );

    Km2TrackClusterEP::Km2TrackClusterEP
        ( const Km2Clusters& km2c, const SingleTrack& st,
          double min_eop, double max_eop)
        :km2_clusters_( km2c), st_( st ),
        min_eop_( min_eop), max_eop_( max_eop)
    {}

    bool Km2TrackClusterEP::do_check() const
    {
        const Km2RecoClusters& km2rc = km2_clusters_.get_reco_clusters();
        if ( km2rc.associate_size() == 0 )
        {
            return true;
        }
        if ( km2rc.associate_size() != 1 )
        {
            throw std::runtime_error(
                    "Km2TrackClusterEP called with more than one cluster");
        }
        const fne::RecoCluster* tc = * (km2rc.associate_begin() );
        TrackProjCorrCluster tcep( *tc );
        double track_cluster_energy = tcep.get_energy();

        const SingleRecoTrack& srt = st_.get_single_track();
        double track_mom = srt.get_mom();
        double eop = track_cluster_energy / track_mom;

        return ( ( eop < max_eop_ ) && ( eop > min_eop_ ) );
    }

    template<>
        Subscriber * create_subscriber<Km2TrackClusterEP>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Km2Clusters* km2c  = get_km2_clusters( instruct, rf );
            const SingleTrack* st = get_single_track( instruct, rf );

            double min_eop = get_yaml<double>( instruct, "min_eop" );
            double max_eop = get_yaml<double>( instruct, "max_eop" );

            return new Km2TrackClusterEP( *km2c, *st, min_eop, max_eop );

        }
}
