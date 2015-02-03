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

namespace fn
{

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

    void MuonVeto::set_muvs(  bool muv1, bool muv2, bool muv3 ) const
    {
        muv1_ = muv1;
        muv2_ = muv2;
        muv3_ = muv3;
        status_ = get_muon_status( muv1, muv2, muv3 );
    }

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

    template<>
        Subscriber * create_subscriber<MuonVeto>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = instruct["method"].as<std::string>();
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
            else
            {
                throw Xcept<UnknownMuonVetoMethod>( method );
            }

        }

    //--------------------------------------------------

    DataMuonVeto::DataMuonVeto( const fne::Event * e )
        :e_( e ){}


    void DataMuonVeto::process_event() const
    {
        const TClonesArray& muons = e_->detector.muons;
        Int_t nmuons = e_->detector.nmuons;

        bool muv1_ = false;
        bool muv2_ = false;
        bool muv3_ = false;

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

            Int_t status = rm->status;

            if ( status != 3 ){ muv1_ = true; }
            if ( status != 4 ){ muv2_ = true; }
            if ( status != 2 ){ muv3_ = true; }
        }

        //overall status
        set_muvs( muv1_, muv2_, muv3_ );
    }

    //--------------------------------------------------

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

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

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

    //--------------------------------------------------
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
        const SingleRecoTrack & srt = st_.get_single_track();
        double xMuv2 = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double yMuv1 = srt.extrapolate_ds( na62const::zMuv1 ).Y();

        int muv1_strip = lround(5.0 + (5.5 * yMuv1 / 140.0) );
        double muv_1_eff = (muv1_strip >0 && muv1_strip < 11) ? muv1_effs_.at(muv1_strip) : 0;

        int muv2_strip = lround(5.5 + (5.5 * xMuv2 / 140.0) );
        double muv_2_eff = (muv2_strip >0 && muv2_strip < 11) ? muv2_effs_.at(muv2_strip) : 0;

        //generate status
        bool  muv1_ = uni_dist_( generator_ ) < muv_1_eff;
        bool  muv2_ = uni_dist_( generator_ ) < muv_2_eff;
        bool  muv3_ = true;

        set_muvs( muv1_, muv2_, muv3_);
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
