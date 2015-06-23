#include "Summary.hh"
#include "stl_help.hh"
#include "Xcept.hh"
#include "yaml-cpp/yaml.h"
#include "yaml_help.hh"
#include <iomanip>
#include <cassert>
#include "RecoFactory.hh"

namespace fn
{

    NameVisitor::NameVisitor()
        :gen_(0){}

    bool NameVisitor::visit_enter( const Selection& s )
    {
        //Increase depth
        ++gen_;
        return true;
    }

    bool NameVisitor::visit( const Selection& s )
    {
        //Record name of leaf
        names_.push_back( nameset{ s.get_name(), s.get_id(), gen_} );
        return true;
    }

    bool NameVisitor::visit_leave( const Selection& s )
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


    bool SummaryVisitor::visit( const Selection& s) 
    {
        //If selection passed, increment the vector
        //position corresponding to the selection id
        //std::cerr <<"Checking" << std::endl;
        if ( s.check() )
        {
            //std::cerr <<"passed" << std::endl;
            counts_.at( s.get_id() )++;
            //std::cerr <<"incremented" << std::endl;
            return true;
        }
        //std::cerr <<"failed" << std::endl;
        return false;
    }

    bool SummaryVisitor::visit_leave( const Selection& s )
    {
        //Do the composite count on the way out

        //std::cerr <<"About to visit" << std::endl;
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

    //Initialize vector with space for maximum id
    //( should remove hardcoded max at some point)
    SummaryWeightVisitor::SummaryWeightVisitor()
        :weights_( 200, 0 ), weight_stack_(1, 1){
            std::cerr << " Summary weight visitor container size: " 
                << weights_.size() << std::endl;
        }

    bool SummaryWeightVisitor::visit_enter( const Selection& )
    {
        weight_stack_.push_back( weight_stack_.back() );
        return true;
    } 

    bool SummaryWeightVisitor::visit( const Selection& s) 
    {
        //If selection passed, increment the vector
        //position corresponding to the selection id
        //std::cerr <<"Checking" << std::endl;
        if ( s.check() )
        {
            //std::cerr <<"passed" << std::endl;
            weight_stack_.back() *= s.get_weight();
            weights_.at( s.get_id() ) += weight_stack_.back();
            //std::cerr <<"incremented" << std::endl;
            return true;
        }
        //std::cerr <<"failed" << std::endl;
        return false;
    }

    bool SummaryWeightVisitor::visit_leave( const Selection& s )
    {
        //Do the composite count on the way out
        //std::cerr <<"About to visit" << std::endl;
        assert( weight_stack_.size() > 0 );
        weight_stack_.pop_back();
        return visit( s );
    } 

    void SummaryWeightVisitor::set_event_weight( double weight )
    { 
        event_weight_ = weight; 
        assert( weight_stack_.size() == 1 );
        weight_stack_[0] =  event_weight_;
    }

    //expose vector of counts
    SummaryWeightVisitor::const_iterator SummaryWeightVisitor::begin() const
    { return weights_.begin(); }

    SummaryWeightVisitor::const_iterator SummaryWeightVisitor::end() const
    { return weights_.end(); }

    double SummaryWeightVisitor::at(  unsigned int i ) const
    { return weights_.at( i ); }

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

        swv_.set_event_weight( get_weight() );
        source_.accept( swv_ );
        //std::cerr << "Done processing" << std::endl;
    }
    void Summary::end_processing()
    {
        //std::cerr << "Collecting names" << std::endl;
        NameVisitor nv{};
        source_.accept( nv );

        std::cerr << "Summary:" << std::endl;
        //loop over namesets
        os_ << "#START " << get_name() << std::endl;
        for ( auto& ns : nv )
        {
            os_ << std::setw(20 + ns.gen * 10) << ns.name ;
            os_
                << std::setw(20)  << sv_.at( ns.id ) 
                << std::setw(20)  << std::fixed << std::setprecision(2) << swv_.at( ns.id ) 
                << std::endl;
        }
        os_ << "#END " << get_name() << std::endl;
    }

    template<>
        Subscriber * create_subscriber<Summary>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            Selection * base = rf.get_selection(
                    get_yaml<std::string>(instruct, "base" ));

            Selection * source = rf.get_selection( 
                    get_yaml<std::string>(instruct, "source"));

            std::ostream& os = rf.get_ostream( 
                    get_yaml<std::string>( instruct, "ostream" ));

            return new Summary( *base, *source, os );
        }
}
