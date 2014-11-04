#include "Reconstruction.hh"
#include "Event.hh"
#include <iostream>
#include <iomanip>
#include "Subscriber.hh"
#include "Selection.hh"

using boost::filesystem::path;
using std::vector;

namespace fn
{

    //SETUP
    Reconstruction::Reconstruction( logger& log)
        :event_ptr_(nullptr),  required_events_specified_(false),
        required_events_(0), event_count_(0),
        stop_(false), event_loaded_(false), os_(std::cerr),
        log_( log )
    {}

    void Reconstruction::set_filenames ( vector<path> filenames )
    { filenames_ = filenames; }

    void Reconstruction::set_required_events( Long64_t n )
    { 
        required_events_specified_ = true;
        required_events_ = n;
    }

    void Reconstruction::load_chain()
    {
        if ( required_events_specified_ )
        {
            chain_ = new EventChain( log_, filenames_, required_events_ );
        }
        else
        {
            chain_ = new EventChain( log_, filenames_ );
            required_events_ = chain_->get_max_event();
        }
        event_ptr_ = chain_->get_event_pointer();
        notifyer_.set_event_ptr( event_ptr_ );
    }

    //----------------------------------------------------------------------

    //ADD SUBSCRIBERS
    void Reconstruction::add_raw_header_subscriber( Subscriber * s )
    {notifyer_.add_raw_header_subscriber(  s ); }

    void Reconstruction::add_header_check( Selection * s )
    {
        notifyer_.add_raw_header_subscriber(  s );
        header_checks_.push_back( s ); 
    }
    void Reconstruction::add_checked_header_subscriber( Subscriber * s )
    {notifyer_.add_checked_header_subscriber(  s );}

    void Reconstruction::add_event_subscriber( Subscriber *  s )
    {notifyer_.add_event_subscriber(   s );}

    const fne::Event * Reconstruction::get_event_ptr() const
    {
        if ( !chain_ )
        { throw std::runtime_error(
                "Event pointer requested before chain loaded" ); }
        return event_ptr_;
    }

    void Reconstruction::status_report( std::ostream& os )
    {
        int width = 50;

        os_ << "\nRECONSTRUCTION STATUS\n";

        notifyer_.status_report( os );

        os_ << "\nHEADER CHECKS\n";

        os_ << std::setw(width) << "Header checks: "
            << header_checks_.size() <<"\n";

        os_ << std::endl;

    }

    //----------------------------------------------------------------------

    //EVENT PROCESSING
    bool Reconstruction::next_event()
    {
        ++event_count_;
        if ( !chain_ )
        { throw std::runtime_error(
                "next_event requested before chain loaded" ); }

        //check for stop command
        if ( stop_ )
        { return false; }

        if (remote_stop_ && *remote_stop_ )
        { 

            std::cerr << "*** ***ALERT ...  JOB KILLED"
                " ... SHUTTING DOWN EARLY *** *** \n";
            return false; 
        }

        //Load header
        event_loaded_ = false;
        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  about to do load_next_event_header";
        bool ok = chain_->load_next_event_header();
        if (!ok)
        { return false; }

        //Event type
        int tree_number =chain_->get_tree_number();
        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  About to notify new_event";
        notifyer_.new_event( tree_number);

#if 0
        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  About to do get_event_version";

        chain_->get_event_version() ;
#endif

        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  About to notify raw_header";

        notifyer_.notify_raw_header();

        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  Finished next_event";

        return true;
    }

    bool Reconstruction::continue_event()
    {
        //Should we continue?
        for ( auto& check : header_checks_ )
        {
            if ( !check->check() )
            { return false; }
        }
        return true;
    }

    void Reconstruction::process_headers()
    {
        load_event();
        notifyer_.notify_checked_header();
    }

    void Reconstruction::load_event()
    {
        if ( !event_loaded_  )
        {chain_->load_full_event();}
    }

    void Reconstruction::process_event()
    {
        load_event();
        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  about to notify full event";
        notifyer_.notify_event();
    }

    //Convenience function if no intervening input required
    bool Reconstruction::auto_next_event()
    {
        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  about to do next_event";
        bool carry_on = next_event();

        if ( !carry_on )
        { return false; }

        int run = event_ptr_->header.run;

        //If we haven't reached the required 
        //run range, don't process
        if ( min_run_ && ( run < *min_run_ ) )
        { return true; }

        //If we've gone past the required
        //run range, stop proceesing
        if ( max_run_ && ( run > *max_run_ ) )
        { return false; }

        if ( ! continue_event() )
        { return true; }

        BOOST_LOG_SEV( log_, fn::severity_level::debug)
            << "RECO:  about to process headers";

        process_headers();

        if ( notifyer_.need_full_event() )
        {

            BOOST_LOG_SEV( log_, fn::severity_level::debug)
                << "RECO:  about to load full event";

            process_event();
        }
        return true;
    }

    //----------------------------------------------------------------------

    //Tell all subscribers we're done
    void  Reconstruction::end_processing()
    {
        notifyer_.end_processing();
        std::cerr << chain_->read_info() << std::endl;
    }

    Long64_t Reconstruction::max_events() const
    {
        return required_events_;
    }

    void Reconstruction::set_remote_stop( bool& remote_stop )
    {
        remote_stop_ = remote_stop;
    }

    boost::filesystem::path Reconstruction::get_current_filename()
    {
        return chain_->get_filename();
    }

    bool Reconstruction::is_mc() const
    {
        return chain_->is_mc();
    }

    void Reconstruction::set_min_run( int min_run )
    { min_run_.reset( min_run); }

    void Reconstruction::set_max_run( int max_run )
    { max_run_.reset(max_run); }
}

