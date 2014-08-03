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

    void RecoFactory::create_subscriber( std::string name, 
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

    }

    Subscriber * RecoFactory::get_subscriber ( std::string name )
    {
        auto it = subscribers_.find( name );
        if ( it != subscribers_.end() )
        {
            throw Xcept<UnknownSubscriberName>( name );
        }

        return it->second;
    }


}