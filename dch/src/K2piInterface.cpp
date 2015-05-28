#include "K2piInterface.hh"
#include "K2piEventData.hh"
#include "NA62Constants.hh"
#include "TTree.h"
#include "TList.h"

namespace fn
{
    //K2PI LKR INTERFACE

    const double&  K2piLkrInterface::E1(){ return data_.E1; }
    const double&  K2piLkrInterface::E2(){ return data_.E2; }

    const double&  K2piLkrInterface::posC1_X(){ return data_.X1; }
    const double&  K2piLkrInterface::posC2_X(){ return data_.X2; }

    const double&  K2piLkrInterface::posC1_Y(){ return data_.Y1; }
    const double&  K2piLkrInterface::posC2_Y(){ return data_.Y2; }

    const double&  K2piLkrInterface::pK_X(){ return data_.pkx; }
    const double&  K2piLkrInterface::pK_Y(){ return data_.pky; }
    const double&  K2piLkrInterface::pK_Z(){ return data_.pkz; }

    const double&  K2piLkrInterface::pos0K_X(){ return data_.kx0; }
    const double&  K2piLkrInterface::pos0K_Y(){ return data_.ky0; }

    //--------------------------------------------------

    //WRITEABLE K2PI LKR INTERFACE

    double&  WK2piLkrInterface::E1(){ return data_.E1; }
    double&  WK2piLkrInterface::E2(){ return data_.E2; }

    double&  WK2piLkrInterface::posC1_X(){ return data_.X1; }
    double&  WK2piLkrInterface::posC2_X(){ return data_.X2; }

    double&  WK2piLkrInterface::posC1_Y(){ return data_.Y1; }
    double&  WK2piLkrInterface::posC2_Y(){ return data_.Y2; }

    double&  WK2piLkrInterface::pK_X(){ return data_.pkx; }
    double&  WK2piLkrInterface::pK_Y(){ return data_.pky; }
    double&  WK2piLkrInterface::pK_Z(){ return data_.pkz; }

    double&  WK2piLkrInterface::pos0K_X(){ return data_.kx0; }
    double&  WK2piLkrInterface::pos0K_Y(){ return data_.ky0; }

    //--------------------------------------------------
    //K2PI DCH INTERFACE

    const double& K2piDchInterface::p(){ return data_.p ; }
    const double& K2piDchInterface::dxdz(){ return data_.dxdz ; }
    const double& K2piDchInterface::dydz(){ return data_.dydz; }
    const double& K2piDchInterface::x0(){ return data_.x0 ; }
    const double& K2piDchInterface::y0(){ return data_.y0 ; }

    const double& K2piDchInterface::ds_dxdz(){ return data_.ds_dxdz ; }
    const double& K2piDchInterface::ds_dydz(){ return data_.ds_dydz; }
    const double& K2piDchInterface::ds_x0(){   return data_.ds_x0 ; }
    const double& K2piDchInterface::ds_y0(){   return data_.ds_y0 ; }

    TLorentzVector K2piDchInterface::get_pip_4mom()
    {
        TVector3 mom = TVector3( dxdz(), dydz(), 1 ).Unit() * p();
        double energy = std::hypot( data_.p, na62const::mPi );
        return { mom, energy };
    }

    //--------------------------------------------------
    //K2PI DCH INTERFACE

    double& WK2piDchInterface::p(){ return data_.p ; }
    double& WK2piDchInterface::dxdz(){ return data_.dxdz ; }
    double& WK2piDchInterface::dydz(){ return data_.dydz; }
    double& WK2piDchInterface::x0(){ return data_.x0 ; }
    double& WK2piDchInterface::y0(){ return data_.y0 ; }

    double& WK2piDchInterface::ds_dxdz(){ return data_.ds_dxdz ; }
    double& WK2piDchInterface::ds_dydz(){ return data_.ds_dydz; }
    double& WK2piDchInterface::ds_x0(){   return data_.ds_x0 ; }
    double& WK2piDchInterface::ds_y0(){   return data_.ds_y0 ; }

    TLorentzVector WK2piDchInterface::get_pip_4mom()
    {
        TVector3 mom = TVector3( dxdz(), dydz(), 1 ).Unit() * p();
        double energy = std::hypot( data_.p, na62const::mPi );
        return { mom, energy };
    }


    //--------------------------------------------------

    //K2PI MC INTERFACE

    TLorentzVector& K2piMcInterface::p4k(){ return data_.p4K; }
    TLorentzVector& K2piMcInterface::p4pip(){ return data_.p4pip; }
    TLorentzVector& K2piMcInterface::p4pi0(){ return data_.p4pi0; }
    TLorentzVector& K2piMcInterface::p4g1(){ return data_.p4g1; }
    TLorentzVector& K2piMcInterface::p4g2(){ return data_.p4g2; }
    TVector3& K2piMcInterface::vertex(){ return data_.vertex; }

    //--------------------------------------------------

    template<>
        boost::optional<bool> is_mc<K2piEventData>( TTree * t )
        {
            std::cerr << "Checking for tree mc" << std::endl;

            k2pi_user_info * info  = 0;
            info = static_cast<k2pi_user_info*>
                (t->GetUserInfo()->At(0) );

            std::cerr << info->test_string << " " << info->is_mc << std::endl; 

            return info->is_mc;
        }

    K2PIGStatus::K2PIGStatus( const fn::K2piEventData * e, bool mc )
        :e_( e ), mc_( mc )
    {}

         Long64_t K2PIGStatus::get_run() const
    {
        return e_->run;
    }

    bool K2PIGStatus::is_mc() const
    {
        return mc_;
    }
}
