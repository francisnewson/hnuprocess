#ifndef EHELP_HH
#define EHELP_HH
#include "TObject.h"
#include <boost/optional.hpp>
#if 0
/*
 *  _____ _   _      _
 * | ____| | | | ___| |_ __
 * |  _| | |_| |/ _ \ | '_ \
 * | |___|  _  |  __/ | |_) |
 * |_____|_| |_|\___|_| .__/
 *                    |_|
 *
 */
#endif
class TTree;
namespace fn
{

    template <class E>
        boost::optional<bool> is_mc( TTree* t )
        {
            return boost::none;
        };
}
#endif
