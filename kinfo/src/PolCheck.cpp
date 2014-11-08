#include "PolCheck.hh"
#include "tracking_selections.hh"
#include "yaml_help.hh"

namespace fn
{

    REG_DEF_SUB( PolCheck );

    PolCheck::PolCheck( const Selection& sel, 
            TFile& tf, std::string folder,
            std::ostream& fout,
            const fne::Event * e, bool mc,
            SingleTrack& st )
        :Analysis( sel), tfile_( tf ), folder_( folder ),
        fout_( fout ), e_( e), st_( st )
    {

        h_momkick = hs_.MakeTH2D( 
                "h_momkick", "Single track deflection by spectrometer",
                100, -0.5 , 0.5 , "X kick ( GeV)",
                100, -0.05, 0.05, "Y kick ( GeV)" );

        h_momkick = hs_.MakeTH2D( 
                "h_field", "Deflection * particle charge",
                100, -0.5 , 0.5 , "X kick ( GeV)",
                100, -0.05, 0.05, "Y kick ( GeV)" );
    }

    void PolCheck::new_run()
    {
        BOOST_LOG_SEV( get_log(), log_level() )
            << "PolCheck: new run";

        current_run_ = e_->header.run;
        run_count_ = 0;
        field_sum_ = 0;
    }

    void PolCheck::end_run()
    {
        BOOST_LOG_SEV( get_log(), log_level() )
            << "PolCheck: end run";

        run_average_field_.insert( std::make_pair(
                    current_run_, field_sum_ / run_count_ ) );

        run_average_kick_.insert( std::make_pair(
                    current_run_, kick_sum_ / run_count_ ) );
    }

    void PolCheck::process_event()
    {
        const SingleRecoTrack& srt = st_.get_single_track();

        TVector3 us_mom = srt.get_us_mom();
        TVector3 ds_mom = srt.get_ds_mom();
        TVector3 mnp33_kick = ds_mom - us_mom;

        double wgt = get_weight();
        int charge = srt.get_charge();

        h_momkick->Fill( mnp33_kick.X(), mnp33_kick.Y(), wgt );
        h_momkick->Fill( charge * mnp33_kick.X(), mnp33_kick.Y(), wgt );

        ++run_count_;
        field_sum_ += charge * mnp33_kick.X();
        kick_sum_ +=  mnp33_kick.X();
    }

    void PolCheck::end_processing()
    {
        end_run();

        auto ifield = begin( run_average_field_ );
        auto ikick = begin( run_average_kick_ );

        auto endfield = end( run_average_field_ );
        auto endkick = end( run_average_kick_ );

            fout_ << "#" <<  std::setw(9) << "run"
                << std::setw(10) << "kick"
                << std::setw(10) << "q*kick"
                << "\n";

        while( ifield != endfield && ikick != endkick )
        {
            assert( ifield->first == ikick->first );

            fout_ << std::setw(10) << ifield->first
                << std::setw(10) << ikick->second
                << std::setw(10) << ifield->second
                << "\n";

            ++ifield;
            ++ikick;
        } 

        cd_p( &tfile_, folder_ );
        hs_.Write();
    }

    template<>
        Subscriber * create_subscriber<PolCheck>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string folder = get_folder( instruct, rf );

            const fne::Event * event = rf.get_event_ptr();
            SingleTrack * st = get_single_track( instruct, rf );

            const Selection * sel = rf.get_selection
                ( get_yaml<std::string>( instruct, "selection" ) );

            TFile& tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::ostream& ofs = rf.get_ostream( 
                    get_yaml<std::string>( instruct, "fout" ) );

            bool is_mc = rf.is_mc();

            return new PolCheck( *sel, tfile, folder, ofs, event, is_mc, *st );
        }
}
