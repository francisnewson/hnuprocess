#include "km3_selections.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{

            REG_DEF_SUB( Km3M2Miss );

    Km3M2Miss::Km3M2Miss( const Km3Reco& km3_reco,
            double min_m2, double max_m2 )
        :km3r_( km3_reco), min_( min_m2 ), max_( max_m2) {}

    bool Km3M2Miss::do_check() const
    {
        if ( !km3r_.found_km3_event())
        { return false; }

        auto km3_event = km3r_.get_reco_event();
        double m2m = km3_event.get_m2m_miss();
        return (m2m > min_ && m2m < max_);
    }

    template<>
        Subscriber * create_subscriber<Km3M2Miss>
        (YAML::Node& instruct, RecoFactory& rf  )
        {
            Km3Reco * km3reco = get_km3_reco( instruct, rf );
            double min = get_yaml<double>( instruct, "min_m2" );
            double max = get_yaml<double>( instruct, "max_m2" );

            return new Km3M2Miss(  *km3reco, min, max );
        }

    //--------------------------------------------------

            REG_DEF_SUB( Km3PtMiss );

    Km3PtMiss::Km3PtMiss( const Km3Reco& km3_reco,
            double min_pt, double max_pt )
        :km3r_( km3_reco), min_( min_pt ), max_( max_pt ){}

    bool Km3PtMiss::do_check() const
    {
        if ( !km3r_.found_km3_event())
        { return false; }

        auto km3_event = km3r_.get_reco_event();
        double pt = km3_event.get_pt_miss();
        return (pt > min_ && pt < max_);
    }

    template<>
        Subscriber * create_subscriber<Km3PtMiss>
        (YAML::Node& instruct, RecoFactory& rf  )
        {
            Km3Reco * km3reco = get_km3_reco( instruct, rf );
            double min = get_yaml<double>( instruct, "min_pt" );
            double max = get_yaml<double>( instruct, "max_pt" );

            return new Km3PtMiss(  *km3reco, min, max );
        }

    //--------------------------------------------------

            REG_DEF_SUB( Km3MomMiss );

    Km3MomMiss::Km3MomMiss( const Km3Reco& km3_reco,
            double min_mom, double max_mom )
        :km3r_( km3_reco), min_( min_mom ), max_( max_mom ){}

    bool Km3MomMiss::do_check() const
    {
        if ( !km3r_.found_km3_event())
        { return false; }

        auto km3_event = km3r_.get_reco_event();
        double mom = km3_event.get_p_miss();
        return (mom > min_ && mom < max_);
    }

    template<>
        Subscriber * create_subscriber<Km3MomMiss>
        (YAML::Node& instruct, RecoFactory& rf  )
        {
            Km3Reco * km3reco = get_km3_reco( instruct, rf );
            double min = get_yaml<double>( instruct, "min_mom" );
            double max = get_yaml<double>( instruct, "max_mom" );

            return new Km3MomMiss(  *km3reco, min, max );
        }

    //--------------------------------------------------

            REG_DEF_SUB( Km3DV );

    Km3DV::Km3DV( const Km3Reco& km3_reco,
            double min_dv, double max_dv )
        :km3r_( km3_reco), min_( min_dv ), max_( max_dv ){}

    bool Km3DV::do_check() const
    {
        if ( !km3r_.found_km3_event())
        { return false; }

        auto km3_event = km3r_.get_reco_event();
        double dv = km3_event.get_neutral_vertex().Z() - km3_event.get_charged_vertex().Z();
        return (dv > min_ && dv < max_);
    }

    template<>
        Subscriber * create_subscriber<Km3DV>
        (YAML::Node& instruct, RecoFactory& rf  )
        {
            Km3Reco * km3reco = get_km3_reco( instruct, rf );
            double min = get_yaml<double>( instruct, "min_dv" );
            double max = get_yaml<double>( instruct, "max_dv" );

            return new Km3DV(  *km3reco, min, max );
        }

    //--------------------------------------------------

            REG_DEF_SUB( Km3eop );

    Km3eop::Km3eop( const Km3Reco& km3_reco,
            double min_eop, double max_eop )
        :km3r_( km3_reco), min_( min_eop ), max_( max_eop ){}

    bool Km3eop::do_check() const
    {
        if ( !km3r_.found_km3_event())
        { return false; }

        auto km3_event = km3r_.get_reco_event();
        double eop = km3_event.get_eop();
        return (eop > min_ && eop < max_);
    }

    template<>
        Subscriber * create_subscriber<Km3eop>
        (YAML::Node& instruct, RecoFactory& rf  )
        {
            Km3Reco * km3reco = get_km3_reco( instruct, rf );
            double min = get_yaml<double>( instruct, "min_eop" );
            double max = get_yaml<double>( instruct, "max_eop" );

            return new Km3eop(  *km3reco, min, max );
        }

    //--------------------------------------------------


            REG_DEF_SUB( Km3cda );

    Km3cda::Km3cda( const Km3Reco& km3_reco,
            double min_cda, double max_cda )
        :km3r_( km3_reco), min_( min_cda ), max_( max_cda ){}

    bool Km3cda::do_check() const
    {
        if ( !km3r_.found_km3_event())
        { return false; }

        auto km3_event = km3r_.get_reco_event();
        double cda = km3_event.get_cda();
        return (cda > min_ && cda < max_);
    }

    template<>
        Subscriber * create_subscriber<Km3cda>
        (YAML::Node& instruct, RecoFactory& rf  )
        {
            Km3Reco * km3reco = get_km3_reco( instruct, rf );
            double min = get_yaml<double>( instruct, "min_cda" );
            double max = get_yaml<double>( instruct, "max_cda" );

            return new Km3cda(  *km3reco, min, max );
        }

    //--------------------------------------------------

}
