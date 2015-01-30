#include "muon_selections.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "NA62Constants.hh"


namespace fn
{
    REG_DEF_SUB( MuonReqStatus );

    MuonReqStatus::MuonReqStatus( const fne::Event * e ,
            std::set<int> allowed_status )
        :e_( e ), allowed_status_( allowed_status )
    {}

    bool MuonReqStatus::do_check() const
    {
        const TClonesArray& muons = e_->detector.muons;
        Int_t nmuons = e_->detector.nmuons;

        for ( Int_t imuon = 0 ; imuon != nmuons ; ++ imuon )
        {
            fne::RecoMuon * rm = static_cast<fne::RecoMuon*>
                ( muons.At( imuon ) );

            Int_t status = rm->status;

            if ( allowed_status_.find( status) 
                    != allowed_status_.end() )
            {
                return true;
            }
        }
        return false;
    }

    template<>
        Subscriber * create_subscriber<MuonReqStatus>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event *  event = rf.get_event_ptr();
            std::vector<int> allowed_status_list = 
                get_yaml<std::vector<int>>( instruct, "allowed_status" );

            std::set<int> allowed_status
                ( begin( allowed_status_list ), end( allowed_status_list ) );

            return new MuonReqStatus( event, allowed_status );
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

    REG_DEF_SUB( FakeMuv );

    FakeMuv::FakeMuv( const SingleTrack& st,
                    const fne::Event * e,
            std::vector<double> muv1_effs,
            std::vector<double> muv2_effs,
            std::set<int> allowed_status )
        :st_( st ), e_( e ), muv1_effs_( muv1_effs),muv2_effs_( muv2_effs ),
        allowed_status_( allowed_status ), uni_dist_( 0.0, 1.0 )
    {}


    bool FakeMuv::do_check() const
    {


#define MCMUVMETHOD true

#if defined(MCMUVMETHOD)
        const TClonesArray& tca =  e_->mc.particles;
        int nparticles = e_->mc.npart;

        Track t;

        for ( int ip = 0; ip != nparticles ; ++ip )
        {
            const fne::McParticle* particle =
                static_cast<const fne::McParticle*>( tca[ip]  );

            if ( particle->type == 32 ) // found the muon
            {
                t = Track{ particle->production_vertex, particle->momentum.Vect() };
                //particle->production_vertex.Print();
                //particle->momentum.Print();
                break;
            }

        }
        double yMuv1 = t.extrapolate( na62const::zMuv1 ).Y();
        double xMuv2 = t.extrapolate( na62const::zMuv2 ).X();
#endif

#if defined(DATAMUVMETHOD)
        const SingleRecoTrack & srt = st_.get_single_track();
        double xMuv2 = srt.extrapolate_ds( na62const::zMuv2 ).X();
        double yMuv1 = srt.extrapolate_ds( na62const::zMuv1 ).Y();
#endif

        //extrapolate to muv planes
        int muv1_strip = lround(5.0 + (5.5 * yMuv1 / 140.0) );
        double muv_1_eff = (muv1_strip >0 && muv1_strip < 11) ? muv1_effs_.at(muv1_strip) : 0;


        int muv2_strip = lround(5.5 + (5.5 * xMuv2 / 140.0) );
        double muv_2_eff = (muv2_strip >0 && muv2_strip < 11) ? muv2_effs_.at(muv2_strip) : 0;


//        std::cerr << yMuv1 << " " << xMuv2 << std::endl;
//        std::cerr << muv1_strip << " " << muv2_strip << std::endl;
//        std::cerr << muv_1_eff << " " << muv_2_eff << std::endl;


        //generate status
        bool muv_1_hit = uni_dist_( generator_ ) < muv_1_eff;
        bool muv_2_hit = uni_dist_( generator_ ) < muv_2_eff;

        int status = 0;
        if ( muv_1_hit && muv_2_hit ){ status = 1; }
        else if ( muv_1_hit ){ status = 4; }
        else if ( muv_2_hit ){ status = 3; }

        return ( allowed_status_.find( status) != allowed_status_.end() );
    }

    template<>
        Subscriber * create_subscriber<FakeMuv>
        (YAML::Node& instruct, RecoFactory& rf )
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

            std::vector<int> allowed_status_list = 
                get_yaml<std::vector<int>>( instruct, "allowed_status" );

            std::set<int> allowed_status
                ( begin( allowed_status_list ), end( allowed_status_list ) );

            const fne::Event *  event = rf.get_event_ptr();

            return new FakeMuv( *st, event, muv1_effs, muv2_effs, allowed_status );
        }
}
