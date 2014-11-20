#include "K2piInterface.hh"
#include "K2piEventData.hh"

namespace fn
{
    //K2PI LKR INTERFACE

    double&  K2piLkrInterface::E1(){ return data_.E1; }
    double&  K2piLkrInterface::E2(){ return data_.E2; }

    double&  K2piLkrInterface::posC1_X(){ return data_.X1; }
    double&  K2piLkrInterface::posC2_X(){ return data_.X2; }

    double&  K2piLkrInterface::posC1_Y(){ return data_.Y1; }
    double&  K2piLkrInterface::posC2_Y(){ return data_.Y2; }

    double&  K2piLkrInterface::pK_X(){ return data_.pkx; }
    double&  K2piLkrInterface::pK_Y(){ return data_.pky; }
    double&  K2piLkrInterface::pK_Z(){ return data_.pkz; }

    double&  K2piLkrInterface::pos0K_X(){ return data_.kx0; }
    double&  K2piLkrInterface::pos0K_Y(){ return data_.ky0; }

    //--------------------------------------------------

    //K2PI DCH INTERFACE

    double& K2piDchInterface::p(){ return data_.p ; }
    double& K2piDchInterface::dxdz(){ return data_.dxdz ; }
    double& K2piDchInterface::dydz(){ return data_.dydz; }
    double& K2piDchInterface::x0(){ return data_.x0 ; }
    double& K2piDchInterface::y0(){ return data_.y0 ; }

    //--------------------------------------------------

    //K2PI MC INTERFACE
    
    TLorentzVector& K2piMcInterface::p4k(){ return data_.p4K; }
    TLorentzVector& K2piMcInterface::p4pip(){ return data_.p4pip; }
    TLorentzVector& K2piMcInterface::p4pi0(){ return data_.p4pi0; }
    TLorentzVector& K2piMcInterface::p4g1(){ return data_.p4g1; }
    TLorentzVector& K2piMcInterface::p4g2(){ return data_.p4g2; }
    TVector3& K2piMcInterface::vertex(){ return data_.vertex; }
}
