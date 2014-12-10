#ifndef NA62CONSTANTS_HH
#define NA62CONSTANTS_HH
#include <type_traits> 

#if 0
/*
 *  _   _    _    __  ____
 * | \ | |  / \  / /_|___ \
 * |  \| | / _ \| '_ \ __) |
 * | |\  |/ ___ \ (_) / __/
 * |_| \_/_/   \_\___/_____|
 *   ____                _              _
 *  / ___|___  _ __  ___| |_ __ _ _ __ | |_ ___
 * | |   / _ \| '_ \/ __| __/ _` | '_ \| __/ __|
 * | |__| (_) | | | \__ \ || (_| | | | | |_\__ \
 *  \____\___/|_| |_|___/\__\__,_|_| |_|\__|___/
 *
*/
#endif

namespace na62const
{

    constexpr double mPi   = 0.139570;
    constexpr double mPi0  = 0.1349766;
    constexpr double mK    = 0.493667;
    constexpr double mMu   = 0.105659;
    constexpr double mE    = 5.109e-4;

    constexpr double mK2 =  mK * mK;
    constexpr double mMu2 = mMu * mMu;

    constexpr double zFinalCollimator = -1950;

    constexpr double zLkr  = 12108.2;
    constexpr double rLkrPojection = 10998.0;

    constexpr double bz_tracking =  9700.62;
    constexpr double z_tracking =  11880.2;

    constexpr double zDch1 = 9708.42;
    constexpr double zMagnet = 10929.3;
    constexpr double zDch4 = 11888.0;

    //This is wrong in the ComPACT database
    //constexpr double zMuv1 = 12740.0;
    
    constexpr double zMuv1 = 12778.0; //correct value from KM
    constexpr double zMuv2 = 12902.8;
    constexpr double zMuv3 = 13012.0;

    constexpr double pi = 3.141592653589793238462643383279502884;

}
#endif
