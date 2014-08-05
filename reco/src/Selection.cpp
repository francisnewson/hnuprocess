#include "Selection.hh"
#include <iostream>
#include "Xcept.hh"
#include "yaml-cpp/yaml.h"
#include "RecoFactory.hh"

namespace fn
{
    /*****************************************************
     * SELECTION VISITOR
     *****************************************************/

    bool Selection::accept( SelectionVisitor& sv )
    {
        return sv.visit( *this );
        return true;
    };

    /*****************************************************
     * CACHED SELECTION
     *****************************************************/

    void CachedSelection::new_event()
    {
        //Set Dirty flags
        check_.reset();
        weight_.reset();
    }

    //Logic to return cached value having delegated to
    //do_* functions if necessary
    bool CachedSelection::check() const
    {
        if ( check_ )
        { return *check_ ; }
        else
        {
            check_ = do_check();
            assert (check_ );
            return * check_;
        }
    }

    double CachedSelection::get_weight() const
    {
        if ( weight_ )
        { return *weight_ ; }
        else
        {
            weight_ = do_weight();
            assert ( weight_ );
            return * weight_;
        }
    }

    //--------------------------------------------------

    /*****************************************************
     * COMPOSITE SELECTION
     *****************************************************/

    REG_DEF_SUB( CompositeSelection);

    //Composite constructions from selections
    CompositeSelection::CompositeSelection
        ( std::vector<Selection*> selections)
        :children_ ( selections ) {}

    CompositeSelection::CompositeSelection (Selection* selection)
        :children_( {selection} ) {}

    void CompositeSelection::AddChild( Selection* selection )
    {
        children_.push_back( selection );
    }

    //checking is the AND of children's reponses.
    bool CompositeSelection::do_check () const
    {
        for( auto& selection : children_ )
        {
            if ( ! selection->check() )
            {
                return false;
            }
        }
        return true;
    }

    //weight is product of children's weights
    double CompositeSelection::do_weight () const
    {
        double weight = 1;

        for ( auto& selection : children_ )
        { weight *= selection->get_weight(); }

        return weight;
    }

    //SectionVisitor interface
    bool CompositeSelection::accept( SelectionVisitor& sv )
    {
        if ( sv.visit_enter( *this ) ) 
        {
            for ( auto& selection : children_ )
            {
                if ( !selection->accept( sv ) ){ break; }
            }
        }
        return sv.visit_leave( *this );
    };

    //Builder function for CompositeSelection
    template<>
        Subscriber * create_subscriber<CompositeSelection>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            //Collect child selections
            std::vector<Selection*> children;

            YAML::Node child_list = instruct["children"];

            if ( !child_list )
            {throw Xcept<MissingNode>( "children" );}

            for (YAML::const_iterator it=child_list.begin();
                    it!=child_list.end();++it)
            {
                Selection * child = rf.get_selection( it->as<std::string>() );
                children.push_back( child );
            }

            return new CompositeSelection( children );
        }
}
