#include "BadBurst.hh"
#include <iostream>
#include <fstream>
#include "RecoFactory.hh"
#include "Xcept.hh"

namespace fn
{

    REG_DEF_SUB( BadBurst);

    bool BadBurst::do_check() const
    {
        auto this_burst = BurstId
        { event_->header.run, event_->header.burst_time};

        return ! burst_tracker_.check_value( this_burst );
    }

    template<>
        Subscriber * create_subscriber<BadBurst>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            //Extract burst list filename
            YAML::Node burst_list = instruct["burst_list"];

            if ( !burst_list )
            {throw Xcept<MissingNode>( "burst_list" );}

            //Open file
            std::ifstream ifs ( burst_list.as<std::string>() );
            if ( !ifs.good() )
            {
                throw std::runtime_error
                    ( FILE_STRING  + " Can't open " +
                      burst_list.as<std::string>() );
            }

            const fne::Event * e = rf.get_event_ptr();

            return new BadBurst( 
                    std::istream_iterator<BadBurst::BurstId>( ifs ),
                    std::istream_iterator<BadBurst::BurstId>(),e );
        }

    //--------------------------------------------------

    //Burst operators
    std::istream& operator >> 
        ( std::istream& is , BadBurst::BurstId& bi)
        {
            return is >> bi.run >> bi.burst_time ;
        }

    bool operator<  ( const BadBurst::BurstId& lhs,
            const BadBurst::BurstId& rhs )
    {
        return ( std::tie( lhs.run, lhs.burst_time )
                < std::tie( rhs.run, rhs.burst_time ) );
    }

    bool operator>  ( const BadBurst::BurstId& lhs,
            const BadBurst::BurstId& rhs )
    {
        return rhs < lhs;
    }

    bool operator!=  ( const BadBurst::BurstId& lhs,
            const BadBurst::BurstId& rhs )
    {
        return ( lhs < rhs ) || ( rhs < lhs );
    }

    bool operator==  ( const BadBurst::BurstId& lhs, 
            const BadBurst::BurstId& rhs )
    {
        return ! ( lhs != rhs ); 
    }
}