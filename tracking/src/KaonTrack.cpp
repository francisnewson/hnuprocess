#include "KaonTrack.hh"

namespace fn
{

    KaonTrack::KaonTrack( const fne::Event * e, bool mc)
        :e_( e ), mc_( mc ){}

    void KaonTrack::new_event() {}


    double KaonTrack::get_kaon_mom() const
    {
        return e_->conditions.pkp;
    }

    TVector3 KaonTrack::get_kaon_3mom() const
    {
        const  fne::Conditions& c = e_->conditions;
        TVector3 kaon_dir{ c.pkdxdzp, c.pkdydzp, 1 };
        return get_kaon_mom() * (  kaon_dir.Unit() );
    }

}
