#ifndef SCMPVSCMP_HH
#define SCMPVSCMP_HH
#include "Analysis.hh"
#include "Event.hh"
#include <iosfwd>
#if 0
/*
 *  ____                    __     __    ____                 
 * / ___|  ___ _ __ ___  _ _\ \   / /__ / ___|_ __ ___  _ __  
 * \___ \ / __| '_ ` _ \| '_ \ \ / / __| |   | '_ ` _ \| '_ \ 
 *  ___) | (__| | | | | | |_) \ V /\__ \ |___| | | | | | |_) |
 * |____/ \___|_| |_| |_| .__/ \_/ |___/\____|_| |_| |_| .__/ 
 *                      |_|                            |_|    
 * 
 */
#endif
namespace fn
{
    /* **************************************************
     * SCMPVSCMP
     * Analysis to compare output of compact reader
     * starting from .cmp or .scmp files. It just outputs
     * text files to be compared with e.g. vimdiff
     * **************************************************/

    class ScmpVsCmp  : public Analysis
    {
        public :
            ScmpVsCmp( const fn::Selection& sel, 
                    const fne::Event* e, std::ostream& os );
            void process_event();

        private:
            const fne::Event * e_;
            std::ostream& os_;
            REG_DEC_SUB( ScmpVsCmp );
    };


    template<>
        Subscriber * create_subscriber<ScmpVsCmp>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
