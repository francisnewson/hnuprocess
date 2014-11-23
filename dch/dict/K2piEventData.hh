#ifndef K2PIEVENTDATA_HH
#define K2PIEVENTDATA_HH
#include "TLorentzVector.h"
#include "TVector3.h"
#if 0
/*
 *  _  ______        _ ____        _
 * | |/ /___ \ _ __ (_)  _ \  __ _| |_ __ _
 * | ' /  __) | '_ \| | | | |/ _` | __/ _` |
 * | . \ / __/| |_) | | |_| | (_| | || (_| |
 * |_|\_\_____| .__/|_|____/ \__,_|\__\__,_|
 *            |_|
 *
 */
#endif
class TTree;
namespace fn
{
    class K2piLkrData : public TObject
    {
        public:
            //Cluster 1
            Double_t E1;
            Double_t X1;
            Double_t Y1;

            //Cluster 2
            Double_t E2;
            Double_t X2;
            Double_t Y2;

            //Kaon momentum
            Double_t pkx;
            Double_t pky;
            Double_t pkz;

            //Kaon position
            Double_t kx0;
            Double_t ky0;

            virtual ~K2piLkrData(){}
            ClassDef ( K2piLkrData, 2 );
    };

    class K2piDchData : public TObject
    {
        public:
            //Momentum
            Double_t p;
            Double_t dxdz;
            Double_t dydz;

            //Postion
            Double_t x0;
            Double_t y0;

            virtual ~K2piDchData(){}
            ClassDef ( K2piDchData, 2 );
    };

    class K2piMcData : public TObject
    {
        public:
            TLorentzVector p4K;
            TLorentzVector p4pip;
            TLorentzVector p4pi0;
            TLorentzVector p4g1;
            TLorentzVector p4g2;

            TVector3 vertex;

            virtual ~K2piMcData(){}
            ClassDef ( K2piMcData, 2 );
    };

    class K2piEventData : public TObject
    {
        public:
            //Header
            Long64_t run;
            Long64_t burst_time;
            Long64_t compact_number;
            Double_t weight;

            //Track Cluster
            Bool_t found_track_cluster;
            Double_t TCE;
            Double_t TCX;
            Double_t TCY;

            //LKr
            K2piLkrData raw_lkr;
            K2piLkrData fit_lkr;
            K2piLkrData fit_lkr_err;
            Double_t lkr_fit_chi2;

            //DCH
            K2piDchData raw_dch;

            //MC
            K2piMcData mc;

            virtual ~K2piEventData(){}
            ClassDef ( K2piEventData, 2 );
    };

    class k2pi_user_info : public TObject
    {
        public:
            bool is_mc;
            std::string test_string;
            virtual ~k2pi_user_info(){}
            ClassDef ( k2pi_user_info, 1 );
    };

}

#endif
