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
        const double& muon_mass = na62const::mMu;
        const double& pion_mass = na62const::mPi;
        const double& kaon_mass = na62const::mK;

        //daughter muon 4 mom
        TVector3 muon_3mom = srt_->get_3mom();
        double muon_mom = srt_->get_mom();
        double muon_energy = std::hypot( muon_mom, muon_mass );
        TLorentzVector daughter_muon_4mom { muon_3mom, muon_energy };

        //parent kaon 4 mom
        TVector3  kaon_3mom = kt_->get_kaon_3mom();
        double kaon_mom = kt_->get_kaon_mom();
        double kaon_energy = std::hypot( kaon_mom, kaon_mass );
        TLorentzVector parent_kaon_4mom { kaon_3mom, kaon_energy };

        //parent pion 4 mom
        double parent_pion_energy = std::hypot( kaon_mom, pion_mass );
        TLorentzVector parent_pion_4mom { kaon_3mom, parent_pion_energy };

        //daughter pion 4 mom
        double daughter_pion_energy = std::hypot( muon_mom, pion_mass );
        TLorentzVector daughter_pion_4mom { muon_3mom, daughter_pion_energy };

        //km2 missing mass
        p4miss_kmu_ = parent_kaon_4mom - daughter_muon_4mom;
        m2m_kmu_ = p4miss_kmu_.M2();

        //pm2 missing mass
        TLorentzVector p4miss_pimu = parent_pion_4mom - daughter_muon_4mom; 
        m2m_pimu_ = p4miss_pimu.M2();

        //k2pi missing mass
        TLorentzVector p4miss_kpi = parent_kaon_4mom- daughter_pion_4mom;
        m2m_kpi_ = p4miss_kpi.M2();

        opening_angle_ = kaon_3mom.Angle( muon_3mom );

        pt_ = muon_3mom.Perp( kaon_3mom );
    }

    //Return missing masses
    double Km2RecoEvent::get_m2miss() const
    { return m2m_kmu_; }

    double Km2RecoEvent::get_m2m_kmu() const 
    { return m2m_kmu_; }

    double Km2RecoEvent::get_m2m_kpi() const 
    { return m2m_kpi_; }

    double Km2RecoEvent::get_m2m_pimu() const 
    { return m2m_pimu_; }

    double Km2RecoEvent::get_kaon_mom() const
    { return kt_->get_kaon_mom() ; }

    double Km2RecoEvent::get_muon_mom() const
    { return srt_->get_mom(); }

    double Km2RecoEvent::get_zvertex() const
    { return srt_->get_vertex().Z(); }

    double Km2RecoEvent::get_cda() const
    { return srt_->get_cda(); }

    double Km2RecoEvent::get_opening_angle() const
    { return opening_angle_; }

    const SingleRecoTrack * Km2RecoEvent::get_reco_track() const
    { return srt_; }

    double Km2RecoEvent::get_pt()  const
    {
        return pt_;
    }

    //--------------------------------------------------

    REG_DEF_SUB( Km2Event );

    Km2Event::Km2Event( const fne::Event * event,
            const SingleTrack& st, const KaonTrack& kt, bool mc )
        :e_( event), st_( st ), kt_( kt )
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
            KaonTrack * kt = get_kaon_track( instruct, rf );
            bool mc = rf.is_mc();

            return new Km2Event( event, *st, *kt,  mc  );
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
