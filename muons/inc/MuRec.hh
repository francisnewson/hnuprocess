#ifndef MUREC_HH
#define MUREC_HH
#include "Subscriber.hh"
#include "SingleMuon.hh"
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

    class MuvGeom
    {
        public:
            MuvGeom();
            int num_channels();
            int num_strips();
            double time_cut();

            int which_H_strips0902( double x, double y, double err, int istrip );
            int which_V_strips0902( double x, double y, double err, int istrip );

            float scint_speed( int chan  );

            double channel_time_0902( fne::MuonHit * mh, double hod_time, 
                    double  x1_track, double y1_track, double v);

            std::pair<double,double> rec_xy0902( std::vector<double>&in_time_channels);

            float muonchan2pos(int chan);
            int muonpos2chan(float pos, int plane);

        private:

            std::array<double,56> scint_speed_;
            std::array<double,11> V_centres_; 
            std::array<double,11> H_centres_;

            std::array<double,56>::size_type num_channels_;
            std::array<double,11>::size_type num_strips_;
            double time_cut_;
    };

    class DataMuRec : public SingleMuon
    {
        public:
            DataMuRec( const fne::Event * e,  const SingleTrack& st, double multiplier );

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

