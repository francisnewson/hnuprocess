#ifndef SELECTION_HH
#define SELECTION_HH
#include "Subscriber.hh"
#include "boost/optional.hpp"

#if 0
/*
 *  ____       _           _   _
 * / ___|  ___| | ___  ___| |_(_) ___  _ __
 * \___ \ / _ \ |/ _ \/ __| __| |/ _ \| '_ \
 *  ___) |  __/ |  __/ (__| |_| | (_) | | | |
 * |____/ \___|_|\___|\___|\__|_|\___/|_| |_|
 *
 *
 */
#endif

namespace fn
{

    struct UnpreparedRequest;

    /*****************************************************
     * SELECTION VISITOR
     *
     * Base class for algorithms which are going to 
     * traverse selection trees ( mostly for debugging
     * since they implement checking events themselves)
     * 
     *****************************************************/
    class Selection;

    class SelectionVisitor 
    {
        public:
            virtual	bool visit_enter( Selection& ) = 0;
            virtual	bool visit_leave( Selection& ) = 0;
            virtual bool visit( Selection& ) = 0;
    };

    /*****************************************************
     * SELECTION CLASS
     *
     * Base class for both individual cuts and composite
     * selections ( implements compositie patern)
     *****************************************************/
    class Selection : public Subscriber
    {
        public:
            virtual bool check() const = 0;
            virtual double get_weight() const = 0; 
            virtual ~Selection(){};

            virtual bool accept( SelectionVisitor& sv );
    };

    /*****************************************************
     * CACHED SELECTION CLASS
     *
     * Returns cached values for check and get_weight()
     * Override do_check and do_weight to customize.
     *****************************************************/
    class CachedSelection : public Selection
    {
        public:
            void new_event();
            bool check() const;
            double get_weight() const ;

        private:
            virtual bool do_check () const = 0 ;
            virtual double do_weight () const { return 1.0; }
            mutable boost::optional<bool> check_;
            mutable boost::optional<double> weight_;
    };

    /*****************************************************
     * COMPOSITE SELECTION CLASS
     *
     * Returns the combined result of all its children
     *****************************************************/
    struct MissingNode{};

    class CompositeSelection : public CachedSelection
    {
        public:
            typedef std::vector<Selection*>::iterator
                child_iterator;

            typedef std::vector<Selection*>::const_iterator
                const_child_iterator;

            CompositeSelection(){};
            CompositeSelection
                ( std::vector<Selection*> selections);
            CompositeSelection (Selection* selection);

            void AddChild( Selection* selection );

            bool accept( SelectionVisitor& sv );

            REG_DEC_SUB( CompositeSelection );

        private:
            bool do_check () const;
            double do_weight () const;
            std::vector<Selection*> children_;
    };


    //Builder function for CompositeSelection
    template<>
        Subscriber * create_subscriber<CompositeSelection>
        (YAML::Node& instruct, RecoFactory& rf );


}
#endif
