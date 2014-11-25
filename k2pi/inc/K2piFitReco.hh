#ifndef K2PIFITRECO
#define K2PIFITRECO
#include "K2piReco.hh"
#include "Math/Minimizer.h"

namespace fn
{
    struct k2pi_params
    {
        std::array<double, 11> par_;

        double&  E1()  { return par_[0] ;} 
        double&  E2()  { return par_[1] ;}

        double&  posC1_X()  { return par_[2] ;}
        double&  posC2_X()  { return par_[3] ;}

        double&  posC1_Y()  { return par_[4] ;}
        double&  posC2_Y()  { return par_[5] ;}

        double&  pK_X()  { return par_[6] ;}
        double&  pK_Y()  { return par_[7] ;}
        double&  pK_Z()  { return par_[8] ;}

        double&  pos0K_X()  { return par_[9] ;}
        double&  pos0K_Y()  { return par_[10];}

    };

    struct k2pi_fit
    {
        TLorentzVector pi0;
        TLorentzVector p1;
        TLorentzVector p2;
        TLorentzVector pip;
        TVector3 neutral_vertex;
        double chi2;
    };

    class k2pi_fitter
    {
        public:
            k2pi_fitter (
                    const fne::Event * event,
                    const KaonTrack& kt,
                    const SingleRecoTrack& srt,
                    const K2piRecoClusters& k2pirc,
                    bool mc);

            void init();

            void set_variables(  ROOT::Math::Minimizer * minimizer );
            double operator()( const double * fit_params );
            const k2pi_fit& result() { return result_ ;}


        private:
            double chi2( double pion_mass );
            const fne::Event * e_;
            const KaonTrack& kt_;
            const SingleRecoTrack& srt_;
            const K2piRecoClusters& k2pirc_ ;
            bool mc_;

            k2pi_params measured_;
            k2pi_params fit_;
            k2pi_params errors_;
            std::vector<std::string> names_;

            k2pi_fit result_;
    };

    //--------------------------------------------------

    class K2piFitRecoEvent : public K2piRecoEvent
    {
        public:

            K2piFitRecoEvent();
            void update( 
                    const fne::Event * event,
                    const KaonTrack& kt,
                    const SingleTrack& st,
                    const K2piClusters& k2pic, bool mc );

            TLorentzVector get_p4pip() const;
            TLorentzVector get_p4pi0() const;

            TLorentzVector get_p4g1() const;
            TLorentzVector get_p4g2() const;

            const ClusterData get_cluster1()  const;
            const ClusterData get_cluster2()  const;

            const ClusterData get_track_cluster() const;
            bool found_track_cluster() const;

            double get_chi2() const;

        private: 
            double get_zvertex() const;
            TVector3 get_vertex() const;
            double get_m2pip() const;
            double get_m2pi0() const;

            ClusterData c1_;
            ClusterData c2_;
            ClusterData tc_;

            bool found_track_cluster_;

            k2pi_fit fit_result_;
    };
}
#endif
