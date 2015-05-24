#include "muon_selections.hh"
#include "muon_functions.hh"
#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "NA62Constants.hh"
#include "yaml_help.hh"
#include "FunctionCut.hh"
#include "TH2D.h"
#include "root_help.hh"

namespace fn
{

    REG_DEF_SUB( MuonReq );

    MuonReq::MuonReq( const SingleMuon& sm, 
            const SingleTrack& st, double multiplier )
        :sm_(sm), st_(st), multiplier_( multiplier)
    {}

    bool MuonReq::do_check() const
    {
        //check we have a muon
        if ( ! sm_.found() )
        {
            return false;
        }

        const auto& srt = st_.get_single_track();
        return check_muon_track_distance( sm_, srt, multiplier_ );
    }

    double MuonReq::do_weight() const
    {
        return sm_.found() * sm_.weight();
    }

    template<>
        Subscriber * create_subscriber<MuonReq>
        (YAML::Node& instruct, RecoFactory& rf )
        {

            const SingleTrack * st = get_single_track( instruct, rf );
            const SingleMuon * sm = get_single_muon( instruct, rf );
            double multiplier = get_yaml<double>( instruct, "multiplier" );

            return new MuonReq(*sm, *st, multiplier );
        }

    //--------------------------------------------------

    REG_DEF_SUB( NoAssMuon );

    NoAssMuon::NoAssMuon( const SingleMuon& sm, 
            const SingleTrack& st, double multiplier, const Selection& weighter, bool mc )
        :sm_(sm), st_(st), multiplier_( multiplier), weighter_( weighter ), mc_( mc )
    {}

    bool NoAssMuon::do_check() const
    {
        if ( mc_ )
        {
            return true;
        }
        else
        {
            return raw_pass();
        }
    }

    double NoAssMuon::do_weight() const
    {
        if ( !mc_) { return 1.0; }

        if (raw_pass() )
        {
            BOOST_LOG_SEV( get_log(), log_level() ) 
                << "raw_pass" ;
            return 1.0;
        }
        else
        {
            BOOST_LOG_SEV( get_log(), log_level() ) 
                << "weight: " << 1 - weighter_.get_weight()  ;
            return (1 - weighter_.get_weight() );
        }
    }

    bool NoAssMuon::raw_pass() const
    {
        //check if we have a muon
        if ( ! sm_.found() )
        {
            return true;
        }

        const auto& srt = st_.get_single_track();
        if ( !check_muon_track_distance( sm_, srt, multiplier_ ) )
        {
            return true;
        }
        return false;
    }

    template<>
        Subscriber * create_subscriber<NoAssMuon>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const SingleTrack * st = get_single_track( instruct, rf );
            const SingleMuon * sm = get_single_muon( instruct, rf );
            double multiplier = get_yaml<double>( instruct, "multiplier" );

            const Selection * weighter = rf.get_selection(
                    get_yaml<std::string>( instruct, "weighter" ) );

            bool mc = rf.is_mc() || rf.is_halo();

            return new NoAssMuon(*sm, *st, multiplier, *weighter, mc );
        }



    //--------------------------------------------------
    bool check_muon_track_distance
        ( const SingleMuon& sm, const SingleRecoTrack& srt, double multiplier )
        {
            std::pair<double,double> muon_track_separation
                = get_muon_track_separation( sm, srt );

            double dx = muon_track_separation.first;
            double dy = muon_track_separation.second;

            double mom = srt.get_mom();

            double err_x = multiplier * mu_error_0902_sc( mom, 2 );
            double err_y = multiplier * mu_error_0902_sc( mom, 1 );

            //Check separation is within error
            if ( fabs(dx) > err_x + na62const::muv_half_width )
            { return false; }

            if ( fabs(dy) > err_y + na62const::muv_half_width )
            { return false; }

            return true;
        }

    std::pair<double,double> get_muon_track_separation
        ( const SingleMuon& sm, const SingleRecoTrack& srt )
        {
            double muon_x = sm.x();
            double muon_y = sm.y();

            //extrapolate track
            double track_x = srt.extrapolate_ds( na62const::zMuv2).X();
            double track_y = srt.extrapolate_ds( na62const::zMuv1).Y();

            //find track - muon sepration
            double dx = track_x - muon_x;
            double dy = track_y - muon_y;

            return std::make_pair( dx, dy );
        }

    //--------------------------------------------------

    REG_DEF_SUB( MuonXYWeight );

    MuonXYWeight::MuonXYWeight( const SingleTrack& st,
            std::string muon_effs_file )
        :st_( st )
    {

        YAML::Node eff_conf = YAML::LoadFile(muon_effs_file );

        effs_ = Effs2D{ eff_conf["xbins"].as<std::vector<double>>(),
            eff_conf["ybins"].as<std::vector<double>>(),
            eff_conf["effs"].as<std::vector<double>>() };

        effs_.print_eff_map( std::cerr );
    }

    bool MuonXYWeight::do_check() const { return true; }

    double MuonXYWeight::do_weight() const 
    { 
        const SingleRecoTrack& srt = st_.get_single_track();

        //extrapolate track
        double x = srt.extrapolate_ds( na62const::zMuv2).X();
        double y = srt.extrapolate_ds( na62const::zMuv1).Y();

        double efficiency = effs_.efficiency( x, y );
        return efficiency;
    }

    template<>
        Subscriber * create_subscriber<MuonXYWeight>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            if( !rf.is_mc() ){ return new FunctionCut<auto_pass>{{0}} ; }

            const SingleTrack * st = get_single_track( instruct, rf );
            std::string effs_file = get_yaml<std::string>
                ( instruct, "effs_file" );

            return new MuonXYWeight( *st, effs_file );
        }

    //--------------------------------------------------

    REG_DEF_SUB( MuonTHXYWeight );

    MuonTHXYWeight::MuonTHXYWeight( const SingleTrack& st, const TH2D& heffs )
        :st_(st), effs_(heffs)
    {}

    bool MuonTHXYWeight::do_check() const { return true; }

    double MuonTHXYWeight::do_weight() const 
    { 
        const SingleRecoTrack& srt = st_.get_single_track();

        //extrapolate track
        double x = srt.extrapolate_ds( na62const::zMuv2).X();
        double y = srt.extrapolate_ds( na62const::zMuv1).Y();

        double efficiency = effs_.efficiency( x, y );
        return efficiency;
    }

    template<>
        Subscriber * create_subscriber<MuonTHXYWeight>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            if( ! (rf.is_mc() || rf.is_halo() ) ){ return new FunctionCut<auto_pass>{{0}} ; }

            const SingleTrack * st = get_single_track( instruct, rf );

            std::string effs_file = get_yaml<std::string>
                ( instruct, "effs_file" );

            std::string effs_hist_path = get_yaml<std::string>
                ( instruct, "effs_hist" );


            TFile teffs( effs_file.c_str() );
            auto eff_hist = extract_hist<TH2D>( teffs, effs_hist_path );

            return new MuonTHXYWeight( *st, *eff_hist );
        }

    //--------------------------------------------------

    REG_DEF_SUB( MuonTHPWeight );

    MuonTHPWeight::MuonTHPWeight( const SingleTrack& st , const TH1D& heffs )
        :st_( st ), effs_( heffs )
    {}

    bool MuonTHPWeight::do_check() const { return true; }

    double MuonTHPWeight::do_weight() const 
    { 
        const SingleRecoTrack& srt = st_.get_single_track();
        double mom = srt.get_mom();

        double efficiency = effs_.efficiency( mom );
        return efficiency;
    }

    template<>
        Subscriber * create_subscriber<MuonTHPWeight>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            if( ! (rf.is_mc() || rf.is_halo() ) ){ return new FunctionCut<auto_pass>{{0}} ; }

            const SingleTrack * st = get_single_track( instruct, rf );

            std::string effs_file = get_yaml<std::string>
                ( instruct, "effs_file" );

            std::string effs_hist_path = get_yaml<std::string>
                ( instruct, "effs_hist" );


            TFile teffs( effs_file.c_str() );
            if ( teffs.IsZombie() ){ throw std::runtime_error(
                    "Couldn't open " + effs_file ); }

            teffs.Print();
            teffs.ls();

            auto eff_hist = extract_hist<TH1D>( teffs, effs_hist_path );

            return new MuonTHPWeight( *st, *eff_hist );
        }


}
