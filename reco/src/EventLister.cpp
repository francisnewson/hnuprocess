#include "EventLister.hh"
#include "yaml_help.hh"
#include "RecoFactory.hh"
#include "FunctionCut.hh"
#include "Summary.hh"

namespace fn
{
    REG_DEF_SUB( EventLister );

    EventLister::EventLister( const Selection& base, 
            const fne::Event * e, std::ostream& os )
        :Analysis( base), e_( e ), os_( os )
    {}

    void EventLister::process_event()
    {
        os_ << e_->header.run << " " 
            << e_->header.burst_time << " " 
            << e_->header.compact_number << std::endl;
    }

    template<>
        Subscriber * create_subscriber<EventLister>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel = rf.get_selection( 
                    get_yaml<std::string>( instruct, "selection" ) );

            std::string ostream_name =
                get_yaml<std::string>( instruct, "ostream" ) ;
            std::ostream& os = rf.get_ostream( ostream_name);

            const fne::Event * event = rf.get_event_ptr();

            return new EventLister( *sel, event, os );
        }
    //--------------------------------------------------

    REG_DEF_SUB( EventSelector );

    bool EventSelector::do_check() const
    {

        auto this_event = EventId
        { e_->header.run, e_->header.burst_time,
            e_->header.compact_number };

        bool event_in_list = event_tracker_.check_value( this_event);

        return event_in_list;
    }

    template<>
        Subscriber * create_subscriber<EventSelector>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string event_list = get_yaml<std::string>( instruct, "event_list" );
            std::ifstream ifs( event_list );

            if ( !ifs.good() )
            {
                throw std::runtime_error
                    ( FILE_STRING  + " Can't open " + event_list );
            }

            const fne::Event * e = rf.get_event_ptr();

            return new EventSelector( e,
                    std::istream_iterator<EventId>( ifs ),
                    std::istream_iterator<EventId>());

        }

    //--------------------------------------------------

    //Event operators
    std::istream& operator >> 
        ( std::istream& is , EventId& ei)
        {
            return is >> ei.run >> ei.burst_time  >> ei.compact_number;
        }

    std::ostream& operator << 
        ( std::ostream& os , const EventId& ei)
        {
            return os << "[ " << ei.run << " "
                << ei.burst_time << " " << ei.compact_number<< " ]";
        }

    bool operator<  ( const EventId& lhs,
            const EventId& rhs )
    {
        return ( std::tie( lhs.run, lhs.burst_time, lhs.compact_number )
                < std::tie( rhs.run, rhs.burst_time, rhs.compact_number ) );
    }

    bool operator>  ( const EventId& lhs,
            const EventId& rhs )
    {
        return rhs < lhs;
    }

    bool operator!=  ( const EventId& lhs,
            const EventId& rhs )
    {
        return ( lhs < rhs ) || ( rhs < lhs );
    }

    bool operator==  ( const EventId& lhs, 
            const EventId& rhs )
    {
        return ! ( lhs != rhs ); 
    }


    //--------------------------------------------------

    REG_DEF_SUB( EventPrinter );

    EventPrinter::EventPrinter
        (
         const fne::Event * e,
         Selection& req, Selection& desc,
          std::ostream& os )
        :Analysis( req ), e_(e), desc_(desc ), os_(os)
        { }

    void EventPrinter::process_event()
    {
        os_ << "Event : " << e_->header.compact_number << std::endl;
        FunctionCut<auto_pass> passer( e_ );
        Summary summary( passer, desc_, os_ );
        summary.process_event();
        summary.end_processing();
    }

    template<>
        Subscriber * create_subscriber<EventPrinter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * e = rf.get_event_ptr();

            Selection * req = rf.get_selection( 
                    get_yaml<std::string>( instruct, "required" ) );

            Selection * desc = rf.get_selection( 
                    get_yaml<std::string>( instruct, "description" ) );

            std::string ostream_name =
                get_yaml<std::string>( instruct, "ostream" ) ;
            std::ostream& os = rf.get_ostream( ostream_name);

            return new EventPrinter( e, *req, *desc, os );
        }
}
