#include "Subscriber.hh"
#include "RecoFactory.hh"
#include "Xcept.hh"

namespace fn
{
    void Subscriber::set_name( std::string name )
    { name_ = name; }

    std::string Subscriber::get_name()
    { return name_; }

    void Subscriber::set_id( int id )
    { id_ = id; }

    int Subscriber::get_id()
    { return id_; }

    //--------------------------------------------------
  
            Subscriber::Subscriber()
            :log_level_( per_event ){}

    void Subscriber::set_log( logger& log)
    {
        log_ =& log ;
    }

    logger& Subscriber::get_log()
    {
        if ( !log_ )
        {
            throw Xcept<NoLoggerSet>( LINE_STRING );
        }
        return * log_;
    }

    void Subscriber::set_log_level( severity_level sl ) 
    { log_level_ = sl; }

    severity_level Subscriber::log_level() const
    { return log_level_; }

    logger * Subscriber::log_ = 0;
}
