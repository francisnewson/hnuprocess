#include "Analysis.hh"

namespace fn
{
    Analysis::Analysis( const Selection& s )
        :selection_( s )
    {}

    void Analysis::new_event()
    {
        if ( selection_.check() )
        {
            process_event();
        }
    }

    double Analysis::get_weight() const
    {
        return selection_.get_weight();
    }
}
