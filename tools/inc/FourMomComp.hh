#ifndef FOURMOMCOMP_HH
#define FOURMOMCOMP_HH

#include "HistStore.hh"
#include "TLorentzVector.h"
#include <boost/filesystem/path.hpp>

namespace fn
{
    class FourMomComp
    {
        public:
            FourMomComp();
            void Fill( const TLorentzVector& a,
                    const TLorentzVector& b , double wgt );

            void FillM2( const TLorentzVector& a, 
                    const TLorentzVector& b ,
                    const TLorentzVector& ref,  double wgt );

            void Write();

            void SetColor( Color_t color );

        private:
            TH1D * hdp_;
            TH1D * hdpop2_;
            TH1D * hdtx_;
            TH1D * hdty_;
            TH2D * hdpvsp_;
            TH1D * hdm2_;

            //Output properties
            std::string name_;
            HistStore hs_;

    };
}

#endif
