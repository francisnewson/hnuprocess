#include "TimingOffsets.hh"
#include "Event.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"
#include "RecoTrack.hh"
#include "RecoCluster.hh"
namespace fn
{
    REG_DEF_SUB( TimingOffsets);

    TimingOffsets::TimingOffsets( const Selection& sel, const fne::Event * e ,
            TFile& tfile, std::string folder)
        :Analysis( sel), e_( e ), tfile_( tfile), folder_( folder )
    {
        h_track_time_ = hs_.MakeTH1D( "h_track_time", "DCH Track Time",
                1000, -200, 200, "time (ns)" );

        h_cluster_time_ = hs_.MakeTH1D( "h_cluster_time", "LKr Cluster Time",
                1000, -200, 200, "time (ns)" );

        h_track_cluster_time_ = hs_.MakeTH1D( "h_track_cluster_time",
                "DCH Track - LKr Cluster Time",
                1000, -200, 200, "time (ns)" );
    }

    void TimingOffsets::process_event()
    {
        auto etracks = e_->detector.tracks;
        int ntracks = e_->detector.ntracks;

        for ( int i  = 0 ; i != ntracks ; ++i )
        {
            fne::RecoTrack * rt = static_cast<fne::RecoTrack*>( etracks[i] );
            h_track_time_->Fill( rt->time );
        }

        auto eclusters = e_->detector.clusters;
        int nclusters = e_->detector.nclusters;

        for ( int i  = 0 ; i != nclusters ; ++i )
        {
            fne::RecoCluster * rc = static_cast<fne::RecoCluster*>( eclusters[i] );
            h_cluster_time_->Fill( rc->time );
        }

        for ( int i  = 0 ; i != ntracks ; ++i )
            for ( int j  = 0 ; j != nclusters ; ++j )
            {
                fne::RecoTrack * rt = static_cast<fne::RecoTrack*>( etracks[i] );
                fne::RecoCluster * rc = static_cast<fne::RecoCluster*>( eclusters[j] );

                h_track_cluster_time_->Fill( rt->time - rc->time );
            }

    }

    void TimingOffsets::end_processing()
    {
        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<TimingOffsets>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const fne::Event * e = rf.get_event_ptr();

            return new TimingOffsets( *sel, e, tfile, folder );
        }
}
