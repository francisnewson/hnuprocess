#ifndef CONDITIONS_SELECTIONS_HH
#define CONDITIONS_SELECTIONS_HH

#include "Selection.hh"
#include "PolarityFinder.hh"
#include "Event.hh"

#if 0
/*
 *                      _ _ _   _                 
 *   ___ ___  _ __   __| (_) |_(_) ___  _ __  ___ 
 *  / __/ _ \| '_ \ / _` | | __| |/ _ \| '_ \/ __|
 * | (_| (_) | | | | (_| | | |_| | (_) | | | \__ \
 *  \___\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|___/
 *                                                
 *           _           _   _                 
 *  ___  ___| | ___  ___| |_(_) ___  _ __  ___ 
 * / __|/ _ \ |/ _ \/ __| __| |/ _ \| '_ \/ __|
 * \__ \  __/ |  __/ (__| |_| | (_) | | | \__ \
 * |___/\___|_|\___|\___|\__|_|\___/|_| |_|___/
 *                                             
 */
#endif

namespace fn
{

    //Check event polarity using run number
    class Polarity : public CachedSelection
    {
        public:
            Polarity( const fne::Event *e, int polarity);

        private:
            bool do_check() const;

            const fne::Event * e_;
            PolarityFinder pf_;
            int polarity_;

            REG_DEC_SUB(Polarity);
    };

    template<>
        Subscriber * create_subscriber<Polarity>
        (YAML::Node& instruct, RecoFactory& rf );

    //--------------------------------------------------
}
#endif
