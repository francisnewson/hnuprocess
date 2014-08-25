#ifndef K2PIVARS_HH
#define K2PIVARS_HH
#include "TVector3.h"
#include "TLorentzVector.h"

#if 0
/*
 *  _  ______        ___     __
 * | |/ /___ \ _ __ (_) \   / /_ _ _ __ ___
 * | ' /  __) | '_ \| |\ \ / / _` | '__/ __|
 * | . \ / __/| |_) | | \ V / (_| | |  \__ \
 * |_|\_\_____| .__/|_|  \_/ \__,_|_|  |___/
 *            |_|
 *
 */
#endif
namespace fn
{
    class K2piData : public TObject
    {
        public:
        TVector3 neutral_vertex;
        TLorentzVector p4pi0;
        TLorentzVector p4pip;

        double E1;
        double E2;

        TVector3 pos1;
        TVector3 pos2;

        virtual ~K2piData(){};
        ClassDef( K2piData, 1 );
    };

    class K2piMC : public TObject
    {
        public:
        TVector3 vertex;

        TLorentzVector p4pi0;
        TLorentzVector p4pip;
        TLorentzVector p4k;
        TLorentzVector p4g1;
        TLorentzVector p4g2;

        virtual ~K2piMC(){};
        ClassDef( K2piMC, 1 );
    };

    class K2piVars : public TObject
    {
        public:
            K2piData data;
            K2piMC mc;
            double chi2;

        virtual ~K2piVars(){};

        ClassDef( K2piVars, 1 );
    };
}
#endif
