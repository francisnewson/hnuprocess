#ifndef MCZCUT_HH
#define MCZCUT_HH
#include "Selection.hh"
#include "Event.hh"
#if 0
/*
 *  __  __  ____ _________      _
 * |  \/  |/ ___|__  / ___|   _| |_
 * | |\/| | |     / / |  | | | | __|
 * | |  | | |___ / /| |__| |_| | |_
 * |_|  |_|\____/____\____\__,_|\__|
 *
 *
 */
#endif
namespace fn
{
    class MCZCut : public CachedSelection
    {
        public:
            MCZCut ( const fne::Event * e, bool mc,  double min_z, double max_z );

        private:
            bool do_check() const;
            const fne::Event * e_;
            bool mc_;
            double min_z_;
            double max_z_;

            REG_DEC_SUB( MCZCut );
    };

    template<>
        Subscriber * create_subscriber<MCZCut>
        (YAML::Node& instruct, RecoFactory& rf );
}
#endif
