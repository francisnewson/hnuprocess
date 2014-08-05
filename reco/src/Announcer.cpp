#include "Subscriber.hh"
#include "RecoFactory.hh"
#include "Announcer.hh"

namespace fn
{

    SubscriberRegister<Announcer> Announcer::announcer_reg_("Announcer");

    template<>
        Subscriber * create_subscriber<Announcer>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            auto& log = rf.get_log();
            return new Announcer( log );
        }

    Announcer::Announcer( logger& log )
        :log_( log ) 
    {
        sl_ = always_print;
    }

    void Announcer::new_file()
    { 

        BOOST_LOG_SEV( log_, sl_ )
            << "ANNOUNCEMENT: New file";
    }

    void Announcer::new_run()
    { 

        BOOST_LOG_SEV( log_, sl_ )
            << "ANNOUNCEMENT: New run";
    }

    void Announcer::new_burst()
    { 

        BOOST_LOG_SEV( log_, sl_ )
            << "ANNOUNCEMENT: New burst";
    }

    void Announcer::new_event()
    { 

        BOOST_LOG_SEV( log_, per_event )
            << "ANNOUNCEMNET: New event";
    }

    void Announcer::end_processing()
    { 

        BOOST_LOG_SEV( log_, sl_ )
            << "ANNOUNCEMNET: End processing";
    }

}

