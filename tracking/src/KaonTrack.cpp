#include "KaonTrack.hh"
#include "tracking_functions.hh"
#include "NA62Constants.hh"

namespace fn
{

    KaonTrack::KaonTrack( const fne::Event * e, bool mc)
        :e_( e ), mc_( mc )
    {
        t_ = get_kaon_track( e->conditions);
    }

    void KaonTrack::new_event() {}


    double KaonTrack::get_kaon_mom() const
    {
        return e_->conditions.pkp;
    }

    TVector3 KaonTrack::get_kaon_point() const
    {
        return t_.get_point();
    }

    TVector3 KaonTrack::get_kaon_3mom() const
    {
        const  fne::Conditions& c = e_->conditions;
        TVector3 kaon_dir{ c.pkdxdzp, c.pkdydzp, 1 };
        return get_kaon_mom() * (  kaon_dir.Unit() );
    }

    TVector3 KaonTrack::extrapolate_z( double z ) const
    {
        return t_.extrapolate( z );
    }

    TLorentzVector KaonTrack::get_kaon_4mom() const
    {
        return { get_kaon_3mom(),
            std::hypot( get_kaon_mom(), na62const::mK) };
    }

}
