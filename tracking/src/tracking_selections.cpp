#include "tracking_selections.hh"
#include "Xcept.hh"
#include "RecoParser.hh"

namespace fn
{
    SingleTrack * get_single_track
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            SingleTrack * ogt = 0;

            try
            {
                YAML::Node yogt = instruct["inputs"]["ogt"];

                if ( !yogt )
                {throw Xcept<MissingNode>( "ogt" );}

                ogt = dynamic_cast<SingleTrack*>
                    ( rf.get_subscriber( yogt.as<std::string>() ));

                if ( !ogt )
                { throw Xcept<BadCast>( "OGT" ); }

            }
            catch ( ... )
            {
                std::cerr << "Trying to get single track (" __FILE__ ")\n" ;
            }

            return ogt;
        }
    //--------------------------------------------------

    REG_DEF_SUB( FoundGoodTrack);

    FoundGoodTrack::FoundGoodTrack( const SingleTrack& st )
        :st_( st )
    {}


    bool FoundGoodTrack::do_check() const
    { return st_.found_single_track(); }

    template<>
        Subscriber * create_subscriber<FoundGoodTrack>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            return new FoundGoodTrack ( * get_single_track( instruct, rf ) );
        }
    //--------------------------------------------------

    REG_DEF_SUB( TrackCharge);


    TrackCharge::TrackCharge( const SingleTrack& st, int charge )
        :st_( st ), charge_( charge )
    {}

    bool TrackCharge::do_check() const
    { return st_.get_single_track().get_charge() == charge_; }

    template<>
        Subscriber * create_subscriber<TrackCharge>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            int charge = instruct["charge"].as<int>();
            SingleTrack * st = get_single_track( instruct, rf );
            return new TrackCharge ( * st, charge );
        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackQuality);

    TrackQuality::TrackQuality( const SingleTrack& st, double quality )
        :st_( st ), quality_( quality )
    {}

    bool TrackQuality::do_check() const
    { return st_.get_single_track().get_quality() >= quality_; }

    template<>
        Subscriber * create_subscriber<TrackQuality>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            double quality = instruct["quality"].as<double>();
            SingleTrack * st = get_single_track( instruct, rf );
            return new TrackQuality ( * st, quality );
        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackRadialAcceptance);

    TrackRadialAcceptance::TrackRadialAcceptance(const SingleTrack& st, 
            track_section ts, double z, 
            double inner, double outer )
        :st_( st), ts_( ts), z_( z),
        inner_( inner), outer_( outer){}

    bool TrackRadialAcceptance::do_check() const
    {
        const SingleRecoTrack& srt = st_.get_single_track();
        if ( ts_ == track_section::us )
        {
            zpoint_ = srt.extrapolate_us( z_ );
        }
        else if ( ts_ == track_section::ds )
        {
            zpoint_ = srt.extrapolate_ds( z_ );
        }
        else if ( ts_ == track_section::bf )
        {
            zpoint_ = srt.extrapolate_bf( z_ );
        }

        double rho = zpoint_.Perp();

        return ( ( rho > inner_) && rho < outer_ );
    }


    template<>
        Subscriber * create_subscriber<TrackRadialAcceptance>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            SingleTrack * st = get_single_track( instruct, rf );
            std::string s_ts= instruct["track_section"].as<std::string>();

            TrackRadialAcceptance::track_section ts;

            if ( s_ts == "us" || s_ts == "upstream" )
            {
                ts = TrackRadialAcceptance::track_section::us;
            }
            else if ( s_ts == "ds" || s_ts == "downstream" )
            {
                ts = TrackRadialAcceptance::track_section::ds;
            }
            else if ( s_ts == "bf" || s_ts == "BlueField" )
            {
                ts = TrackRadialAcceptance::track_section::bf;
            }
            else
            {
                throw std::runtime_error(
                        "Uknown track_section: " + s_ts );
            }

            double z = instruct["z"].as<double>();
            double inner = instruct["inner"].as<double>();
            double outer = instruct["outer"].as<double>();

            return new TrackRadialAcceptance( *st, ts, z, inner, outer );

        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackPZ);

    TrackPZ::TrackPZ( const SingleTrack& st,
            std::vector<rectangle> recs )
        :st_( st), area_cut_( recs )
    {}

    bool TrackPZ::do_check() const
    {
        const SingleRecoTrack& srt = st_.get_single_track();
        TVector3 vertex = srt.get_vertex();
        return area_cut_.allowed({ srt.get_mom(), vertex.Z() } );
    }

    template<>
        Subscriber * create_subscriber<TrackPZ>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            SingleTrack * st = get_single_track( instruct, rf );
            std::string shape = instruct["shape"].as<std::string>();

            if ( shape == "rectangles" )
            {
                std::vector<rectangle> recs=
                    instruct["points"].as<std::vector<rectangle>>();

                return new TrackPZ( *st, recs );
            }
            else
            {
                throw std::runtime_error(
                        "Unknown TrackPZShape" );
            }
        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackCda);

    TrackCda::TrackCda( const SingleTrack& st,
            double cda )
        :st_( st), cda_( cda ){}

    bool TrackCda::do_check() const
    {
        const SingleRecoTrack& srt = st_.get_single_track();
        return srt.get_cda() < cda_;
    }

    template<>
        Subscriber * create_subscriber<TrackCda>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            SingleTrack * st = get_single_track( instruct, rf );
            double cda = instruct["max_cda"].as<double>();

            return new TrackCda( *st, cda );
        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackMomentum);

    TrackMomentum::TrackMomentum( const SingleTrack& st,
            double min_p, double max_p )
        :st_( st), min_p_( min_p ), max_p_(max_p){}

    bool TrackMomentum::do_check() const
    {
        const SingleRecoTrack& srt = st_.get_single_track();
        double mom = srt.get_mom();
        return ( mom < max_p_ ) && ( mom > min_p_ );
    }

    template<>
        Subscriber * create_subscriber<TrackMomentum>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            SingleTrack * st = get_single_track( instruct, rf );
            double min_p = instruct["min"].as<double>();
            double max_p = instruct["max"].as<double>();

            return new TrackMomentum( *st, min_p, max_p );
        }

    //--------------------------------------------------

    REG_DEF_SUB( TrackTime);

    TrackTime::TrackTime(  const fne::Event * e, 
            const SingleTrack& st ,
            double max_dt )
        :e_( e ), st_( st ), max_dt_( max_dt ){}

    bool TrackTime::do_check() const
    {
        double dch_offset = e_->conditions.dch_toffst;
        const SingleRecoTrack& srt = st_.get_single_track();

        return fabs( srt.get_time() - dch_offset ) < max_dt_ ;
    }

    template<>
        Subscriber * create_subscriber<TrackTime>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();
            SingleTrack * st = get_single_track( instruct, rf );
            double max_dt = instruct["max_dt"].as<double>();

            return new TrackTime( event, *st, max_dt );
        }


    //--------------------------------------------------
}

