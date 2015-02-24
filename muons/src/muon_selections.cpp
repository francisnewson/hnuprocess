#include "muon_selections.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "NA62Constants.hh"
#include "muon_functions.hh"
#include "SingleMuon.hh"

namespace fn
{
    REG_DEF_SUB( MuonReqStatus );

    MuonReqStatus::MuonReqStatus( const MuonVeto& muv,
            std::set<int> allowed_status )
        :muv_( muv ), allowed_status_( allowed_status )
    {}

    bool MuonReqStatus::do_check() const
    {
        int status = muv_.get_muv_status();
        return allowed_status_.find( status )!= allowed_status_.end() ;
    }

    template<>
        Subscriber * create_subscriber<MuonReqStatus>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const MuonVeto*  muv = get_muon_veto( instruct, rf );
            std::vector<int> allowed_status_list = 
                get_yaml<std::vector<int>>( instruct, "allowed_status" );

            std::set<int> allowed_status
                ( begin( allowed_status_list ), end( allowed_status_list ) );

            return new MuonReqStatus( *muv, allowed_status );
        }

    //--------------------------------------------------

    REG_DEF_SUB( NoMuv );

    NoMuv::NoMuv( const fne::Event * e )
        :e_( e ){}

    bool NoMuv::do_check() const
    {
        return (e_->detector.nmuons == 0 );
    }

    template<>
        Subscriber * create_subscriber<NoMuv>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            (void ) instruct;

            const fne::Event *  event = rf.get_event_ptr();

            return new NoMuv( event );
        }

    //--------------------------------------------------

    REG_DEF_SUB( CombinedMuonVeto );

    CombinedMuonVeto::CombinedMuonVeto( const fne::Event * e,
            Eff2D muv_eff, const SingleMuon& sm, const SingleTrack& st,
            double range_scale, bool mc )
        :e_(e), sm_( sm ), st_( st ), range_scale_( range_scale), muv_eff_( muv_eff ), 
        pf_( "input/reco/conditions/run_polarity.dat" ),
        mc_( mc )
    {}

    bool CombinedMuonVeto::do_check() const
    {
        //require a muon
        if ( ! sm_.found_muon()  ){ return false; }

        const auto& muon = sm_.get_muon();

        //require status 1 or 2
        if ( ! ( muon->status == 1 || muon->status == 2 ) ) { return false; }

        //check track distance
        const auto& srt = st_.get_single_track();

        double mom = srt.get_mom();

        double track_x = srt.extrapolate_ds( na62const::zMuv2).X();
        double track_y = srt.extrapolate_ds( na62const::zMuv1).Y();

        double muon_x = muon->x;
        double muon_y = muon->y;

        double dx = track_x - muon_x;
        double dy = track_y - muon_y;

        double error_x = range_scale_ * mu_error_0902_sc( mom, 2 );
        double error_y = range_scale_ * mu_error_0902_sc( mom, 1 );

        if ( fabs(dx) > error_x + na62const::muv_half_width ){ return false; }
        if ( fabs(dy) > error_y + na62const::muv_half_width ){ return false; }

        return true;
    }

    double CombinedMuonVeto::do_weight() const
    {
        if ( !mc_ )
        {
            return 1; //no weighting for data
        }

        //propagate MC muon to MUV
        //find muon impact point
        std::size_t muon_pos = find_muon( e_->mc );

        if ( muon_pos == e_->mc.npart )
        {
            return 1; // no MC muon
        }

        std::pair<double, double>  impact_point = muv_impact(
                e_->mc, muon_pos, pf_.get_polarity( e_->header.run ) );

        double efficiency = muv_eff_.efficiency( 
                impact_point.first, impact_point.second);

        return efficiency;
    }

    template<>
        Subscriber * create_subscriber<CombinedMuonVeto>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();

            bool is_mc = rf.is_mc();

            std::string eff_file = get_yaml<std::string>( instruct, "effs_file" );
            YAML::Node eff_conf = YAML::LoadFile( eff_file );

            const SingleMuon* sm = get_single_muon( instruct, rf );
            const SingleTrack* st = get_single_track( instruct, rf );

            double range_scale = get_yaml<double>( instruct, "range_scale" );

            return new CombinedMuonVeto( event,
                    Eff2D(  eff_conf["xbins"].as<std::vector<double>>(),
                        eff_conf["ybins"].as<std::vector<double>>(),
                        eff_conf["effs"].as<std::vector<double>>() ),
                    *sm, *st,  range_scale, is_mc);
        }

    //--------------------------------------------------

    double mu_error_0902_sc( double mom, int muv_plane )
    {
        //Copied from compact/rlib/anasrc/murec0902.c

        double total_rad_lengths = 
            + na62const::len_lkr / na62const::X0_lkr /* scattering in LKr */
            + na62const::len_hac / na62const::X0_iron /* scattering in HAC */
            + muv_plane * na62const::len_muv_iron / na62const::X0_iron; /* scattering in MUVs */

        double zMuv = 0;

        switch( muv_plane )
        {
            case 1: zMuv = na62const::zMuv1; break;
            case 2: zMuv = na62const::zMuv2; break; 
            case 3: zMuv = na62const::zMuv2; break;
            default:
                    throw std::runtime_error(
                            "Unknown MUV plane: " + std::to_string( muv_plane ) );
        }

        double p = 1000 * mom; //GeV to MeV to match COmPACT

            double scattering_error = ( zMuv - na62const::zLkr )
            * (13.6 / p) * std::sqrt( total_rad_lengths / 3 );

        double dch_error = ( (0.1 + 8.5 * (1000 / p ) ) / 10000 ) 
            * ( zMuv - na62const::z_tracking ) ;

        double total_error  = std::hypot( scattering_error, dch_error );

        return total_error;
    }

    //--------------------------------------------------
}
