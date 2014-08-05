#ifndef SUMMARY_HH
#define SUMMARY_HH
#include "Analysis.hh"
#if 0
/*
 *  ____
 * / ___| _   _ _ __ ___  _ __ ___   __ _ _ __ _   _
 * \___ \| | | | '_ ` _ \| '_ ` _ \ / _` | '__| | | |
 *  ___) | |_| | | | | | | | | | | | (_| | |  | |_| |
 * |____/ \__,_|_| |_| |_|_| |_| |_|\__,_|_|   \__, |
 *                                             |___/
 *
 */
#endif
namespace fn
{
    /* **************************************************
     * Class to build a collection of ids and names in
     * order of traversal
     * **************************************************/

    class NameVisitor : public SelectionVisitor
    {

        public:
            struct nameset{
                std::string name;
                int id;
                int gen;
            };
            NameVisitor();

            //Visitor interface
            bool visit( Selection& ) ;
            bool visit_enter( Selection& );
            bool visit_leave( Selection& );

            //Collection (vectorish) interface
            std::vector<nameset>::iterator begin();
            std::vector<nameset>::iterator end();

        private:
            int gen_;
            std::vector<nameset> names_;
    };

    /* **************************************************
     * Class to count the number of times each selection
     * is passed. Maps ids to counts
     * **************************************************/
    class SummaryVisitor : public SelectionVisitor
    {
        public:
            SummaryVisitor();
            bool visit( Selection& ) ;
            bool visit_enter( Selection& ){ return true;} 
            bool visit_leave( Selection& );

            //Collection (vectorish) interface
            typedef std::vector<int> container;

            typedef container::const_iterator const_iterator;

            const_iterator begin() const;
            const_iterator end() const;
            int at( unsigned int i) const;

        private:
            container counts_;

    };

    //--------------------------------------------------

    /* **************************************************
     * Analysis class managing a summary visitor
     * **************************************************/

    class Summary : public Analysis
    {
        public:
            Summary
                ( Selection& base, Selection& source, std::ostream& );

            void process_event();
            void end_processing();

        private:
            Selection& source_;
            std::ostream& os_;
            SummaryVisitor sv_;

            REG_DEC_SUB( Summary );
    };

    template<>
        Subscriber * create_subscriber<Summary>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
