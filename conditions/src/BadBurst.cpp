#include "BadBurst.hh"
#include <iostream>
#include <fstream>
#include "RecoFactory.hh"
#include "FunctionCut.hh"
#include "Xcept.hh"
#include "yaml_help.hh"

namespace fn
{

    REG_DEF_SUB( BadBurst);

    bool BadBurst::do_check() const
    {
        auto this_burst = BurstId
        { event_->header.run, event_->header.burst_time};

        bool burst_in_list = burst_tracker_.check_value( this_burst );

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Burst: " << this_burst 
            << burst_in_list;

        return ! burst_in_list;
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

    REG_DEF_SUB( BadBurstRange);

    bool BadBurstRange::do_check() const
    {
        auto this_burst = BurstId
        { event_->header.run, event_->header.burst_time};

        bool burst_in_list = burst_ranges_.in_any_range( this_burst );

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Burst: " << this_burst 
            << burst_in_list;

        return ! burst_in_list;
    }

    template<>
        Subscriber * create_subscriber<BadBurstRange>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            bool is_mc = rf.is_mc();

            if ( is_mc )
            {
                return new FunctionCut<auto_pass>{{0}} ; 
            }

            //Extract burst list filename
            auto burst_list = get_yaml<std::string>(instruct,"burst_range_list");

            //Open file
            std::ifstream ifs ( burst_list );
            if ( !ifs.good() )
            {
                throw std::runtime_error
                    ( FILE_STRING  + " Can't open " + burst_list );
            }

            const fne::Event * e = rf.get_event_ptr();

            return new BadBurstRange( 
                    std::istream_iterator<std::pair<BadBurst::BurstId,BadBurst::BurstId>>( ifs ),
                    std::istream_iterator<std::pair<BadBurst::BurstId,BadBurst::BurstId>>(),
                    e );
        }


    //--------------------------------------------------

    //Burst operators
    std::istream& operator >> 
        ( std::istream& is , BadBurst::BurstId& bi)
        {
            return is >> bi.run >> bi.burst_time ;
        }

    std::istream& operator >> 
        ( std::istream& is , 
          std::pair<BadBurst::BurstId,BadBurst::BurstId>& bi_pair)
        {
            return is >> bi_pair.first >> bi_pair.second ;
        }

    std::ostream& operator << 
        ( std::ostream& os , const BadBurst::BurstId& bi)
        {
            return os << "[ " << bi.run << " "
                << bi.burst_time << " ]";
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
