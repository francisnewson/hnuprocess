#include "tracking_selections.hh"
#include "Xcept.hh"
#include "RecoParser.hh"

namespace fn
{
    SingleTrack * get_single_track
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            YAML::Node yogt = instruct["inputs"]["ogt"];

            if ( !yogt )
            {throw Xcept<MissingNode>( "ogt" );}

            SingleTrack * ogt = dynamic_cast<SingleTrack*>
                ( rf.get_subscriber( yogt.as<std::string>() ));

            if ( !ogt )
            { throw Xcept<BadCast>( "OGT" ); }

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

    TrackQuality::TrackQuality( const SingleTrack& st, double quality )
        :st_( st ), quality_( quality )
    {}

    bool TrackQuality::do_check() const
    { return st_.get_single_track().get_quality() == quality_; }

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

    //--------------------------------------------------

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
            double outer = instruct["inner"].as<double>();

            return new TrackRadialAcceptance( *st, ts, z, inner, outer );

        }

    //--------------------------------------------------
}

