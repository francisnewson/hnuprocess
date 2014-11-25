#include "GlobalStatus.hh"
#include "Event.hh"

namespace fn
{
    FNEGStatus::FNEGStatus( const fne::Event * e, bool mc )
        :e_( e ), mc_( mc )
    {}

         Long64_t FNEGStatus::get_run() const
    {
        return e_->header.run;
    }

    bool FNEGStatus::is_mc() const
    {
        return mc_;
    }

    GlobalStatus*& raw_global_status()
    {
        static GlobalStatus* gs = 0;
        return gs;
    }

    GlobalStatus& global_status()
    {
        GlobalStatus * gs = raw_global_status();
        if ( gs  == 0 )
        { throw std::runtime_error( 
                "Attempt to access uniitialized global status" ); }

        return * raw_global_status();
    }
}
