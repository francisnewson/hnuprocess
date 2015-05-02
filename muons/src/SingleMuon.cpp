#include "SingleMuon.hh"
#include "SingleTrack.hh"
#include "Event.hh"
#include "RecoMuon.hh"
#include "yaml_help.hh"
#include "Xcept.hh"
#include "tracking_selections.hh"
#include "muon_functions.hh"
#include "NA62Constants.hh"

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
            else if ( method == "auto" )
            {
                if ( is_mc )
                {
                    return new MCScatterSingleMuon( event, *st );
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

    MCScatterSingleMuon::MCScatterSingleMuon( const fne::Event * e, const SingleTrack& st)
        :e_( e ), st_( st ), pf_( "input/reco/conditions/run_polarity.dat" )
    {}

    void MCScatterSingleMuon::new_event()
    {
        dirty_ = true;
    }

    void MCScatterSingleMuon::new_run()
    {
        mss_.set_magnet_polarity( pf_.get_polarity( e_->header.run ) );
    }

    void MCScatterSingleMuon::update() const
    {
        if (!dirty_ ){ return ; }

        //Extract MC muon
        auto muon_id = find_muon( e_->mc );


        if ( muon_id == e_->mc.npart )
        {
            found_muon_  = false;
            dirty_ = false;
            return;
        }

        fne::McParticle * rm = 
            static_cast<fne::McParticle*>( e_->mc.particles.At( muon_id ) );

        //set_log_level( always_print );

        BOOST_LOG_SEV( get_log(), log_level() )
            << "Muon id: " << muon_id ;

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
            //Propagate MC muon
            std::pair<double, double> muv_hit_xy = mss_.transfer( 
                    toymc::track_params{ four_mom.P(), +1, 
                    prod.X(), four_mom.X() / four_mom.Z(),
                    prod.Y(), four_mom.Y() / four_mom.Z(),
                    prod.Z() } );

            found_muon_ = true;
            x_ = muv_hit_xy.first  - fmod( muv_hit_xy.first  , na62const::muv_half_width);
            y_ = muv_hit_xy.second - fmod( muv_hit_xy.second , na62const::muv_half_width);
        }
        else
        {
            found_muon_  = false;
        }
    }

    bool MCScatterSingleMuon::found()    const{ update(); return found_muon_; }
    double MCScatterSingleMuon::weight() const{ update(); return 1.0; }
    double MCScatterSingleMuon::x()      const{ update(); return x_; }
    double MCScatterSingleMuon::y()      const{ update(); return y_; }
}
