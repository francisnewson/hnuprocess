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
    std::size_t find_muon( const fne::Mc& mce );
    double mu_error_0902_sc( double mom, int muv_plane );
}

#endif 
