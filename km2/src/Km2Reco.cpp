#include "Km2Reco.hh"
#include "NA62Constants.hh"
#include "RecoFactory.hh"
#include "tracking_selections.hh"
#include "Xcept.hh"

namespace fn
{

    void Km2RecoEvent::update( const fne::Event * event, 
            const KaonTrack * kt, const SingleTrack* st )
    {
        e_  = event ;
        srt_ =  &st->get_single_track() ;
        kt_ = kt ;

        //COMPUTE MISSING MASS

        //muon 4 mom
        TVector3 muon_3mom = srt_->get_3mom();
        double muon_mom = srt_->get_mom();
        const double& muon_mass = na62const::mMu;
        double muon_energy = std::hypot( muon_mom, muon_mass );

        TLorentzVector muon_4mom { muon_3mom, muon_energy };


        //kaon 4 mom
        TVector3  kaon_3mom = kt_->get_kaon_3mom();
        double kaon_mom = kt_->get_kaon_mom();
        const double& kaon_mass = na62const::mK;
        double kaon_energy = std::hypot( kaon_mom, kaon_mass );
        TLorentzVector kaon_4mom { kaon_3mom, kaon_energy };

        //neutrino 4 mom
        p4miss_ = kaon_4mom - muon_4mom;
        m2miss_ = p4miss_.M2();

        //Beam pion 4 mom
        const double& pion_mass = na62const::mPi;
        double pion_energy = std::hypot( kaon_mom, pion_mass );
        TLorentzVector pion_4mom { kaon_3mom, pion_energy };
        TLorentzVector p4pimiss = pion_4mom - muon_4mom; 
        m2pimiss_ = p4pimiss.M2();
    }

    double Km2RecoEvent::get_m2miss() const
    {
        return m2miss_;
    }

    double Km2RecoEvent::get_m2pimiss() const
    {
        return m2pimiss_;
    }

    double Km2RecoEvent::get_muon_mom() const
    {
        return srt_->get_mom();
    }

    double Km2RecoEvent::get_zvertex() const
    {
        return srt_->get_vertex().Z();
    }

    double Km2RecoEvent::get_cda() const
    {
        return srt_->get_cda();
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Event );

    Km2Event::Km2Event( const fne::Event * event,
            const SingleTrack& st, bool mc )
        :e_( event), st_( st ), kt_( event, mc )
    {}

    void Km2Event::new_event()
    {
        dirty_ = true;
    }


    const Km2RecoEvent& Km2Event::get_reco_event() const
    {
        if ( dirty_ )
        {
            process_event();
            dirty_ = false;
        }

        assert ( !dirty_ );

        return reco_event_;
    }

    void Km2Event::process_event() const
    {
        reco_event_.update( e_, &kt_, &st_ );
    }

    template<>
        Subscriber * create_subscriber<Km2Event>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event =  rf.get_event_ptr();
            SingleTrack * st = get_single_track( instruct, rf );
            bool mc = rf.is_mc();

            return new Km2Event( event, *st, mc  );
        }

    //--------------------------------------------------

    Km2Event * get_km2_event
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            YAML::Node yogt = instruct["inputs"]["km2event"];

            if ( !yogt )
            {throw Xcept<MissingNode>( "km2event" );}

            Km2Event * ogt = dynamic_cast<Km2Event*>
                ( rf.get_subscriber( yogt.as<std::string>() ));

            if ( !ogt )
            { throw Xcept<BadCast>( "K2EVENT" ); }

            return ogt;
        }

}
