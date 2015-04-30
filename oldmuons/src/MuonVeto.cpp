#include "MuonVeto.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "NA62Constants.hh"
#include <stdexcept>
#include "Track.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "Xcept.hh"
#include "yaml_help.hh"
#include "muon_functions.hh"

namespace fn
{

    //Convert muon planes into hit status
    int get_muon_status( bool muv1, bool muv2, bool muv3 )
    {
        if ( (!muv1) && (!muv2) ){ return 0; }
        else if ( muv1 && muv2 && muv3 ){ return 1; }
        else if ( muv1 && muv2 && !muv3 ){ return 2; }
        else if ( !muv1 && muv2 && muv3 ){ return 3; }
        else if ( muv1 && !muv2 && muv3 ){ return 4; }
        else { throw std::logic_error{ "MUVSTATUS" }; }
    }
    //--------------------------------------------------

    REG_DEF_SUB( MuonVeto );

    void MuonVeto::new_event() { dirty_ = true; }

    //Store result of Muon processing
    void MuonVeto::set_muvs(  bool muv1, bool muv2, bool muv3 ) const
    {
        muv1_ = muv1;
        muv2_ = muv2;
        muv3_ = muv3;
        status_ = get_muon_status( muv1, muv2, muv3 );
    }

    //Implement cacheing
    int MuonVeto::get_muv_status()  const
    { 
        if ( dirty_ ){ process_event(); }
        return status_;
    }
    bool MuonVeto::get_muv1() const
    {
        if ( dirty_){ process_event(); }
        return muv1_;
    }
    bool MuonVeto::get_muv2() const
    {
        if ( dirty_){ process_event(); }
        return muv2_;
    }
    bool MuonVeto::get_muv3() const
    {
        if ( dirty_){ process_event(); }
        return muv3_;
    }

    //--------------------------------------------------

    //factory function
    template<>
        Subscriber * create_subscriber<MuonVeto>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = instruct["method"].as<std::string>();

            //all versions need an event pointer
            const fne::Event * event = rf.get_event_ptr();

            if ( method == "Data" )
            {
                return new DataMuonVeto{ event };
            }

            else if ( method == "DataMatched" )
            {
                const SingleTrack* st = get_single_track( instruct, rf );
                return new DataMatchedMuonVeto{ event, *st };
            }

            else if ( method == "MC" )
            {
                const SingleTrack* st = get_single_track( instruct, rf );

                std::vector<double> muv1_effs = 
                    get_yaml<std::vector<double>>( instruct, "muv1_effs" );

                if ( muv1_effs.size()  != 11 )
                {
                    throw std::runtime_error( "We need exactly 11 effs for MUV1" );
                }

                std::vector<double> muv2_effs = 
                    get_yaml<std::vector<double>>( instruct, "muv2_effs" );

                if ( muv2_effs.size()  != 11 )
                {
                    throw std::runtime_error( "We need exactly 11 effs for MUV2" );
                }

                return new MCMuonVeto{ event, *st, muv1_effs, muv2_effs };
            }
            else if ( method == "MCXY") 
            {
                std::string eff_file = get_yaml<std::string>( instruct, "effs_file" );
                const SingleTrack* st = get_single_track( instruct, rf );

                YAML::Node eff_conf = YAML::LoadFile( eff_file );
                return new MCXYMuonVeto( event, *st,
                        Eff2D(  eff_conf["xbins"].as<std::vector<double>>(),
                            eff_conf["ybins"].as<std::vector<double>>(),
                            eff_conf["effs"].as<std::vector<double>>() ) );
            }
            else
            {
                throw Xcept<UnknownMuonVetoMethod>( method );
            }
        }


    //--------------------------------------------------

    //**************************************************
    //Data Muon Veto
    //**************************************************

    DataMuonVeto::DataMuonVeto( const fne::Event * e )
        :e_( e ){}


    void DataMuonVeto::process_event() const
    {
        const TClonesArray& muons = e_->detector.muons;
        Int_t nmuons = e_->detector.nmuons;

        if ( nmuons > 0 ){ found_muon_ = true; }
        else { found_muon_ = false;}

        bool muv1_ = false;
        bool muv2_ = false;
        bool muv3_ = false;

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

            Int_t status = rm->status;

            //any muon will do
            if ( status != 3 ){ muv1_ = true; }
            if ( status != 4 ){ muv2_ = true; }
            if ( status != 2 ){ muv3_ = true; }
        }

        //overall status
        set_muvs( muv1_, muv2_, muv3_ );
    }

    bool DataMuonVeto::found_muon() const
    {
        return found_muon_;
    }

    //--------------------------------------------------

    //**************************************************
    //Data Matched Muon Veto
    //**************************************************
    DataMatchedMuonVeto::DataMatchedMuonVeto( const fne::Event * e ,
            const SingleTrack& st )
        :e_( e ), st_(st ){}


    void DataMatchedMuonVeto::process_event() const
    {
        const TClonesArray& muons = e_->detector.muons;
        Int_t nmuons = e_->detector.nmuons;
        const SingleRecoTrack & srt = st_.get_single_track();

        int track_compact = srt.get_compact_id();

        bool muv1_ = false;
        bool muv2_ = false;
        bool muv3_ = false;
        found_muon_ = false;

        muon_position_.SetXYZ( 0, 0, 0 );

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

            found_muon_ = true;
            muon_position_.SetXYZ( rm->x, rm->y, 0 );

            //only accept muons which are matched with track
            if ( rm->iTrk == track_compact )
            {
                Int_t status = rm->status;

                if ( status != 3 ){ muv1_ = true; }
                if ( status != 4 ){ muv2_ = true; }
                if ( status != 2 ){ muv3_ = true; }
            }
        }

        //overall status
        set_muvs( muv1_, muv2_, muv3_ );
    }

    TVector3 DataMatchedMuonVeto::get_muon_position() const
    {
        return muon_position_;
    }

    bool DataMatchedMuonVeto::found_muon() const
    {
        return found_muon_;
    }

    //--------------------------------------------------

    //**************************************************
    //MC Muon Veto
    //**************************************************

    MCMuonVeto::MCMuonVeto( const fne::Event * e,
            const SingleTrack& st, 
            std::vector<double> muv1_effs,
            std::vector<double> muv2_effs
            )
        :e_(e), st_(st), pf_( "input/reco/conditions/run_polarity.dat" ),
        muv1_effs_( muv1_effs), muv2_effs_( muv2_effs ),
        uni_dist_( 0.0, 1.0 )
    { }

    void MCMuonVeto::process_event() const
    {
        found_muon_ = false;

        const SingleRecoTrack & srt = st_.get_single_track();

        //Get x coord from MUV2 and y coord from MUV1
        double xMuv2 = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double yMuv1 = srt.extrapolate_ds( na62const::zMuv1 ).Y();

        muon_position_.SetXYZ( xMuv2, yMuv1, 0 );

        int muv1_strip = lround(5.0 + (5.5 * yMuv1 / 140.0) );
        double muv_1_eff = (muv1_strip >0 && muv1_strip < 11) ? muv1_effs_.at(muv1_strip) : 0;

        int muv2_strip = lround(5.5 + (5.5 * xMuv2 / 140.0) );
        double muv_2_eff = (muv2_strip >0 && muv2_strip < 11) ? muv2_effs_.at(muv2_strip) : 0;

        //generate status
        bool  muv1_ = uni_dist_( generator_ ) < muv_1_eff;
        bool  muv2_ = uni_dist_( generator_ ) < muv_2_eff;
        bool  muv3_ = true;

        if ( muv1_ || muv2_ ) { found_muon_ = true; }

        set_muvs( muv1_, muv2_, muv3_);
    }

    TVector3 MCMuonVeto::get_muon_position() const
    {
        return muon_position_;
    }

    bool MCMuonVeto::found_muon() const
    {
        return found_muon_;
    }

    //--------------------------------------------------

    //**************************************************
    //Eff2D
    //**************************************************

    Eff2D::Eff2D( std::vector<double> xedges, 
            std::vector<double> yedges, 
            std::vector<double> effs )
        :xedges_( xedges ), yedges_( yedges ), effs_( effs )
    {
        std::size_t nbins = ( xedges_.size() -1 ) * (yedges_.size() -1 );
        if ( effs_.size() != nbins )
        {
            throw std::runtime_error( "Eff2D: Wrong number of effs" );
        }

        std::cout << "MUVEFFS\n";

        for ( int j = yedges_.size()-2 ; j != 0 ; --j )
        {
            for ( int i = 0; i !=  xedges_.size() -2 ; ++i )

            {
                int bin = (j) * ( xedges_.size() -1 ) + (i );
                try
                {
                double eff = effs_.at( bin );
                std::cout << ( eff > 0.1 ) << " " ;
                }
                catch( std::out_of_range& e )
                {
                    std::cout << e.what() << std::endl;
                    std::cout << i << " " << j << " "<< bin << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }

    double Eff2D::efficiency( double x, double y ) const
    {
        //Check we are in range of grid
        if ( x <= xedges_.front() || x >= xedges_.back() ){ return 0; }
        if ( y <= yedges_.front() || y >= yedges_.back() ){ return 0; }

        std::size_t xbin = get_bin( x, xedges_ );
        std::size_t ybin = get_bin( y, yedges_ );

        std::size_t eff_bin = ybin * ( xedges_.size() - 1 ) + xbin;

        double result = 0;
        try {
            result = effs_.at( eff_bin );
        }
        catch( std::out_of_range& e )
        {
            std::cout << e.what() << std::endl;
            std::cout << "pos_x: " << x << "\npos_y: " << y <<"\n" ;
            std::cout << "bin_x: " << xbin << "\nbin_y: " << ybin <<"\n" ;
            std::cout << "min_x: " << xedges_.front() << "\nmax_x: " << xedges_.back()  <<"\n" ;
            std::cout << "min_y: " << yedges_.front() << "\nmax_y: " << yedges_.back()  <<"\n" ;
            throw;
        }
        return result;
    }

    std::size_t Eff2D::get_bin( double val, const std::vector<double>& edges ) const
    {
        auto it = edges.cbegin();
        while (  val >= *it ){ ++it; }
        --it;
        std::size_t bin = std::distance( edges.cbegin() , it );
        return bin;
    }

    //--------------------------------------------------

    //**************************************************
    //MCXY Muon Veto
    //**************************************************

    MCXYMuonVeto::MCXYMuonVeto( const fne::Event * e, 
            const SingleTrack& st, Eff2D  muv_eff)
        :e_( e ), st_( st ), 
        pf_( "input/reco/conditions/run_polarity.dat" ),
        muv_eff_( muv_eff ),
        uni_dist_( 0.0, 1.0 )
    {}

    void MCXYMuonVeto::process_event() const
    {
        //set_log_level( always_print );

        std::size_t muon_pos = find_muon( e_->mc );
        found_muon_ = false;

        //no muon -> no MUV hits
        if ( muon_pos == e_->mc.npart )
        {
            set_muvs( false, false, false );
            return;
        }

        //find muon impact point
        std::pair<double, double>  impact_point = muv_impact(
                e_->mc, muon_pos, pf_.get_polarity( e_->header.run ) );

        muon_position_.SetXYZ( impact_point.first, impact_point.second, 0 );
        found_muon_ = true;

        BOOST_LOG_SEV( get_log(), log_level() )
            << "pos: " << impact_point.first << "  " << impact_point.second;

        double efficiency = muv_eff_.efficiency( 
                impact_point.first, impact_point.second);

        BOOST_LOG_SEV( get_log(), log_level() )
            << "eff: " << efficiency;

        bool  muv_hit = uni_dist_( generator_ ) < efficiency;

        //We simulate all or nothing
        if ( muv_hit )
        {
            //There was a hit. Was it associated with the track?
            const SingleRecoTrack & srt = st_.get_single_track();

            double track_x = srt.extrapolate_ds( na62const::zMuv2).X() ;
            double track_y = srt.extrapolate_ds( na62const::zMuv1).Y() ;

            double half_width = 12.5;//cm

            if ( fabs(impact_point.first - track_x) < 2*half_width 
                    && fabs( impact_point.second - track_y ) < 2*half_width )
            {
                set_muvs( true, true, false);
            }
            else
            {
                BOOST_LOG_SEV( get_log(), log_level() )
                    << "Failed impact point cut" ;
                set_muvs( false, false, false);
            }
        }
        else
        {
            BOOST_LOG_SEV( get_log(), log_level() )
                << "Failed efficiency cut" ;
            set_muvs( false, false, false);
        }
    }

    TVector3 MCXYMuonVeto::get_muon_position() const
    {
        return muon_position_;
    }

    bool MCXYMuonVeto::found_muon() const
    {
        return found_muon_;
    }

    //--------------------------------------------------

    MuonVeto * get_muon_veto
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            MuonVeto * mv = 0;
            try
            {
                YAML::Node ymv = instruct["inputs"]["muv"];

                if ( !ymv )
                { throw Xcept<MissingNode>( "muv" ); }

                mv = dynamic_cast<MuonVeto*>
                    (rf.get_subscriber( ymv.as<std::string>() ) );

                if ( !mv )
                { throw Xcept<BadCast>( "MUV" );}

            }
            catch ( ... )
            {
                std::cerr << "Trying to get Muon Veto ( " __FILE__ ")\n";
                throw;
            }
            return mv;
        }
}
