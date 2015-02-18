#include "Analysis.hh"

namespace fn
{
    Analysis::Analysis( const Selection& s )
        :selection_( s )
    {}

    void Analysis::new_event()
    {
        //If we pass selection
        if ( selection_.check() )
        {
            //call virtual function
            process_event();
        }
    }

    double Analysis::get_weight() const
    {
        return selection_.get_weight();
    }
}
