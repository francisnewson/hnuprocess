#include "SingleTrack.hh"
#include "NA62Constants.hh"
#include "Xcept.hh"
#include "tracking_functions.hh"
#include "yaml_help.hh"
#include <random>

namespace fn
{
    //SingleTrack results cacheing
    //Calculation is delgated to descendents
    void SingleTrack::new_event()
    {
        dirty_ = true;

        BOOST_LOG_SEV( get_log() , log_level() )
            << "In SingleTrack event";
    }

    bool SingleTrack::found_single_track() const
    {
        if ( dirty_ )
        {
            found_ = process_event();
            dirty_ = false;
        }
        assert( !dirty_ );
        return found_;
    }

    const SingleRecoTrack& SingleTrack::get_single_track() const
    {
        if ( dirty_ )
        {
            found_ =   process_event();
            dirty_ = false;
        }
        if ( !found_ )
        {
            throw Xcept<EventDoesNotContain>( "SingleTrack");
        }
        assert( !dirty_ );
        assert( found_);
        return *single_reco_track_;
    }

    void SingleTrack::set_reco_track( const SingleRecoTrack * srt )
    {
        single_reco_track_ = srt;
    }

    REG_DEF_SUB( SingleTrack );

    //Factory creation for SingleTrack descendents
    template<>
        Subscriber * create_subscriber<SingleTrack>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = instruct["method"].as<std::string>();
            const fne::Event * event = rf.get_event_ptr();
            KaonTrack * kt = get_kaon_track( instruct, rf );

            if ( method == "BF" )
            {
                return new BFSingleTrack{ event, instruct, *kt } ;
            }
            else if ( method == "BFScatter" )
            {
                return new BFScatterSingleTrack{ event, instruct, *kt };
            }
            else
            {
                throw Xcept<UnknownSingleTrackMethod>( method );
            }
        }

    //--------------------------------------------------

    BFSingleRecoTrack::BFSingleRecoTrack()
        :bfc_( global_BFCorrection() )
    {}

    void BFSingleRecoTrack::update(  
            const processing_track * proc_track, 
            const fne::Event * event )
    {
        proc_track_ = proc_track;
        bf_track_ = bfc_.compute_bf_track
            ( event, proc_track_->rt, proc_track_->vert );
    }

    int BFSingleRecoTrack::get_charge() const
    {
        return proc_track_->rt->q;
    }

    TVector3 BFSingleRecoTrack::get_3mom() const
    {
        return proc_track_->corr_mom * bf_track_.get_direction().Unit();
    }

    double BFSingleRecoTrack::get_mom() const
    {
        return proc_track_->corr_mom;
    }

    TVector3 BFSingleRecoTrack::get_vertex() const
    {
        return proc_track_->vert.point;
    }

    double BFSingleRecoTrack::get_cda() const
    {
        return proc_track_->vert.cda;
    }

    double BFSingleRecoTrack::get_time() const
    {
        return proc_track_->rt->time;
    }

    double BFSingleRecoTrack::get_quality() const
    {
        return proc_track_->rt->quality;
    }

    double BFSingleRecoTrack::get_ddead_cell() const
    {
        return proc_track_->rt->dDeadCell;
    }

    TVector3 BFSingleRecoTrack::extrapolate_ds( double z) const
    {
        return extrapolate_z( * proc_track_->rt, z );
    }

    TVector3 BFSingleRecoTrack::extrapolate_us( double z) const
    {
        return extrapolate_bz( * proc_track_->rt, z );
    }

    TVector3 BFSingleRecoTrack::extrapolate_bf( double z) const
    {
        return bf_track_.extrapolate( z );
    }

    TVector3 BFSingleRecoTrack::get_us_mom() const
    {
        const fne::RecoTrack& rt = *proc_track_->rt;
        TVector3 usmom{ rt.bdxdz, rt.bdydz, 1};
        return proc_track_->corr_mom  * usmom.Unit();
    }

    TVector3 BFSingleRecoTrack::get_ds_mom() const
    {
        const fne::RecoTrack& rt = *proc_track_->rt;
        TVector3 dsmom{ rt.dxdz, rt.dydz, 1};
        return proc_track_->corr_mom  * dsmom.Unit();
    }

    //--------------------------------------------------

    //BF

    processing_track::processing_track( const processing_track& other )
        :corr_mom( other.corr_mom ), vert( other.vert ), good( other.good ),
        rt_( other.rt_ ), rt( &rt_ )
    {}

    void swap
        ( processing_track& first, processing_track& second )
        {
            using std::swap;
            swap( first.corr_mom, second.corr_mom );
            swap( first.vert, second.vert );
            swap( first.good, second.good );
            swap( first.rt_, second.rt_ );
            first.rt = & first.rt_;
            second.rt = & second.rt_;
        }

    processing_track& processing_track::operator= ( processing_track other )
    {
        swap( *this, other );
        return *this;
    }

    BFSingleTrack::BFSingleTrack( const fne::Event * event, 
            YAML::Node& instruct, KaonTrack& kt)
        :event_( event ), kt_( kt )
    {
        //Load BF single track parameters
        try
        {
            init_min_mom_ = instruct["init_min_mom"].as<double>();
            init_max_mom_ = instruct["init_max_mom"].as<double>();

            init_max_cda_ = instruct["init_max_cda"].as<double>();

            init_min_z_ = instruct["init_min_z"].as<double>();
            init_max_z_ = instruct["init_max_z"].as<double>();

            dch_1_merge_sep_ = instruct["dch_merge_sep"].as<double>();
        }
        catch( YAML::ParserException& e )
        {
            std::cerr << e.what() << "\n";
            throw ;
        }

        //Set up result pointer of base class
        set_reco_track( &single_reco_track_ );
    }

    //Identify and calculate single track
    bool BFSingleTrack::process_event() const
    {
        int ntracks = event_->detector.ntracks;
        auto& etracks = event_->detector.tracks;

        const double& alpha = event_->conditions.alpha;
        const double& beta = event_->conditions.beta;

        BOOST_LOG_SEV( get_log() , log_level() )
            << "BFST: ntracks " << ntracks;

        if ( ntracks < 1 )
        { return false; }

        //Collect tracks in a vector
        proc_tracks_.clear();
        processing_track pt;

        //Look for tracks within initial momentum range
        for ( int itrk = 0 ; itrk != ntracks ; ++itrk )
        {
            //copy reco track ( and point to it )
            fne::RecoTrack * rt = static_cast<fne::RecoTrack*>( etracks[itrk] );
            pt.rt_ =  *rt;
            pt.rt = &pt.rt_;

            assert( pt.rt != rt );
            assert( pt.rt->p == rt->p );

            modify_processing_track( pt );

            pt.corr_mom = p_corr_ab( pt.rt->p, pt.rt->q, alpha, beta  );
            pt.good = true;

            BOOST_LOG_SEV( get_log() , log_level() )
                << "BFST:  corr_mom " << pt.corr_mom;

            if ( pt.corr_mom < init_min_mom_ ){ continue; }
            if ( pt.corr_mom > init_max_mom_ ){ continue; }

            proc_tracks_.push_back( pt );
        }

        if ( proc_tracks_.size() < 1 ){ return false; }

        BOOST_LOG_SEV( get_log() , log_level() )
            << "BFST: passed init " << ntracks;

        //Loop over pairs of tracks
        for ( auto otrk = proc_tracks_.begin() ; 
                otrk != proc_tracks_.end() ; ++ otrk )
        {
            auto & ot = * otrk;

            for ( auto itrk = proc_tracks_.begin() ; 
                    itrk != otrk ; ++ itrk )
            {
                auto & it = * itrk;

                //Separation at DCH1
                TVector3 dch1_sep = extrapolate_bz( *ot.rt, na62const::zDch1 ) -
                    extrapolate_bz( *it.rt, na62const::zDch1 );

                if ( dch1_sep.Mag()  > dch_1_merge_sep_ )
                { continue; }

                //For nearby tracks,
                //mark lower quality as bad
                if ( it.rt->quality < ot.rt->quality )
                { 
                    it.good = false;
                    continue;
                }

                if ( it.rt->quality > ot.rt->quality )
                {
                    ot.good = false; 
                    continue;
                }

                //Equal quality, check delta y

                double idy = fabs(  (
                            extrapolate_bz( *it.rt, na62const::zDch1 )
                            - extrapolate_z( *it.rt, na62const::zDch4 ) ).Y() );

                double ody = fabs(  (
                            extrapolate_bz( *ot.rt, na62const::zDch1 )
                            - extrapolate_z( *ot.rt, na62const::zDch4 ) ).Y() );

                if ( idy > ody )
                { it.good = false; }
                else
                { ot.good = false; }
            }
        } //End loop over pairs

        BOOST_LOG_SEV( get_log() , log_level() )
            << "BFST: passed pairs " << ntracks;

        auto& conditions = event_->conditions;
        auto& dch_toffst = conditions.dch_toffst ;

        //Set other bad criteria
        for ( auto& pt : proc_tracks_ )
        {

            //ignore tracks which are already bad
            if ( !pt.good )
            { continue; }

            //out of time tracks are bad
            if ( fabs( pt.rt->time - dch_toffst ) > 62.5 )
            {
                BOOST_LOG_SEV( get_log() , log_level() )
                    << "BFST: bad time ";
                pt.good = false;
                continue;
            }

            Track pt_track = get_bz_track( *pt.rt );
            const Track& kaon_track = kt_.get_kaon_track();

            //extract raw vertex and cda
            pt.vert  = compute_cda(
                    pt_track, kaon_track );

            //raw cda requirement
            if ( pt.vert.cda > init_max_cda_ )
            {
                BOOST_LOG_SEV( get_log() , log_level() )
                    << "BFST: bad cda " << pt.vert.cda  
                    << " > " << init_max_cda_;
                pt.good = false;
            }

            //raw vertex range
            if ( pt.vert.point.Z() < init_min_z_ )
            {
                BOOST_LOG_SEV( get_log() , log_level() )
                    << "BFST: bad min z ";
                pt.good = false;
            }

            if ( pt.vert.point.Z() > init_max_z_ )
            {
                BOOST_LOG_SEV( get_log() , log_level() )
                    << "BFST: bad max z ";
                pt.good = false;
            }
        }

        BOOST_LOG_SEV( get_log() , log_level() )
            << "BFST: done checks ";

        //remove bad tracks
        proc_tracks_.erase(
                std::remove_if ( begin( proc_tracks_ ), end( proc_tracks_),
                    []( const processing_track& pt){ return !(pt.good ); } ),
                proc_tracks_.end() );

        if ( proc_tracks_.size() != 1 )
        {
            BOOST_LOG_SEV( get_log() , log_level() )
                << "BFST: BAD size " << proc_tracks_.size();
            return false;
        }

        BOOST_LOG_SEV( get_log() , log_level() )
            << "BFST: Passed! computing BF " << ntracks;

        //Compute details for selected track
        single_reco_track_.update( &proc_tracks_[0], event_ );
        return true;
    }

    //--------------------------------------------------

    BFScatterSingleTrack::BFScatterSingleTrack( const fne::Event * event,
            YAML::Node& instruct, KaonTrack& kt )
        :BFSingleTrack( event, instruct, kt), event_( event)
    {
        angle_sigma_ = get_yaml<double>( instruct, "angle_sigma" );
        angle_frequency_ = get_yaml<double>( instruct, "angle_frequency" );

        mom_sigma_ = get_yaml<double>( instruct, "mom_sigma" );
        mom_frequency_ = get_yaml<double>( instruct, "mom_frequency" );

        BOOST_LOG_SEV( get_log(), always_print)
            << "K2pi scatter angle_frequency: " << angle_frequency_;
    }

    void BFScatterSingleTrack::modify_processing_track
        ( processing_track& pt ) const
        {
            //seed the random number generator with the event time stamp
            std::mt19937 gen( event_->header.time_stamp );

            std::uniform_real_distribution<double> uni_dist;
            std::normal_distribution<double> angle_dist(0, angle_sigma_ );
            std::normal_distribution<double> mom_dist(0, mom_sigma_ );

            double uniform_roll = 0;

            //should we generate an x kick
            uniform_roll = uni_dist(gen);
            if( uniform_roll < angle_frequency_ )
            {
                //xkick
                double xanglekick = angle_dist( gen );
                pt.rt->bdxdz += xanglekick;
            }

            //should we generate an y kick
            uniform_roll = uni_dist(gen);
            if( uniform_roll < angle_frequency_ )
            {
                //ykick
                double yanglekick = angle_dist( gen );
                pt.rt->bdydz += yanglekick;
            }

            //should we generate a mom kick
            uniform_roll = uni_dist(gen);
            if ( uniform_roll < mom_frequency_ )
            {
                //mom kick
                double mom_kick = mom_dist( gen ) 
                    / std::pow(pt.corr_mom ,2 );

                pt.corr_mom += mom_kick;
            }
        }
}
