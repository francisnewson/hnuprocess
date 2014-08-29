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
            void Write();

        private:
            TH1D * hdp_;
            TH1D * hdpop2_;
            TH1D * hdtx_;
            TH1D * hdty_;
            TH2D * hdpvsp_;

            //Output properties
            std::string name_;
            HistStore hs_;

    };
}

#endif
