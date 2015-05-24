#include "SingleTrack.hh"
#include "NA62Constants.hh"
#include "Xcept.hh"
#include "tracking_functions.hh"
#include "yaml_help.hh"
#include "root_help.hh"
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

            bool do_bf = true;

            if ( const YAML::Node do_bf_node = instruct["do_bf"] )
            {
                do_bf = do_bf_node.as<bool>() ;
            }

            if ( method == "BF" )
            {
                return new BFSingleTrack{ event, instruct, *kt, do_bf } ;
            }
            else if ( method == "BFScatter" )
            {
                return new BFScatterSingleTrack{ event, instruct, *kt, do_bf };
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
            const fne::Event * event, const Track& kaon_track,
            logger& fnlog, bool do_bf )
    {
        proc_track_ = proc_track;
        if ( do_bf )
        {
            //bf track routine returns track with the corrected direction
            Track effective_track_params = bfc_.compute_bf_track
                ( proc_track_->corr_mom, proc_track_->rt, proc_track_->vert );

            //new track should start at midpoint with corrected direction
            Track pt_track = get_bz_track( *proc_track_->rt );
            double mid_z = 0.5 * ( proc_track_->vert.point.Z() + na62const::bz_tracking );
            TVector3 mid_point = pt_track.extrapolate( mid_z );

            bf_track_ = Track{ mid_point, effective_track_params.get_direction() };
        }
        else
        {
            const fne::RecoTrack& rt = *proc_track_->rt;
            bf_track_ = Track{ TVector3{ rt.bx, rt.by, na62const::bz_tracking}, TVector3 { rt.bdxdz, rt.bdydz, 1} };
        }

        try
        {
            bf_vertex_ = compute_cda( bf_track_, kaon_track );
        }
        catch ( std::domain_error& e )
        {
            //Handle parallel tracks
            BOOST_LOG_SEV( fnlog, always_print )
                << "Event : "
                << event->header.run << " "
                << event->header.burst_time << " " 
                << event->header.compact_number 
                << " " << e.what() ;

            bf_vertex_.point = TVector3( 0, 0, - 1000000.0 );
            bf_vertex_.cda = ( extrapolate_z( *proc_track->rt, 0) - kaon_track.extrapolate(0) ).Mag() ;
        }
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
        //return proc_track_->vert.point;
        return bf_vertex_.point;
    }

    double BFSingleRecoTrack::get_cda() const
    {
    //    return proc_track_->vert.cda;
    return bf_vertex_.cda;
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

    TVector3 BFSingleRecoTrack::get_unscattered_3mom() const
    {
        assert( proc_track_->orig_rt !=0  );
        const fne::RecoTrack& rt = *proc_track_->orig_rt;
        TVector3 unscatmom{ rt.bdxdz, rt.bdydz, 1};
        return proc_track_->unscattered_mom  * unscatmom.Unit();
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

    int BFSingleRecoTrack::get_compact_id() const
    {
        const fne::RecoTrack& rt = *proc_track_->rt;
        return rt.id;
    }

    int BFSingleRecoTrack::get_muon_id() const
    {
        const fne::RecoTrack& rt = *proc_track_->rt;
        return rt.iMuon;
    }

    //--------------------------------------------------

    //BF

    processing_track::processing_track( const processing_track& other )
        :corr_mom( other.corr_mom ), unscattered_mom( other.unscattered_mom), vert( other.vert ), good( other.good ),
        rt_( other.rt_ ), rt( &rt_ ), orig_rt( other.orig_rt )
    {}

    void swap
        ( processing_track& first, processing_track& second )
        {
            using std::swap;
            swap( first.corr_mom, second.corr_mom );
            swap( first.unscattered_mom, second.unscattered_mom );
            swap( first.vert, second.vert );
            swap( first.good, second.good );
            swap( first.rt_, second.rt_ );
            swap( first.orig_rt, second.orig_rt );
            first.rt = & first.rt_;
            second.rt = & second.rt_;
        }

    processing_track& processing_track::operator= ( processing_track other )
    {
        swap( *this, other );
        return *this;
    }

    BFSingleTrack::BFSingleTrack( const fne::Event * event, 
            YAML::Node& instruct, KaonTrack& kt, bool do_bf)
        :event_( event ), kt_( kt ), do_bf_( do_bf )
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
            pt.orig_rt = rt;

            assert( pt.rt != rt );
            assert( pt.rt->p == rt->p );
            assert( pt.orig_rt == rt );

            pt.corr_mom = p_corr_ab( pt.rt->p, pt.rt->q, alpha, beta  );
            pt.unscattered_mom = pt.corr_mom;
            modify_processing_track( pt );
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
            try
            {
                pt.vert  = compute_cda(
                        pt_track, kaon_track );
            }
            catch ( std::domain_error& e )
            {
                //Handle parallel tracks
                BOOST_LOG_SEV( get_log(), always_print )
                    << "Event : "
                    << event_->header.run << " "
                    << event_->header.burst_time << " " 
                    << event_->header.compact_number 
                    << " " << e.what() ;

                pt.vert.point = TVector3( 0, 0, - 1000000.0 );
                pt.vert.cda = ( pt_track.extrapolate(0) - kaon_track.extrapolate(0) ).Mag() ;
            }

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
        single_reco_track_.update( &proc_tracks_[0], event_, kt_.get_kaon_track() , get_log(), do_bf_);
        return true;
    }

    //--------------------------------------------------

    BFScatterSingleTrack::BFScatterSingleTrack( const fne::Event * event,
            YAML::Node& instruct, KaonTrack& kt, bool do_bf)
        :BFSingleTrack( event, instruct, kt, do_bf), event_( event)
    {
        double angle_sigma = get_yaml<double>( instruct, "angle_sigma" );
        double angle_frequency = get_yaml<double>( instruct, "angle_frequency" );

        double mom_sigma = get_yaml<double>( instruct, "mom_sigma" );
        double mom_frequency = get_yaml<double>( instruct, "mom_frequency" );

        scatterer_.set_angle_params( angle_sigma, angle_frequency );
        scatterer_.set_mom_params( mom_sigma, mom_frequency );

        BOOST_LOG_SEV( get_log(), always_print)
            << "K2pi scatter angle_frequency: " << angle_frequency;
    }

    void BFScatterSingleTrack::modify_processing_track
        ( processing_track& pt ) const
        {
            scatterer_.scatter_track( event_->header.time_stamp,
                    pt.rt->bdxdz, pt.rt->bdydz, pt.corr_mom );
        }

    TrackScatterer::TrackScatterer(
            double angle_sigma, double angle_frequency,
            double mom_sigma, double mom_frequency )
        : angle_sigma_( angle_sigma), angle_frequency_( angle_frequency ),
        mom_sigma_( mom_sigma), mom_frequency_( mom_frequency )
    {}

    void TrackScatterer::set_angle_params( double sigma, double frequency)
    {
        angle_sigma_ = sigma;
        angle_frequency_ = frequency;
    }

    void TrackScatterer::set_mom_params( double sigma, double frequency)
    {
        mom_sigma_ = sigma;
        mom_frequency_ = frequency;
    }

    void TrackScatterer::scatter_track( Long64_t seed, 
            double& dxdz, double& dydz, double& mom ) const
    {
        //seed the random number generator with the event time stamp
        std::mt19937 gen( seed );

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
            dxdz += xanglekick;
        }

        //should we generate an y kick
        uniform_roll = uni_dist(gen);
        if( uniform_roll < angle_frequency_ )
        {
            //ykick
            double yanglekick = angle_dist( gen );
            dydz += yanglekick;
        }

        //should we generate a mom kick
        uniform_roll = uni_dist(gen);
        if ( uniform_roll < mom_frequency_ )
        {
            //mom kick
            double mom_roll = mom_dist( gen );
            double mom_kick = mom_roll * std::pow(mom,2 );
            //std::cout << mom_kick << std::endl;
            mom += mom_kick;
        }

    }

    //--------------------------------------------------

    TrackPowerScatterer::TrackPowerScatterer(
            double angle_cutoff, double angle_frequency, 
            double mom_cutoff, double mom_frequency)
    {
        set_angle_params( angle_cutoff, angle_frequency);
        set_mom_params( mom_cutoff, mom_frequency);
    }

    void TrackPowerScatterer::set_angle_params
        ( double cutoff, double frequency)
        {
            angle_function_ = TF1( "f_angle", "tanh( pow(x*[0],4) )*pow(x,-2)",
                    -15*cutoff, 15*cutoff );

            double xscale = 1.021;
            angle_function_.SetParameter( 0 , xscale/cutoff );
            angle_function_.SetNpx( 200 );
            angle_frequency_ = frequency;
        }

    void TrackPowerScatterer::set_mom_params
        ( double cutoff, double frequency)
        {
            //mom_function_ = TF1( "f_mom", "pow(x,-4)", cutoff, 1 );
            mom_function_ = TF1( "f_mom", "tanh( pow(x*[0],4) )*pow(x,-2)",
                    -1, 1 );

            double xscale = 1.021;
            mom_function_.SetParameter(0, xscale/cutoff );
            mom_function_.SetNpx( 200 );
            mom_frequency_ = frequency;
        }

    void TrackPowerScatterer::scatter_track( Long64_t seed, 
            double& dxdz, double& dydz, double& mom ) const
    {
        //seed the random number generator with the event time stamp
        std::mt19937 gen( seed );
        std::uniform_real_distribution<double> uni_dist;
        std::uniform_int_distribution<int> dir_dist( 0, 1 );

        double uniform_roll = 0;

        //should we generate an x kick
        uniform_roll = uni_dist(gen);
        if( uniform_roll < angle_frequency_ )
        {
            //xkick
            double xanglekick = angle_function_.GetRandom();
            int dir = dir_dist( gen );
            if ( dir == 1)
            {dxdz += xanglekick;}
            else
            {dxdz -= xanglekick;}
        }

        //should we generate an y kick
        uniform_roll = uni_dist(gen);
        if( uniform_roll < angle_frequency_ )
        {
            //ykick
            double yanglekick = angle_function_.GetRandom();
            int dir = dir_dist( gen );
            if ( dir == 1)
            {dydz += yanglekick;}
            else
            {dydz -= yanglekick;}
        }

        //should we generate a mom kick
        uniform_roll = uni_dist(gen);
        if ( uniform_roll < mom_frequency_ )
        {
            //mom kick
            double mom_roll = mom_function_.GetRandom();
            double mom_kick = mom_roll * mom ;
            //std::cout << mom_kick << std::endl;
            int dir = dir_dist( gen );
            if ( dir == 1)
            {mom += mom_kick;}
            else
            {mom -= mom_kick;}
        }
    }
}
