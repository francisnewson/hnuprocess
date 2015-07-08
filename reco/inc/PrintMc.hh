#ifndef PRINTMC_HH
#define PRINTMC_HH
#include "Analysis.hh"
#include <iosfwd>

#if 0
/*
 *  ____       _       _   __  __
 * |  _ \ _ __(_)_ __ | |_|  \/  | ___
 * | |_) | '__| | '_ \| __| |\/| |/ __|
 * |  __/| |  | | | | | |_| |  | | (__
 * |_|   |_|  |_|_| |_|\__|_|  |_|\___|
 *
 *
 */
#endif
namespace fne { class Event; }

namespace fn
{
    class PrintMc : public Analysis
    {
        public: 
            PrintMc( const Selection& sel, const fne::Event* e ,
                    std::ostream& os );

        private:
            void process_event();

            const fne::Event * e_;
            std::ostream& os_;

            REG_DEC_SUB( PrintMc );
    };


    template<>
        Subscriber * create_subscriber<PrintMc>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif

