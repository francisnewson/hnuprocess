#ifndef K2PIINTERFACE_HH
#define K2PIINTERFACE_HH
#include "TLorentzVector.h"
#include "K2piEventData.hh"
#include "EHelp.hh"
#if 0
/*
 *  _  ______        _ ___       _             __
 * | |/ /___ \ _ __ (_)_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
 * | ' /  __) | '_ \| || || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
 * | . \ / __/| |_) | || || | | | ||  __/ |  |  _| (_| | (_|  __/
 * |_|\_\_____| .__/|_|___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
 *            |_|
 *
 */
#endif
class TLorentzVector;
class TVector3;

namespace fn
{
    struct K2piLkrInterface
    {
        public:
            K2piLkrInterface( K2piLkrData& data )
                :data_( data ){}

            double&  E1();
            double&  E2();

            double&  posC1_X();
            double&  posC2_X();

            double&  posC1_Y();
            double&  posC2_Y();

            double&  pK_X();
            double&  pK_Y();
            double&  pK_Z();

            double&  pos0K_X();
            double&  pos0K_Y();

        private:
            K2piLkrData& data_;
    };

    class K2piDchData;

    struct K2piDchInterface
    {
        public:
            K2piDchInterface( K2piDchData& data )
                :data_( data ){}

            double& p();
            double& dxdz();
            double& dydz();
            double& x0();
            double& y0();

            TLorentzVector get_pip_4mom();

        private:
            K2piDchData& data_;
    };

    class K2piMcData;

    struct K2piMcInterface
    {
        public:
            K2piMcInterface( K2piMcData& data )
                :data_( data ){}

            TLorentzVector& p4k();
            TLorentzVector& p4pip();
            TLorentzVector& p4pi0();
            TLorentzVector& p4g1();
            TLorentzVector& p4g2();

            TVector3& vertex();

        private:
            K2piMcData& data_;
    };

    template< class L, class R>
        void copy( L& lhs, R& rhs )
        {
            rhs.E1() = lhs.E1();      
            rhs.E2() = lhs.E2();

            rhs.posC1_X() = lhs.posC1_X();
            rhs.posC2_X() = lhs.posC2_X();

            rhs.posC1_Y() = lhs.posC1_Y();
            rhs.posC2_Y() = lhs.posC2_Y();

            rhs.pK_X() = lhs.pK_X();
            rhs.pK_Y() = lhs.pK_Y();
            rhs.pK_Z() = lhs.pK_Z();

            rhs.pos0K_X() = lhs.pos0K_X();
            rhs.pos0K_Y() = lhs.pos0K_Y();
        }


    template<>
        boost::optional<bool> is_mc<K2piEventData>( TTree * T );
}
#endif
