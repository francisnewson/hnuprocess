#ifndef MUVGEOM_HH
#define MUVGEOM_HH
#include <vector>
#include <array>
#if 0
/*
 *  __  __              ____
 * |  \/  |_   ___   __/ ___| ___  ___  _ __ ___
 * | |\/| | | | \ \ / / |  _ / _ \/ _ \| '_ ` _ \
 * | |  | | |_| |\ V /| |_| |  __/ (_) | | | | | |
 * |_|  |_|\__,_| \_/  \____|\___|\___/|_| |_| |_|
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
    class MuvGeom
    {
        public:
            MuvGeom();
            int num_channels()const ;
            int num_strips()const ;
            double time_cut()const ;

            int which_H_strips0902( double x, double y, double err, int istrip )const ;
            int which_V_strips0902( double x, double y, double err, int istrip )const ;

            double V_centres( int i ) const { return V_centres_[i]; }
            double H_centres( int i ) const { return H_centres_[i]; }

            float scint_speed( int chan  ) const;

            double channel_time_0902( fne::MuonHit * mh, double hod_time, 
                    double  x1_track, double y1_track, double v) const;

            std::pair<double,double> rec_xy0902( std::vector<double>&in_time_channels) const;

            float muonchan2pos(int chan) const;
            int muonpos2chan(float pos, int plane) const;

        private:

            const std::array<double,56> scint_speed_;
            const std::array<double,11> V_centres_; 
            const std::array<double,11> H_centres_;

            std::array<double,56>::size_type num_channels_;
            std::array<double,11>::size_type num_strips_;
            double time_cut_;
    };
}
#endif
