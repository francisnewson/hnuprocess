#include "Counter.hh"
#include <iomanip>
#include "RecoFactory.hh"

namespace fn
{
    int nice_mod_count ( int nEvents )
    {
        return std::max( 1, static_cast<int>
                (pow(10, static_cast<int>
                     (floor(log10(nEvents) + 0.3)) -1 ) ) );
    }

    REG_DEF_SUB( Counter);

    Counter::Counter( logger& log, int max )
        :log_( log), max_(max), current_(0)
    {
        mod_ = nice_mod_count( max_ );
    }

    void Counter::new_event()
    {
        if ( current_ % mod_ == 0 )
        {
            BOOST_LOG_SEV( log_, always_print )
                << "Processed: " 
                << std::setw(9) << current_ << " /"
                << std::setw(9) << max_ ;
        }

        ++current_;
    }

    template<>
        Subscriber * create_subscriber<Counter>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            return new Counter( rf.get_log(), rf.get_max_events() );
        }
}
