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
}

