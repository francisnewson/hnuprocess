#ifndef EVENTLISTER_HH
#define EVENTLISTER_HH
#include "Analysis.hh"
#include "Event.hh"
#include "OrderedRanger.hh"
#if 0
/*
 *  _____                 _   _     _     _
 * | ____|_   _____ _ __ | |_| |   (_)___| |_ ___ _ __
 * |  _| \ \ / / _ \ '_ \| __| |   | / __| __/ _ \ '__|
 * | |___ \ V /  __/ | | | |_| |___| \__ \ ||  __/ |
 * |_____| \_/ \___|_| |_|\__|_____|_|___/\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class EventLister : public Analysis
    {
        public:
            EventLister( const Selection& base, 
                    const fne::Event * e , std::ostream& os );

        private:
            void process_event();
            const fne::Event * e_ ;
            std::ostream& os_;

            REG_DEC_SUB( EventLister );
    };

    template<>
        Subscriber * create_subscriber<EventLister>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    struct EventId
    {
        int run;
        int burst_time;
        int compact_number;
    };

    class EventSelector: public CachedSelection
    {
        public:
            template< class InputIterator>
                EventSelector(
                        const fne::Event* e,
                        InputIterator start, InputIterator finish)
                :e_( e )
                {
                    event_tracker_.load_info( start, finish );
                }

        private:
            bool do_check() const;
            const fne::Event * e_ ;

            OrderedTracker<EventId> event_tracker_;

            REG_DEC_SUB( EventSelector );
    };

    template<>
        Subscriber * create_subscriber<EventSelector>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------

    std::istream& operator >> 
        ( std::istream& is , EventId& ei);

    std::ostream& operator << 
        ( std::ostream& os , const EventId& ei);

    bool operator<  ( const EventId& lhs,
            const EventId& rhs );

    bool operator>  ( const EventId& lhs,
            const EventId& rhs );

    bool operator!=  ( const EventId& lhs,
            const EventId& rhs );

    bool operator==  ( const EventId& lhs, 
            const EventId& rhs );

    //--------------------------------------------------

    class EventPrinter : public Analysis
    {
        public:
            EventPrinter
                ( const fne::Event * e, Selection& req,
                  Selection& desc, std::ostream& os );

            void process_event();

        private:
            const fne::Event * e_;
            Selection& desc_;
            std::ostream& os_;

            REG_DEC_SUB( EventPrinter );
    };

    template<>
        Subscriber * create_subscriber<EventPrinter>
        (YAML::Node& instruct, RecoFactory& rf );

}
#endif
