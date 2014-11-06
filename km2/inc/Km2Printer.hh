#ifndef KM2PRINTER_HH
#define KM2PRINTER_HH
#include "Analysis.hh"
#include "Km2Reco.hh"
#include "Km2Clusters.hh"
#if 0
/*
 *  _  __          ____  ____       _       _
 * | |/ /_ __ ___ |___ \|  _ \ _ __(_)_ __ | |_ ___ _ __
 * | ' /| '_ ` _ \  __) | |_) | '__| | '_ \| __/ _ \ '__|
 * | . \| | | | | |/ __/|  __/| |  | | | | | ||  __/ |
 * |_|\_\_| |_| |_|_____|_|   |_|  |_|_| |_|\__\___|_|
 *
 *
 */
#endif
namespace fn
{
    class Km2Printer : public Analysis
    {
        public:
            Km2Printer( const Selection& sel,
                    const fne::Event * e,
                    const Km2Event& km2_event,
                    const Km2Clusters& km2_clusters,
                    std::ostream& os
                    );

        private:
            void process_event();

            const fne::Event * e_ ;
            const Km2Event& km2_event_ ;
            const Km2Clusters& km2_clusters_ ;

            std::ostream& os_;

            REG_DEC_SUB( Km2Printer );
    };

    //--------------------------------------------------

    template<>
        Subscriber * create_subscriber<Km2Printer>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
