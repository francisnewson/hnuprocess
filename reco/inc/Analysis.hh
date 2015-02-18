#ifndef ANALYSIS
#define ANALYSIS

#include "Selection.hh"

#if 0
/*
 *     _                _           _
 *    / \   _ __   __ _| |_   _ ___(_)___
 *   / _ \ | '_ \ / _` | | | | / __| / __|
 *  / ___ \| | | | (_| | | |_| \__ \ \__ \
 * /_/   \_\_| |_|\__,_|_|\__, |___/_|___/
 *                        |___/
 *
 */
#endif

namespace fn
{
    /*****************************************************
     * ANALYSIS
     *
     * Base class for all analysis outputs. Handles 
     * checking of a selection.
     *****************************************************/
    struct MissingSelection;

    class Analysis : public Subscriber
    {
        public:
            Analysis( const Selection&s );
            void new_event(); //overrides Subscriber::new_event()
            double get_weight() const;

        private:
            //Method to do the actual work
            virtual void process_event() = 0;
            const Selection& selection_;
    };
}


#endif
