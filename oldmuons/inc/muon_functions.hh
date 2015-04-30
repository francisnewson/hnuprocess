#ifndef MUON_FUNCTIONS_HH
#define MUON_FUNCTIONS_HH
#include <utility>
#if 0
/*
 *                                   __                  _   _
 *  _ __ ___  _   _  ___  _ __      / _|_   _ _ __   ___| |_(_) ___  _ __  ___
 * | '_ ` _ \| | | |/ _ \| '_ \    | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * | | | | | | |_| | (_) | | | |   |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_| |_| |_|\__,_|\___/|_| |_|___|_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 *                            |_____|
 *
 */
#endif

namespace fne
{
    class Mc;
}

namespace fn
{
    std::pair<double, double> muv_impact(
            const fne::Mc & mce, std::size_t muon_pos, int polarity );

    std::size_t find_muon( const fne::Mc& mce );
}

#endif 
