#include "k2pi_extract.hh"
#include "K2piVars.hh"
#include "K2piReco.hh"
#include "SingleTrack.hh"
#include "KaonTrack.hh"
#include "NA62Constants.hh"
#include "k2pi_functions.hh"
#include "Event.hh"

namespace fn
{
    void k2pi_extract_data( const K2piRecoEvent& k2pirc, 
            const SingleRecoTrack& srt, const KaonTrack& kt,
             K2piData& data_ )
    {
        //Fill reconstructed data variables
        data_.neutral_vertex =  k2pirc.get_vertex();
        data_.charged_vertex = srt.get_vertex();

        data_.track_momentum = srt.get_3mom();
        data_.beam_momentum = kt.get_kaon_3mom();
        data_.beam_point = kt.get_kaon_point();

        data_.pt_dch = data_.track_momentum.Perp
            ( data_.beam_momentum);

        data_.p4pi0_lkr = k2pirc.get_p4pi0();
        data_.p4pip_lkr = k2pirc.get_p4pip();

        TVector3 p3pip_dch = srt.get_3mom();
        double Epip_dch = std::hypot( srt.get_mom() , na62const::mPi );

        data_.p4pip_dch = TLorentzVector( p3pip_dch, Epip_dch );

        ClusterData c1 = k2pirc.get_cluster1();
        ClusterData c2 = k2pirc.get_cluster2();

        data_.E1 = c1.energy;
        data_.E2 = c2.energy;

        data_.pos1 = c1.position;
        data_.pos2 = c2.position;
    }

    void k2pi_extract( 
            bool mc, double weight,
            const fne::Event* event_,
            const K2piRecoEvent& k2pirc,
            const SingleRecoTrack& srt,
            const KaonTrack& kt,
            K2piVars& vars_ )
    {
        k2pi_extract_data( k2pirc, srt, kt, vars_.data );

        if ( mc ) 
        { 
            //Fill mc variables
            k2pi_mc_parts mcparts = extract_k2pi_particles( event_ );

            const fne::Mc& mc = event_->mc;
            vars_.mc.vertex =  mc.decay.decay_vertex;
            vars_.mc.p4pi0 =  mcparts.pi0->momentum;
            vars_.mc.p4pip =  mcparts.pip->momentum;
            vars_.mc.p4k =  mcparts.k->momentum;

            int nphotons = mcparts.photons.size();

            if ( nphotons > 0 )
            {
                vars_.mc.p4g1 = mcparts.photons[0]->momentum;
                if ( nphotons > 1 )
                {
                    vars_.mc.p4g2 = mcparts.photons[1]->momentum;
                }
            }

            vars_.ismc = true;
        }
    }

    //--------------------------------------------------
    
    k2pi_mc_parts extract_k2pi_particles( const fne::Event * e )
    {
        const TClonesArray& tca =  e->mc.particles;
        int nparticles = e->mc.npart;

        k2pi_mc_parts result;

        for ( int ip = 0; ip != nparticles ; ++ip )
        {
            const fne::McParticle* particle = 
                static_cast<const fne::McParticle*>( tca[ip]  );

            if ( particle->type == 16 )
            { result.photons.push_back( particle ); }

            if ( particle->type == 4 )
            { result.pi0 = particle; }

            if ( particle->type == 8 )
            { result.pip = particle; }

            if ( particle->type == 512 )
            { result.k = particle; }

        }

        std::sort( begin( result.photons), end( result.photons),
                [] ( const fne::McParticle * p,
                    const fne::McParticle * q ) 
                {
                return (p->momentum.E() < q->momentum.E() );
                } );

        return result;
    }
}
