#include "TriggerEffs.hh"
#include "Km2Reco.hh"
#include "Event.hh"
#include "yaml_help.hh"
#include "root_help.hh"


namespace fn
{

    REG_DEF_SUB( TriggerEffs );

    TriggerEffs::TriggerEffs( const Selection& base, const Selection& source,
            const  Km2Event& km2e , TFile& tf, std::string folder)
        :Analysis( base ), source_( source ), km2e_( km2e ),
        tf_( tf ), folder_( folder )
    {

        h_all_ = hs_.MakeTH1D( "h_all", "All events",
                10000, -0.7, 0.3, "m^2_miss GeV^2/c^4" );

        h_passed_ = hs_.MakeTH1D( "h_passed", "Passing events",
                10000, -0.7, 0.3, "m^2_miss GeV^2/c^4" );

        h_all_p_t_ = hs_.MakeTH2D( "h_all_p_t", "All events (p vs t)",
                100, 0, 100, "P (GeV)",
                250, 0, 25e-3, "t (rad)" );

        h_passed_p_t_ = hs_.MakeTH2D( "h_passed_p_t", "Passing events (p vs t)",
                100, 0, 100, "P (GeV)",
                250, 0, 25e-3, "t (rad)" );

    }

    void TriggerEffs::process_event()
    {
        auto& km2re = km2e_.get_reco_event();
        double m2miss = km2re.get_m2m_kmu();

        h_all_->Fill( m2miss );
        h_all_p_t_->Fill( km2re.get_muon_mom(), km2re.get_opening_angle() );

        if ( source_.check() )
        {
            h_passed_->Fill( m2miss );
            h_passed_p_t_->Fill( km2re.get_muon_mom(), km2re.get_opening_angle() );
        }
    }

    void TriggerEffs::end_processing()
    {
        cd_p( &tf_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<TriggerEffs>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * base = rf.get_selection(
                    get_yaml<std::string>( instruct, "base" )) ;

            const Selection * source = rf.get_selection(
                    get_yaml<std::string>( instruct, "source" )) ;

            const Km2Event* km2_event = get_km2_event( instruct, rf );

            TFile & tfile = rf.get_tfile(
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder(instruct, rf );

            return new TriggerEffs( *base, *source, *km2_event, tfile, folder );
        }

    //--------------------------------------------------

    REG_DEF_SUB( Q11TTrigger );

    Q11TTrigger::Q11TTrigger( const fne::Event * e , bool mc )
        :e_( e ), mc_( mc ){}

    bool Q11TTrigger::do_check() const
    {
        if ( mc_ ){ return true; }

        const fne::PatternUnitChannel& pu_q11t = e_->pattern_unit.channel[4];
        bool matching_q11t =
            ( pu_q11t.tslice[3] >> 9 ) & 1  ||
            ( pu_q11t.tslice[4] >> 9 ) & 1  ||
            ( pu_q11t.tslice[5] >> 9 ) & 1;

        return matching_q11t;
    }

    template<>
        Subscriber * create_subscriber<Q11TTrigger>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * e  = rf.get_event_ptr();
            bool mc = rf.is_mc();

            return new Q11TTrigger( e, mc );
        }
}
