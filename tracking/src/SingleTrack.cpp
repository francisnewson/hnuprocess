#include "SingleTrack.hh"
#include "NA62Constants.hh"
#include "Xcept.hh"
#include "tracking_functions.hh"

namespace fn
{
    //SingleTrack results cacheing
    //Calculation is delgated to descendents
    void SingleTrack::new_event()
    {
        dirty_ = true;
    }

    bool SingleTrack::found_single_track()
    {
        if ( dirty_ )
        {
            process_event();
        }
        return found_;
    }

    SingleRecoTrack& SingleTrack::get_single_track()
    {
        if ( dirty_ )
        {
            process_event();
        }
        if ( !found_ )
        {
            throw Xcept<EventDoesNotContain>( "SingleTrack");
        }
        return single_reco_track_;
    }

    REG_DEF_SUB( SingleTrack );

    //Factory creation for SingleTrack descendents
    template<>
        Subscriber * create_subscriber<SingleTrack>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = instruct["method"].as<std::string>();
            const fne::Event * event = rf.get_event_ptr();

            if ( method == "BF" )
            {
                return new BFSingleTrack{ event, instruct } ;
            }
            else
            {
                throw Xcept<UnknownSingleTrackMethod>( method );
            }
        }

    //--------------------------------------------------

    //BF
    BFSingleTrack::BFSingleTrack( const fne::Event * event, YAML::Node& instruct )
        :event_( event )
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
    }

    //Identify and calculate single track
    bool BFSingleTrack::process_event()
    {
        int ntracks = event_->detector.ntracks;
        auto& etracks = event_->detector.tracks;

        const double& alpha = event_->conditions.alpha;
        const double& beta = event_->conditions.beta;

        if ( ntracks < 1 )
        { return false; }

        //Collect tracks in a vector
        proc_tracks_.clear();
        processing_track pt;

        //Look for tracks within initial momentum range
        for ( unsigned int itrk = 0 ; itrk != 0 ; ++itrk )
        {
            pt.rt =  static_cast<fne::RecoTrack*>( etracks[itrk] );
            pt.corr_mom = p_corr_ab( pt.rt->p, pt.rt->q, alpha, beta  );
            pt.good = true;

            if ( pt.corr_mom < init_min_mom_ ){ continue; }
            if ( pt.corr_mom > init_max_mom_ ){ continue; }

            proc_tracks_.push_back( pt );
        }

        if ( proc_tracks_.size() < 1 ){ return false; }

        //Loop over pairs of tracks
        for ( auto otrk = proc_tracks_.begin() ; 
                otrk != proc_tracks_.end() ; ++ otrk )
        {
            auto & ot = * otrk;

            for ( auto itrk = proc_tracks_.begin() ; 
                    itrk != proc_tracks_.end() ; ++ itrk )
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
                pt.good = false;
                continue;
            }

            Track pt_track = get_bz_track( *pt.rt );
            Track kaon_track = get_kaon_track( conditions );

            //extract raw vertex and cda
            pt.vert  = compute_cda(
                    pt_track, kaon_track );

            //raw cda requirement
            if ( pt.vert.cda > init_min_cda_ )
            {
                pt.good = false;
            }

            //raw vertex range
            if ( pt.vert.point.Z() < init_min_z_ )
            {
                pt.good = false;
            }

            if ( pt.vert.point.Z() > init_max_z_ )
            {
                pt.good = false;
            }
        }

        //remove bad tracks
        proc_tracks_.erase(
                std::remove_if ( begin( proc_tracks_ ), end( proc_tracks_),
                    []( const processing_track& pt){ return !(pt.good ); } ),
                proc_tracks_.end() );

        if ( proc_tracks_.size() != 1 )
        {
            return false;
        }

        //Compute details for selected track
        single_reco_track_ = compute_track( proc_tracks_[0] );
        return true;
    }
    //--------------------------------------------------

    SingleRecoTrack BFSingleTrack::compute_track
        ( const processing_track& pt )
        {
            return SingleRecoTrack{};

        }
}
