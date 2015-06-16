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

}
