#include "KaonTrack.hh"
#include "yaml_help.hh"
#include "tracking_functions.hh"
#include "NA62Constants.hh"
#include "Xcept.hh"

namespace fn
{
    Track get_kp_track( const fne::Conditions c )
    {
        return Track { c.pkxoffp, c.pkyoffp, 0, 
            c.pkdxdzp, c.pkdydzp, 1 };
    }

    Track get_km_track( const fne::Conditions c )
    {
        return Track { c.pkxoffm, c.pkyoffm, 0, 
            c.pkdxdzm, c.pkdydzm, 1 };
    }
    //--------------------------------------------------
    
    REG_DEF_SUB( KaonTrack );

    template<>
        Subscriber * create_subscriber<KaonTrack>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();
            bool is_mc = rf.is_mc();

            std::string kaon_type =  
                get_yaml<std::string>( instruct, "kaon_type" );

            if ( kaon_type == "rawkp " )
            {
                return new RawKPTrack( event, is_mc );
            }
            else if ( kaon_type == "rawkm" )
            {
                return new RawKMTrack( event, is_mc );
            }
            else
            {
                throw Xcept<UnknownKaonType>( kaon_type );
            }
        }

    KaonTrack * get_kaon_track( YAML::Node& instruct, RecoFactory& rf )
    {
        KaonTrack * kt = 0;

        try
        {
            YAML::Node ykt = instruct["inputs"]["kt"];
            if ( !ykt )
            { throw Xcept<MissingNode>( "kt" ); }

            kt = dynamic_cast<KaonTrack*>
                ( rf.get_subscriber( ykt.as<std::string>() ) );

            if ( !ykt )
            {throw Xcept<BadCast>( "KT" ); }
        }
        catch( ... )
        {
            std::cerr << "Trying to get kaon track (" __FILE__")\n";
        }

        return kt;
    }

    //--------------------------------------------------

    void KaonTrack::new_event() {}

    TVector3 KaonTrack::get_kaon_point() const
    {
        return get_kaon_track().get_point();
    }

    TVector3 KaonTrack::get_kaon_3mom() const
    {
        return get_kaon_mom() * (  get_kaon_track().get_direction().Unit() );
    }

    TVector3 KaonTrack::extrapolate_z( double z ) const
    {
        return get_kaon_track().extrapolate( z );
    }

    TLorentzVector KaonTrack::get_kaon_4mom() const
    {
        return { get_kaon_3mom(),
            std::hypot( get_kaon_mom(), na62const::mK) };
    }

    //--------------------------------------------------

    void CachedKaonTrack::new_event()
    {
        kt_ = load_kaon_track();
        kmom_ = load_kaon_mom();
    }

    const Track& CachedKaonTrack::get_kaon_track() const
    {
        return kt_;
    }

    double CachedKaonTrack::get_kaon_mom() const
    {
        return kmom_;
    }

    //--------------------------------------------------

    RawKPTrack::RawKPTrack ( const fne::Event * e , bool mc )
        :e_(e), mc_( mc ){}

    Track RawKPTrack::load_kaon_track() const
    {
        return get_kp_track( e_->conditions );
    }

    double RawKPTrack::load_kaon_mom() const
    {
        return e_->conditions.pkp;
    }

    //--------------------------------------------------

    RawKMTrack::RawKMTrack ( const fne::Event * e , bool mc )
        :e_(e), mc_( mc ){}

    Track RawKMTrack::load_kaon_track() const
    {
        return get_km_track( e_->conditions );
    }

    double RawKMTrack::load_kaon_mom() const
    {
        return e_->conditions.pkm;
    }
}
