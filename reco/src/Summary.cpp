#include "Summary.hh"
#include "stl_help.hh"
#include "Xcept.hh"
#include "yaml-cpp/yaml.h"
#include <iomanip>
#include "RecoFactory.hh"

namespace fn
{

    NameVisitor::NameVisitor()
        :gen_(0){}

    bool NameVisitor::visit_enter( Selection& s )
    {
        //Increase depth
        ++gen_;
        return true;
    }

    bool NameVisitor::visit( Selection& s )
    {
        //Record name of leaf
        names_.push_back( nameset{ s.get_name(), s.get_id(), gen_} );
        return true;
    }

    bool NameVisitor::visit_leave( Selection& s )
    {
        //Decrease depth
        --gen_;
        //Record name of composite selection
        visit( s );
        return true;
    }

    //Expose list of namesets
    std::vector<NameVisitor::nameset>::iterator 
        NameVisitor::begin()
        { return names_.begin() ; }

    std::vector<NameVisitor::nameset>::iterator
        NameVisitor::end()
        { return names_.end() ; }

    //--------------------------------------------------

    //Initialize vector with space for maximum id
    //( should remove hardcoded max at some point)
    SummaryVisitor::SummaryVisitor()
        :counts_( 200, 0 ){
            std::cerr << " Summary visitor container size: " 
                << counts_.size() << std::endl;
        }


    bool SummaryVisitor::visit( Selection& s) 
    {
        //If selection passed, increment the vector
        //position corresponding to the selection id
        if ( s.check() )
        {
            counts_[ s.get_id() ]++;
            return true;
        }
        return false;
    }

    bool SummaryVisitor::visit_leave( Selection& s )
    {
        //Do the composite count on the way out
        return visit( s );
    } 

    //expose vector of counts
    SummaryVisitor::const_iterator SummaryVisitor::begin() const
    { return counts_.begin(); }

    SummaryVisitor::const_iterator SummaryVisitor::end() const
    { return counts_.end(); }

    int SummaryVisitor::at(  unsigned int i ) const
    { return counts_.at( i ); }

    //--------------------------------------------------

    REG_DEF_SUB( Summary );

    Summary::Summary
        ( Selection& base, Selection& source,
          std::ostream& os )
        :Analysis( base ), source_(source ), os_(os)
        {}

    void Summary::process_event()
    {
        source_.accept( sv_ );
    }
    void Summary::end_processing()
    {
        std::cerr << "Collecting names" << std::endl;
        NameVisitor nv{};
        source_.accept( nv );

        std::cerr << "Summary:" << std::endl;
        //loop over namesets
        for ( auto& ns : nv )
        {
            os_ << std::setw(20 + ns.gen * 10) << ns.name ;
            os_ << std::setw(10)  << sv_.at( ns.id ) << std::endl;
        }
    }

    template<>
        Subscriber * create_subscriber<Summary>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            Selection * base = 0 ;
            Selection * source = 0 ;
            std::ostream& os = std::cerr;

            //Extract burst list filename
            if ( YAML::Node ybase = instruct["base"] )
            {
                base = rf.get_selection( ybase.as<std::string>() );
            }
            else
            {
                throw Xcept<MissingSelection>( "Summary: base" );
            }

            if ( YAML::Node ysource = instruct["source"] )
            {
                source = rf.get_selection( ysource.as<std::string>() );
            }
            else
            {
                throw Xcept<MissingSelection>( "Summary: source" );
            }

            return new Summary( *base, *source, os );
        }
}
