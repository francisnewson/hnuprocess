#ifndef MUREC_HH
#define MUREC_HH
#include "Subscriber.hh"
#include "SingleMuon.hh"
#include "MuvGeom.hh"

#if 0
/*
 *  __  __       ____
 * |  \/  |_   _|  _ \ ___  ___
 * | |\/| | | | | |_) / _ \/ __|
 * | |  | | |_| |  _ <  __/ (__
 * |_|  |_|\__,_|_| \_\___|\___|
 *
 *
 */
#endif
namespace fne
{
    class Event;
    class MuonHit;
}
namespace fn
{
    class SingleTrack;

    class DataMuRec : public SingleMuon
    {
        public:
            DataMuRec( const fne::Event * e,  
                    const SingleTrack& st, double multiplier );

            void new_event();

            bool found() const {return found_; }
            double weight() const { return 1.0; };
            double x() const{ return x_;}
            double y() const{ return y_;}

            void debug_event();

        private:
            const fne::Event* e_;
            const SingleTrack& st_;
            double multiplier_;

            bool found_ ;
            double x_;
            double y_;
            double muon_time_;

            std::vector<int> possible_channels;
            std::vector<double> best_hit;
            std::vector<double> in_time_channels;

            bool print_debug_;

            MuvGeom muv_geom_;
    };

}
#endif

