#include "MuvGeom.hh"
#include <cmath>
#include "NA62Constants.hh"
#include "MuonHit.hh"
#include "MuonHit.hh"

namespace fn
{
    MuvGeom::MuvGeom()
        : scint_speed_{{ 
            16.45, 16.13, 16.56, 15.19,
                15.99, 16.29, 14.83, 15.14,
                16.36, 17.03, 15.69, 15.70,
                16.34, 16.56, 15.60, 15.86,
                16.49, 16.13, 15.44, 15.83,
                21.00, 22.12, 20.16, 19.98,
                16.68, 16.24, 15.38, 15.61,
                16.15, 16.73, 15.08, 15.23,
                16.79, 16.44, 15.84, 15.16,
                17.21, 16.66, 16.00, 15.36,
                15.45, 16.62, 15.30, 15.21,
                15.91, 16.34, 14.32, 14.98,
                15.40, 14.80, 16.67, 18.49,
                18.03, 16.53, 17.35, 16.10 }},

        V_centres_{{
            122.66, 97.84, 72.89, 48.06, 23.13, 0.00,
            -24.63, -49.55, -74.38, -99.28, -124.16} },

        H_centres_{{-123.35, -98.40, -73.63, -48.76, -24.00, 
            0.00, 23.00, 47.93, 72.73, 97.60, 122.37}},

        num_channels_{scint_speed_.size() },
        num_strips_ {V_centres_.size() },
        time_cut_ { 10.0 }
    {}

    int MuvGeom::num_channels() const{ return static_cast<int>( num_channels_  ); }
    int MuvGeom::num_strips() const{ return static_cast<int>( num_strips_  ); }
    double MuvGeom::time_cut() const{ return time_cut_; }

    float MuvGeom::scint_speed( int chan ) const{ return scint_speed_[chan]; }

    int MuvGeom::which_H_strips0902( double x, double y, double err, int istrip ) const
    {
        double x_tube_hole = 10.6;

        if  ( ( istrip == 5 ) && fabs( x) < (x_tube_hole - err ) )
        {
            return 0;
        }

        if ( fabs(y - H_centres_[istrip]) < ( err + na62const::muv_half_width ) )
        {
            return 1;
        }
        return 0;
    }

    int MuvGeom::which_V_strips0902( double x, double y, double err, int istrip ) const
    {
        double y_tube_hole = 10.6;


        if ( ( istrip == 5 ) && ( fabs( y ) < ( y_tube_hole - err ) ) )
        {
            return 0;
        }

        if ( fabs( x - V_centres_[istrip] )  < ( err + na62const::muv_half_width ) )
        {
            return 1;
        }

        return 0;
    }

    double MuvGeom::channel_time_0902( fne::MuonHit * mh, double hod_time, 
            double  x1_track, double y2_track, double v )  const
    {
        double muv_hit_time = 0;
        int chan = mh->channel;

        if (chan <= num_strips_)
        {
            // HL channel
            muv_hit_time = mh->time + (x1_track/v);
        }
        else if (chan<=(num_strips_*2))
        {
            // HR channel
            muv_hit_time = mh->time - (x1_track/v);
        }
        else if (chan<=(num_strips_*3))
        {
            // VT channel 
            muv_hit_time = mh->time + (y2_track/v);
        }
        else if (chan<=(num_strips_*4))
        {
            // VB channel
            muv_hit_time = mh->time - (y2_track/v);
        }
        return  fabs( muv_hit_time - hod_time );
    }

    float MuvGeom::muonchan2pos(int chan) const
    {
        float pos = -9999;

        if (chan <= num_strips_)		
        {
            pos = H_centres_[chan - 1];
        }
        else if (chan <= (num_strips_ * 2))
        {
            pos = H_centres_[chan - 1 - num_strips_];
        }
        else if (chan <= (num_strips_*3))
        {
            pos = V_centres_[chan - 1 - 2 * num_strips_];
        }
        else if (chan <= (num_strips_*4))
        {
            pos = V_centres_[chan - 1 - 3 * num_strips_];
        }

        return pos;
    }

    // Return channel number given a position and plane
    // channel number is NOT zero based
    int MuvGeom::muonpos2chan(float pos, int plane) const
    {
        int chan = -1;
        int i;

        switch (plane)
        {
            case 1:
                for (i = 0; i< num_strips_; i++)
                {
                    if ((pos > (H_centres_[i] - na62const::muv_half_width)) && (pos < (H_centres_[i] + na62const::muv_half_width)))
                    {
                        chan = i + 1;
                    }
                }
                break;
            case 2:
                for (i = 0; i< num_strips_; i++)
                {
                    if ((pos > (V_centres_[i] - na62const::muv_half_width)) && (pos < (V_centres_[i] + na62const::muv_half_width)))
                    {
                        chan = i + 2 * num_strips_ + 1;
                    }
                }
        }

        return chan;
    }

    std::pair<double,double> MuvGeom::rec_xy0902( std::vector<double>&in_time_channels ) const
    {
        double total_x = 0 ;
        double total_y = 0 ;

        int ix = 0;
        int iy = 0;

        int num_channels_ = 56;

        for ( int i = 0; i != num_channels_ ; ++i )
        {
            if (! in_time_channels[i] )
            { continue; }

            if ( i < num_strips_ )
            {
                iy++;
                total_y += H_centres_[i];
            }
            else if ( i < num_strips_ * 2 )
            {
                iy++;
                total_y += H_centres_[i - num_strips_];
            }
            else if ( i < num_strips_ * 3 )
            {
                ix++;
                total_x += V_centres_[i - num_strips_ * 2 ];
            }
            else if ( i < num_strips_ * 4 )
            {
                ix++;
                total_x += V_centres_[ i - num_strips_ * 3 ];
            }
        }

        return std::make_pair( total_x / double( ix ), total_y / double( iy ) );
    }

}
