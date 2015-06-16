#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "Xcept.hh"
#include "tracking_selections.hh"
#include "muon_functions.hh"
#include "NA62Constants.hh"
#include "MuRec.hh"

namespace fn
{
    REG_DEF_SUB( SingleMuon );

    template<>
        Subscriber * create_subscriber<SingleMuon>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            std::string method = get_yaml<std::string>( instruct, "method" );
            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack * st = get_single_track( instruct, rf );

            bool is_mc = rf.is_mc();

            if ( method == "raw" )
            {
                return new RawSingleMuon( event, *st );
            }
            else if ( method == "mcscatter" )
            {
                return new MCScatterSingleMuon( event, *st );
            }
            else if ( method == "datamurec" )
            {
                double multiplier = get_yaml<double>( instruct, "multiplier" );
                return new DataMuRec( event, *st, multiplier );
            }
            else if ( method == "auto" )
            {
                if ( is_mc )
                {
                    return new MCScatterSingleMuon( event, *st );
                }
                else if ( rf.is_halo() )
                {
                    return new HaloScatterSingleMuon( event, *st );
                }
                else
                {
                    return new RawSingleMuon( event, *st );
                }
            }
            else
            {
                throw Xcept<UnknownSingleMuonMethod>( method );
            }
        }

    SingleMuon * get_single_muon( YAML::Node& instruct, RecoFactory& rf )
    {
        YAML::Node muon_node = instruct["inputs"]["sm"];

        if ( ! muon_node )
        { throw Xcept<MissingNode>( "sm" ); }

        SingleMuon * sm  = dynamic_cast<SingleMuon*>
            ( rf.get_subscriber( muon_node.as<std::string>() ) );

        if ( !sm )
        { throw Xcept<BadCast>( "SM" ); }

        return sm;
    }

    //--------------------------------------------------

    RawSingleMuon::RawSingleMuon( const fne::Event * e, const SingleTrack& st)
        :e_( e ), st_( st )
    {}

    void RawSingleMuon::new_event()
    {
        dirty_ = true;
    }

    void RawSingleMuon::update() const
    {
        if (!dirty_ ){ return ; }

        //lookt at track info
        const auto & srt = st_.get_single_track();
        int muon_id = srt.get_muon_id();
        int track_id = srt.get_compact_id();

        //negative id means no muon found
        if ( muon_id < 0 )
        {
            found_muon_ = false;
            return;
        }

        rm_ = static_cast<fne::RecoMuon*>( e_->detector.muons.At( muon_id ) );

        //the muon must have status 1 or 2
        if ( !( rm_->status == 1 || rm_->status == 2 ) )
        {
            found_muon_ = false;
            return;
        }

        //save muon
        found_muon_ = true;
        rm_ = static_cast<fne::RecoMuon*>( e_->detector.muons.At( muon_id ) );

        dirty_ = false;

        //check consistency
        int n_matching_muons = 0;
        unsigned int nmuons = e_->detector.nmuons;
        for ( int imuon = 0 ; imuon != nmuons ; ++imuon )
        {
            const auto * muon = static_cast<const fne::RecoMuon*>(
                    e_->detector.muons.At( imuon ) );

            if ( muon->iTrk ==  track_id )
            {
                ++n_matching_muons;
            }
        }

        if ( n_matching_muons> 1 )
        {
            BOOST_LOG_SEV( get_log(), always_print )
                << "More than one matching muon!" ;
        }
    }

    bool RawSingleMuon::found() const { update(); return found_muon_; }
    double RawSingleMuon::weight() const { update(); return 1; }
    double RawSingleMuon::x() const { update(); return rm_->x; }
    double RawSingleMuon::y() const { update(); return rm_->y; }

    //--------------------------------------------------

    ScatterSingleMuon::ScatterSingleMuon( const fne::Event * e, const SingleTrack& st)
        :e_( e ), st_( st ), pf_( "input/reco/conditions/run_polarity.dat" )
    {}

    void ScatterSingleMuon::new_event()      { dirty_ = true; }
    bool ScatterSingleMuon::found()    const { update(); return found_muon_; }
    double ScatterSingleMuon::weight() const { update(); return 1.0; }
    double ScatterSingleMuon::x()      const { update(); return x_; }
    double ScatterSingleMuon::y()      const { update(); return y_; }

    void ScatterSingleMuon::new_run()
    {
        BOOST_LOG_SEV( get_log(), always_print)
            << "New run " << e_->header.run
            << ". Setting magnet polarity to " << pf_.get_polarity( e_->header.run);

        mss_.set_magnet_polarity( pf_.get_polarity( e_->header.run ) );
    }

    void ScatterSingleMuon::update() const
    {
        if (!dirty_ ){ return ; }

        boost::optional<toymc::track_params> muon_track = get_muon_track( e_, st_ );

        if ( !muon_track )
        { 
            found_muon_ = false;
            dirty_ = false;
            return;
        }

        std::pair<double,double> muon_hit = get_muon_position( *muon_track );
        x_ = muon_hit.first;
        y_ = muon_hit.second;

        found_muon_ = true;
        dirty_ = false;
    }

    std::pair<double,double> ScatterSingleMuon::get_muon_position( toymc::track_params tp) const
    {
        //Propagate MC muon
        std::pair<double, double> muv_hit_xy = mss_.transfer( tp );

        x_ = muv_hit_xy.first; // - fmod( muv_hit_xy.first  , na62const::muv_half_width);
        y_ = muv_hit_xy.second; //- fmod( muv_hit_xy.second , na62const::muv_half_width);

        //check muon has actually hit a strip


        return std::make_pair( x_, y_ );
    }

    //--------------------------------------------------

    MCScatterSingleMuon::MCScatterSingleMuon( const fne::Event * e, const SingleTrack& st)
        :ScatterSingleMuon(  e, st  ) {}

    boost::optional<toymc::track_params> 
        MCScatterSingleMuon::get_muon_track( const fne::Event * e, const SingleTrack& st ) const
        {
            //set_log_level( always_print );

            boost::optional<toymc::track_params> result = boost::none;

            //Extract MC muon
            auto muon_id = find_muon( e->mc );

            BOOST_LOG_SEV( get_log(), log_level() ) << "Muon id: " << muon_id ;

            //no muon found
            if ( muon_id == e->mc.npart ) { return result; }

            fne::McParticle * rm = 
                static_cast<fne::McParticle*>( e->mc.particles.At( muon_id ) );

            //check production and decay points
            const TVector3& prod = rm->production_vertex;
            const TLorentzVector& four_mom = rm->momentum;
            double prod_z = prod.Z();
            double decay_z = rm->decay_vertex.Z();

            //No decay is indicated by COmPact as some negative value
            if ( decay_z < prod_z )
            {
                decay_z = 100000;
            }

            BOOST_LOG_SEV( get_log(), log_level() )
                << "Track: " << prod_z << " - " << decay_z ;


            if ( prod_z <  (na62const::zLkr - na62const::len_lkr) &&  decay_z > na62const::zMuv2 )
            {
                result = toymc::track_params{ four_mom.P(), +1, 
                    prod.X(), four_mom.X() / four_mom.Z(),
                    prod.Y(), four_mom.Y() / four_mom.Z(),
                    prod.Z() } ;
            }

            return result;
        }

    //--------------------------------------------------

    HaloScatterSingleMuon::HaloScatterSingleMuon
        ( const fne::Event * e, const SingleTrack& st)
        :ScatterSingleMuon( e, st ){}

    boost::optional<toymc::track_params> 
        HaloScatterSingleMuon::get_muon_track( const fne::Event * e, const SingleTrack& st ) const
        {
            boost::optional<toymc::track_params> result = boost::none;
            const SingleRecoTrack& srt = st.get_single_track();
            double momentum = srt.get_mom();
            TVector3 point = srt.extrapolate_ds( na62const::zDch3 );
            TVector3 dir = srt.get_ds_mom();

            result = toymc::track_params{ momentum, +1, 
                point.X(), dir.X() / dir.Z(),
                point.Y(), dir.Y() / dir.Z(),
                point.Z() } ;

            return result;
        }

    //--------------------------------------------------
}
