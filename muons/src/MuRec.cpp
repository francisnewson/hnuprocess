#include "MuRec.hh"
#include "MuonHit.hh"
#include "SingleTrack.hh"
#include "muon_functions.hh"
#include "NA62Constants.hh"
#include <array>

namespace fn
{

    DataMuRec::DataMuRec( const fne::Event * e,  const SingleTrack& st, double multiplier )
        :e_ ( e ), st_( st ), multiplier_( multiplier ), print_debug_( false )
    { }

    void DataMuRec::new_event()
    {
        if ( !st_.found_single_track() )
        {
            found_ = false;
            return;
        }

        const SingleRecoTrack& srt = st_.get_single_track();
        double mom = srt.get_raw_mom();

        //predict xy track impact points
        double x1_track = srt.extrapolate_ds( na62const::zMuv1 ).X();
        double y1_track = srt.extrapolate_ds( na62const::zMuv1 ).Y();
        double err_1_track = multiplier_ * mu_error_0902_sc( mom, 1 );

        double x2_track = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double y2_track = srt.extrapolate_ds( na62const::zMuv2 ).Y();
        double err_2_track = multiplier_ * mu_error_0902_sc( mom, 2 );

        double hod_time = srt.get_best_time();

        possible_channels = std::vector<int>( muv_geom_.num_channels(), 0 );
        best_hit = std::vector<double>( muv_geom_.num_channels(), muv_geom_.time_cut() );

        //decide which strips to accept
        for ( int istrip = 0 ; istrip < muv_geom_.num_strips() ; ++istrip )
        {
            //MUV1 strips
            if ( muv_geom_.which_H_strips0902( x1_track, y1_track, err_1_track, istrip ) )
            {
                if ( istrip == 5 )
                {
                    if ( x1_track > 0 ) 
                    {
                        possible_channels[istrip] = 1;
                    }
                    else
                    {
                        possible_channels[istrip + muv_geom_.num_strips()] = 1;
                    }
                }
                else
                {
                    possible_channels[istrip] = 1;
                    possible_channels[istrip+muv_geom_.num_strips()] = 1;
                }
            }

            //MUV2 strips
            if ( muv_geom_.which_V_strips0902( x2_track, y2_track, err_2_track, istrip ) )
            {
                if ( istrip == 5 )
                {
                    if ( y2_track > 0 ) 
                    {
                        possible_channels[istrip + 2 * muv_geom_.num_strips()] = 1;
                    }
                    else
                    {
                        possible_channels[istrip + 3 * muv_geom_.num_strips()] = 1;
                    }
                }
                else
                {
                    possible_channels[istrip + 2 * muv_geom_.num_strips()] = 1;
                    possible_channels[istrip + 3 * muv_geom_.num_strips()] = 1;
                }
            }
        }

        in_time_channels =  std::vector<double> ( muv_geom_.num_channels(), 0.0 );
        int num_in_time_hits = 0;

        //look through the hits
        for ( int ipmuon = 0 ; ipmuon != e_->detector.nmuonhits ; ++ ipmuon )
        {
            fne::MuonHit * mh = static_cast<fne::MuonHit*>( e_->detector.muon_hits[ipmuon] );
            int chan = mh->channel;
            double pmuon_time = mh->time;

            if ( possible_channels[chan-1] )
            {
                double channel_time = muv_geom_.channel_time_0902
                    ( mh, hod_time, x1_track, y2_track, muv_geom_.scint_speed(chan-1) );

                if ( channel_time < best_hit[chan-1] )
                {
                    best_hit[chan-1] = channel_time;
                    ++num_in_time_hits;
                    in_time_channels[chan-1] = pmuon_time - hod_time;
                }
            }
        }

        bool hit_plane_1 = false;
        bool hit_plane_2 = false;

        double tp1 = 0.0;
        double tp2 = 0.0;

        int np1 = 0;
        int np2 = 0;

        int last_chan = -1;

        int num_strips_ = muv_geom_.num_strips();

        //Get a reconstructed time
        for ( int ipmuon = 0 ; ipmuon != e_->detector.nmuonhits ; ++ ipmuon )
        {
            fne::MuonHit * mh = static_cast<fne::MuonHit*>( e_->detector.muon_hits[ipmuon] );
            int chan = mh->channel;

            if ( chan == last_chan ){ continue; }
            last_chan = chan;

            if ( !in_time_channels[chan-1] ){ continue; }

            //MUV1
            if ( chan <= num_strips_ * 2 )
            {
                hit_plane_1 = true;

                if ( print_debug_ )
                {
                    std::cout << chan << " " << hit_plane_1 << "\n" ;
                }

                if ( chan <= num_strips_ )
                {
                    if ( in_time_channels[ chan - 1 + num_strips_ ] )
                    {
                        //HL and HR
                        tp1 += in_time_channels[chan - 1] + in_time_channels[chan - 1 + num_strips_];
                        np1 += 2;
                    }
                    else
                    {
                        //HL only
                        tp1 += in_time_channels[chan - 1] + (x1_track / muv_geom_.scint_speed(chan - 1) ) ;
                        np1 += 1;
                    }
                }
                else
                {
                    //HR only
                    if ( !in_time_channels[ chan - 1 - num_strips_ ] )
                    {
                        tp1 += in_time_channels[chan - 1 ] - ( x1_track / muv_geom_.scint_speed(chan - 1) );
                        np1 += 1;
                    }
                }
            }

            //MUV2
            else if (chan <= num_strips_ * 4 )
            {
                hit_plane_2 = true;

                if ( chan <= (3 * num_strips_ ) )
                {
                    if ( in_time_channels[ chan - 1 + num_strips_ ] )
                    {
                        //VT and VB
                        tp2 += in_time_channels[chan - 1] + in_time_channels[chan - 1 + num_strips_];
                        np2 += 2;
                    }
                    else
                    {
                        //VT only
                        tp2 += in_time_channels[chan - 1] + (y2_track / muv_geom_.scint_speed(chan - 1) ) ;
                        np2 += 1;
                    }
                }
                else if ( chan <= ( num_strips_ * 4 ) )
                {
                    //VB only
                    if ( !in_time_channels[ chan - 1 - num_strips_ ] )
                    {
                        tp2 += in_time_channels[chan - 1 ] - ( y2_track / muv_geom_.scint_speed(chan - 1) );
                        np2 += 1;
                    }
                }
            }

        }

        found_ = hit_plane_1 && hit_plane_2 ;

        muon_time_ = ( tp1 + tp2 ) / ( np1 + np2 );

        if ( !found_ ){ return; }

        //compute x and y positions

        std::pair<double,double> xy = muv_geom_.rec_xy0902( in_time_channels );

        x_ = xy.first;
        y_ = xy.second;
    }


    void DataMuRec::debug_event() 
    {
        std::cout << "Event: " << e_->header.burst_time << " " << e_->header.compact_number << "\n";
        std::cout << "time: " << muon_time_ << "\n" ;

        const SingleRecoTrack& srt = st_.get_single_track();
        double hod_time = srt.get_best_time();
        std::cout << "Hits: \n" ;

        //predict xy track impact points
        double x1_track = srt.extrapolate_ds( na62const::zMuv1 ).X();
        double y1_track = srt.extrapolate_ds( na62const::zMuv1 ).Y();

        double x2_track = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double y2_track = srt.extrapolate_ds( na62const::zMuv2 ).Y();

        //look through the hits
        for ( int ipmuon = 0 ; ipmuon != e_->detector.nmuonhits ; ++ ipmuon )
        {
            fne::MuonHit * mh = static_cast<fne::MuonHit*>( e_->detector.muon_hits[ipmuon] );
            int chan = mh->channel;
            double pmuon_time = mh->time;

            double channel_time = muv_geom_.channel_time_0902
                ( mh, hod_time, x1_track, y2_track, muv_geom_.scint_speed( chan-1 ) );

            std::cout << chan << " " << pmuon_time - hod_time  <<  " " << channel_time 
                <<  " " << muv_geom_.muonchan2pos( chan ) << "\n";
        }

        std::cout << "in time: \n";
        for( int ichan = 0 ; ichan != muv_geom_.num_channels() ; ++ichan )
        {
            if ( fabs(best_hit[ichan]) < muv_geom_.time_cut() || true )
            {
                std::cout << ichan << " " << best_hit[ichan] << "\n";
            }
        }

        if ( e_->header.burst_time == 1188422944 && e_->header.compact_number ==10884 )
        {
            print_debug_ = true;
            new_event();
            print_debug_ = false;
        }
    }

    //--------------------------------------------------

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

    int MuvGeom::num_channels(){ return static_cast<int>( num_channels_  ); }
    int MuvGeom::num_strips(){ return static_cast<int>( num_strips_  ); }
    double MuvGeom::time_cut(){ return time_cut_; }

    float MuvGeom::scint_speed( int chan ){ return scint_speed_[chan]; }

    int MuvGeom::which_H_strips0902( double x, double y, double err, int istrip )
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

    int MuvGeom::which_V_strips0902( double x, double y, double err, int istrip )
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
            double  x1_track, double y2_track, double v ) 
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

    float MuvGeom::muonchan2pos(int chan)
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
    int MuvGeom::muonpos2chan(float pos, int plane)
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

    std::pair<double,double> MuvGeom::rec_xy0902( std::vector<double>&in_time_channels )
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
