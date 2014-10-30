#ifndef NOTIFYER_HH
#define NOTIFYER_HH

#include <vector>
#include "Rtypes.h"
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <iomanip>
#include "Event.hh"


namespace fn
{
    /*****************************************************
     * NOTIFYER CLASS
     *
     * Decides what to tell subscribers based on the type 
     * of event ( e.g. new burst )
     *****************************************************/
    template <class S>
        class Notifyer
        {
            private:
                bool new_burst_;
                bool new_run_;
                bool new_tree_;

                bool started_burst_;
                bool started_run_;
                bool started_tree_;

                Long64_t current_burst_;
                Long64_t current_run_;
                Long64_t current_tree_;

                int event_version_;

                const fne::Event * event_;

                //Subscribers
                std::vector<S*> raw_header_subs_;
                std::vector<S*> checked_header_subs_;
                std::vector<S*> event_subs_;

            public:
                Notifyer()
                    :new_burst_( false ),
                    new_run_( false ),
                    new_tree_( false ),
                    started_burst_( false ),
                    started_run_( false ),
                    started_tree_( false ),
                    current_burst_( -1 ),
                    current_run_( -1 ),
                    current_tree_( -1 ) { };

                //Initializtation
                void set_event_ptr( const fne::Event * e )
                { event_ = e; }

                //Subscriber management
                void add_raw_header_subscriber( S * s )
                { raw_header_subs_.push_back( s ); }

                void add_checked_header_subscriber( S * s )
                { checked_header_subs_.push_back( s ) ; }

                void add_event_subscriber( S *  s )
                { event_subs_.push_back( s ); }

                void status_report( std::ostream& os_ )
                {
                    int width = 50;

                    os_ << std::setw(width) << "Raw header subscribers: "
                        << raw_header_subs_.size() <<"\n";

                    os_ << std::setw(width) << "Checked header subscribers: "
                        << checked_header_subs_.size() <<"\n";

                    os_ << std::setw(width) << "Event subscribers: "
                        << event_subs_.size() <<"\n";
                    os_ << std::endl;
                }

                //Action
                void new_event( int tree )
                {
                    auto& h = event_->header;

                    new_burst_ = false;
                    new_run_ = false;
                    new_tree_ = false;

                    new_burst_ =  ( current_burst_ != h.burst_time );
                    new_run_ = ( current_run_ != h.run );
                    new_tree_ = ( current_tree_ != tree );

                    if ( new_run_ )
                    { 
                        //std::cerr << "NEW RUN" << std::endl;
                        current_run_ = h.run; 
                    }

                    if ( new_tree_ )
                    { 
                        //std::cerr << "NEW TREE" << std::endl;
                        current_tree_ = tree;
                    }

                    if ( new_burst_ )
                    { 
                        //std::cerr << "NEW BURST" << std::endl;
                        current_burst_ = h.burst_time;
                    }
                }

                void notify_raw_header()
                { notify( raw_header_subs_); }

                void notify_checked_header()
                { notify( checked_header_subs_ ); }

                void notify_event()
                { notify( event_subs_ );  }

                void end_processing()
                {
                    for (auto& sub : raw_header_subs_ )
                    {
                        sub->end_processing();
                    }

                    for (auto& sub : checked_header_subs_ )
                    {
                        sub->end_processing();
                    }

                    for (auto& sub : event_subs_ )
                    {
                        sub->end_processing();
                    }
                }

                void notify( std::vector<S*>& s )
                {
                    if ( s.size() == 0 ){ return; }

                    if ( new_tree_ )
                    {
                        if ( started_tree_ )
                        {
                            for ( auto& sub : s )
                            { sub->end_file(); }
                        }

                        for ( auto& sub : s )
                        { sub->new_file(); }

                        started_tree_ = true;
                    }

                    if ( new_burst_ )
                    {
                        if ( started_burst_ )
                        {
                            for ( auto& sub : s )
                            { sub->end_burst(); }
                        }


                        for ( auto& sub : s )
                        { sub->new_burst(); }

                        started_burst_ = true;
                    }

                    if ( new_run_)
                    {
                        if ( started_run_ )
                        {
                            for ( auto& sub : s )
                            { sub->end_run(); }
                        }

                        for ( auto& sub : s )
                        { sub->new_run(); }

                        started_run_ = true;

                    }

                    for ( auto& sub : s )
                    { sub->new_event(); }
                }
                bool need_full_event()
                { return event_subs_.size() > 0 ; }

        };

}

#endif
