#include "RecoFactory.hh"
#include "Reconstruction.hh"
#include "Xcept.hh"

namespace fn
{
    RecoFactory::RecoFactory( Reconstruction& reco )
        :reco_( reco), log_(0), next_sub_id_(0){}

    //logging
    void RecoFactory::set_log( logger& log )
    { log_ = &log; }

    logger& RecoFactory::get_log()
    { 
        if ( ! log_ )
        { 
            throw std::runtime_error
                ( "RecoFactory has no log" );
        }
        return * log_;
    }

    //reco interaction
    Long64_t RecoFactory::get_max_events()
    {
        return reco_.max_events();
    }

    const fne::Event * RecoFactory::get_event_ptr()
    {
        return reco_.get_event_ptr();
    }

    Subscriber * RecoFactory::create_subscriber( std::string name, 
            std::string subscriber_type, YAML::Node instruct )
    {
        auto it = subscribers_.find( name );
        if ( it != subscribers_.end() )
        {
            throw Xcept<DuplicateSubscriberName>( name );
        }

        Subscriber * s = SubscriberFactory::get_subscriber
            ( subscriber_type , instruct, *this);

        s->set_name( name );
        s->set_id( next_sub_id_ ++ );
        reco_.add_event_subscriber( s );

        subscribers_.insert( std::make_pair( name, s ) );

        return s;
    }

    Subscriber * RecoFactory::get_subscriber ( std::string name )
    {
        auto it = subscribers_.find( name );
        if ( it == subscribers_.end() )
        {
            throw Xcept<UnknownSubscriberName>( name );
        }

        return it->second;
    }

    //Selection management
    void RecoFactory::add_selection ( std::string name, Selection * s  )
    {
        auto it = selections_.find( name );
        if ( it != selections_.end() )
        {
            throw Xcept<DuplicateSelectionName>( name );
        }

        selections_.insert( std::make_pair( name, s ) );
    }

    Selection * RecoFactory::get_selection ( std::string name)
    {
        auto it = selections_.find( name );
        if ( it == selections_.end() )
        {
            throw Xcept<UnknownSelectionName>( name );
        }
        return it->second;
    }



}
