#include "K2piReco.hh"
#include <cassert>
#include "Xcept.hh"
#include "SingleTrack.hh"
#include "tracking_selections.hh"
#include "cluster_selections.hh"

namespace fn
{

    void K2piReco::new_event()
    {
        dirty_ = true;
    }

    K2piReco::K2piReco(  K2piRecoEvent* k2pirc )
        :reco_event_( k2pirc )
    {}

    //--------------------------------------------------

    REG_DEF_SUB( K2piReco );

    const K2piRecoEvent& K2piReco::get_reco_event()
    {
        if ( dirty_ )
        {
            process_event();
            dirty_ = false;
        }

        assert(!dirty_);
        return *reco_event_;
    }

    template<>
        Subscriber * create_subscriber<K2piReco>
        (YAML::Node& instruct, RecoFactory& rf )
        {
            const fne::Event * event = rf.get_event_ptr();
            const SingleTrack* st = get_single_track( instruct, rf );
            const K2piClusters* k2pic = get_k2pi_clusters( instruct, rf );
            std::string method = instruct["method"].as<std::string>();
            bool mc = rf.is_mc();

            K2piRecoEvent * re;

            if ( method == "simple" )
            {
                re = new K2piRecoImpEvent;
            }
            else 
            {
                throw Xcept<UnknownK2piRecoMethod>( method );
            }

            return new K2piRecoImp( 
                    event, *st, *k2pic, re, mc );
        }

    //--------------------------------------------------

    K2piRecoImp::K2piRecoImp( const fne::Event * event,
            const SingleTrack& st,
            const K2piClusters& k2pic,
            K2piRecoEvent* k2pirec,
            bool mc )
        :K2piReco( k2pirec),
        e_( event), st_( st), k2pic_( k2pic), mc_( mc)
    {}

    void K2piRecoImp::process_event() const
    {
        reco_event_->update( e_, st_, k2pic_ );
    }

    //--------------------------------------------------
    
             void K2piRecoImpEvent::update( 
                    const fne::Event * event,
                    const SingleTrack& st,
                    const K2piClusters& k2pic )
             {

             }
}
