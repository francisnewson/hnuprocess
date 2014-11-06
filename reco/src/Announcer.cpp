#include "Subscriber.hh"
#include "RecoFactory.hh"
#include "Announcer.hh"

namespace fn
{

    REG_DEF_SUB( Announcer );

    template<>
        Subscriber * create_subscriber<Announcer>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event =  rf.get_event_ptr();
            return new Announcer( event );
        }

    Announcer::Announcer( const fne::Event * e)
        :e_( e ) 
    {}

    void Announcer::new_file()
    { 

        BOOST_LOG_SEV( get_log() , log_level() )
            << "ANNOUNCEMENT: New file";
    }

    void Announcer::new_run()
    { 

        BOOST_LOG_SEV( get_log() , log_level() )
            << "ANNOUNCEMENT: New run"
            << e_->header.run;
    }

    void Announcer::new_burst()
    { 

        BOOST_LOG_SEV( get_log() , always_print )
            << "ANNOUNCEMENT: New burst "
            << e_->header.run << " "
            << e_->header.burst_time;
    }

    void Announcer::new_event()
    { 

        BOOST_LOG_SEV( get_log() , log_level() )
            << "ANNOUNCEMENT: New event";
    }

    void Announcer::end_processing()
    { 

        BOOST_LOG_SEV( get_log() , log_level() )
            << "ANNOUNCEMENT: End processing";
    }

}

