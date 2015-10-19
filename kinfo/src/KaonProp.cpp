#include "KaonProp.hh"
#include "RecoTrack.hh"
#include "root_help.hh"
#include "RecoFactory.hh"
#include "yaml_help.hh"

namespace fn
{
    void fill_pos_burst( const fne::Event * e , boost::optional<PolarityFinder*>& pf, kaon_burst& kb )
    {
        const auto&  h   = e->header;
        const auto&  c   = e->conditions;
        kb.run           = h.run;
        kb.burst_time    = h.burst_time;
        kb.dxdz          = c.pkdxdzp;
        kb.dydz          = c.pkdydzp;
        kb.xoff          = c.pkxoffp;
        kb.yoff          = c.pkyoffp;
        kb.pmag          = c.pkp;
        kb.alpha         = c.alpha;
        kb.beta          = c.beta;
        kb.pol           =  pf ? (*pf)->get_polarity(h.run) : 0;
        kb.count         = 0;
    }

    void fill_neg_burst( const fne::Event * e , boost::optional<PolarityFinder*>& pf, kaon_burst& kb )
    {
        const auto&  h   = e->header;
        const auto&  c   = e->conditions;
        kb.run           = h.run;
        kb.burst_time    = h.burst_time;
        kb.dxdz          = c.pkdxdzm;
        kb.dydz          = c.pkdydzm;
        kb.xoff          = c.pkxoffm;
        kb.yoff          = c.pkyoffm;
        kb.pmag          = c.pkm;
        kb.alpha         = c.alpha;
        kb.beta          = c.beta;
        kb.pol           =  pf ? (*pf)->get_polarity(h.run) : 0;
        kb.count         = 0;
    }

    bool operator==( const kaon_burst& lhs, const kaon_burst& rhs )
    {
        return 
            lhs.run        == rhs.run &&
            lhs.burst_time == rhs.burst_time &&
            lhs.dxdz       == rhs.dxdz &&
            lhs.dydz       == rhs.dydz &&
            lhs.xoff       == rhs.xoff &&
            lhs.yoff       == rhs.yoff &&
            lhs.pmag       == rhs.pmag &&
            lhs.alpha      == rhs.alpha &&
            lhs.beta       == rhs.beta ;
    }

    //--------------------------------------------------

    REG_DEF_SUB( KaonProp );

    KaonProp::KaonProp( const Selection& sel, const fne::Event * e, TFile& tf, std::string folder )
        :Analysis( sel ), e_( e ), tf_( tf ), folder_( folder ),
        hposkick_( "hposkick", "Pos kick", 10000, -500, 500 ),
        hnegkick_( "hnegkick", "Neg kick", 10000, -500, 500 )
    {
        kplus_tree_.reset( new TTree( "KPLUSTREE", "KPLUS BURSTS" ) );
        kplus_tree_->Branch( "kaon_burst", &kplus_burst_, 
                "run/L:burst_time/L:count/L:dxdz/D:dydz/D:xoff/D:yoff/D:pmag/D:alpha/D:beta/D:pol/I" );

        kminus_tree_.reset( new TTree( "KMINUSTREE", "KMINUS BURSTS" ) );
        kminus_tree_->Branch( "kaon_burst", &kminus_burst_, 
                "run/L:burst_time/L:count/L:dxdz/D:dydz/D:xoff/D:yoff/D:pmag/D:alpha/D:beta/D:pol/I" );

        mpn33_tree_.reset( new TTree( "MPN33TREE", "MPN33 BURSTS" ) );
        mpn33_tree_->Branch( "mpn33_burst", &mpn33_burst_,
                "run/L:burst_time/L:poskick/D:negkick/D" );

        pol_finder_ = new PolarityFinder( "input/reco/conditions/run_polarity.dat" );
    }

    void KaonProp::set_pos_sel( Selection* sel ){ selpos_ = sel; }
    void KaonProp::set_neg_sel( Selection* sel ){ selneg_ = sel; }

    void KaonProp::new_burst()
    {
        const auto&  h = e_->header;

        fill_pos_burst( e_, pol_finder_, kplus_burst_ );
        fill_neg_burst( e_, pol_finder_, kminus_burst_ );


        hposkick_.Reset();
        hnegkick_.Reset();

        mpn33_burst_.run = h.run;
        mpn33_burst_.burst_time = h.burst_time;
    }

    void KaonProp::process_event()
    {
        fill_pos_burst( e_, pol_finder_, test_kplus_burst_ );
        fill_neg_burst( e_, pol_finder_,  test_kminus_burst_ );

        assert( kplus_burst_ == test_kplus_burst_ );
        assert( kminus_burst_ == test_kminus_burst_ );


        if ( selpos_ )
        {
            if ( (*selpos_)->check() ) { ++kplus_burst_.count; }
        }
        else
        {
            ++kplus_burst_.count;
        }

        if ( selneg_ )
        {
            if ( (*selneg_)->check() ) { ++kminus_burst_.count; }
        }
        else
        {
            ++kminus_burst_.count;
        }

        
        const auto& tracks = e_->detector.tracks;
        unsigned int ntracks = e_->detector.ntracks;

        for ( unsigned int itrack = 0 ; itrack != ntracks ; ++itrack )
        {
            const auto * rt = static_cast<fne::RecoTrack*>( tracks.At( itrack ) );
            double xkick = (rt->dxdz - rt->bdxdz )* rt->p;
            double q = rt->q;

            if ( q > 0 )
            {
                hposkick_.Fill( xkick );
            }
            else if (q < 0 )
            {
                hnegkick_.Fill( xkick );
            }
            else
            {
                throw std::runtime_error( "Neutral track in KaonProp" );
            }
        }
    }

    void KaonProp::end_burst()
    {
        BOOST_LOG_SEV( get_log(), always_print ) 
            << "Counts " <<  kplus_burst_.count << " " << kminus_burst_.count;

        kplus_tree_->Fill();
        kminus_tree_->Fill();


        mpn33_burst_.poskick = median( &hposkick_ );
        mpn33_burst_.negkick = median( &hnegkick_ );
        mpn33_tree_->Fill();
    }

    void KaonProp::end_processing()
    {
        end_burst();
        cd_p( &tf_, folder_ );
        kplus_tree_->Scan("*");
        kplus_tree_->Write();
        kminus_tree_->Write();
        mpn33_tree_->Write();
    }

    template<>
        Subscriber * create_subscriber<KaonProp>
        ( YAML::Node& instruct, RecoFactory& rf )
        {
            const Selection * sel  = rf.get_selection(
                    get_yaml<std::string>( instruct, "selection" ) );

            TFile & tfile = rf.get_tfile( 
                    get_yaml<std::string>( instruct, "tfile" ) );

            std::string folder = get_folder( instruct, rf );

            const fne::Event * e = rf.get_event_ptr();

            auto * result  = new KaonProp( *sel,  e, tfile, folder );

            return result;
        }
}
