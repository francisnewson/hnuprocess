#ifndef K2PIPLOTS_HH
#define K2PIPLOTS_HH
#include "Subscriber.hh"
#include "K2piVars.hh"
#include "HistStore.hh"
#include "FourMomComp.hh"
#include "TH2D.h"
#if 0
/*
 *  _  ______        _ ____  _       _
 * | |/ /___ \ _ __ (_)  _ \| | ___ | |_ ___
 * | ' /  __) | '_ \| | |_) | |/ _ \| __/ __|
 * | . \ / __/| |_) | |  __/| | (_) | |_\__ \
 * |_|\_\_____| .__/|_|_|   |_|\___/ \__|___/
 *            |_|
 *
 */
#endif
namespace fn
{
    class K2piPlots : public Subscriber
    {
        public:
            K2piPlots( const fn::K2piVars * vars, TFile& tf );

            void init_data();
            void init_mc();

            void new_event();

            void process_data();
            void process_mc();

            void end_processing();

        private:
            const fn::K2piVars * vars_;
            TFile& tf_;
            bool found_mc_;

            //Data histograms
            TH1D * hm2pip_lkr_;
            TH1D * hpt_dch_;
            TH1D * hz_lkr_dch_;

            //MC histograms
            TH1D * hdz_neutral_;
            TH2D * hdx1dz_ ;
            TH2D * hdy1dz_ ;

            TH2D * hdxdy_ ;
            TH2D * hdxdy1_ ;
            TH2D * hdxdy2_ ;

            TH2D * hdEEdz_ ;
            TH2D * hdEEvsEE_ ;
            TH1D * hdEE_ ;

            TH2D * hdSepdz_ ;
            TH2D * hdpvsdz_ ;
            //----------------------------------------


            HistStore dths_;
            HistStore mchs_;

            FourMomComp lkr_dch_cmp_;
    };

    //--------------------------------------------------

    //auxillary functions

    TVector3 extrapolate_photon( 
            const TLorentzVector& p, const TVector3& c,
            const TVector3& vertex );

    TVector3 delta_cluster(
            const TLorentzVector& p, const TVector3& c,
            const TVector3& vertex );

    void fill_dx_dy( TH2D * h, 
            const TLorentzVector& p, const TVector3& c,
            const TVector3& vertex );
}
#endif
