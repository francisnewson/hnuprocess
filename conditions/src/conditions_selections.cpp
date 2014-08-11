#include "conditions_selections.hh"
#include "RecoFactory.hh"

namespace fn
{

    REG_DEF_SUB(Polarity);

    Polarity::Polarity( const fne::Event *e, int polarity )
        :e_( e),
        pf_( "input/reco/conditions/run_polarity.dat"),
        polarity_( polarity )
    {}

    bool Polarity::do_check() const
    {
        return  ( pf_.get_polarity( e_->header.run ) 
                == polarity_ );
    }

    template<>
        Subscriber * create_subscriber<Polarity>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            int polarity  = instruct["polarity"].as<int>();
            const fne::Event * event  = rf.get_event_ptr();

            return new Polarity(  event, polarity );
        }

    //--------------------------------------------------
}
