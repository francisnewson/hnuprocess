#include <math.h>
#include "cmpio.h"
#include "cfortran.h"
#include "hbook.h"
#include "constants.h"
#include "user.h"

/*** particle masses ***/
#define MKCH    0.493677
#define MPI0    0.1349764
#define MPI     0.13957
#define MEL     0.000510998902
#define MMU     0.105658357
#define SQMKCH  (MKCH*MKCH)
#define SQMPI   0.0194797849
#define SQMPI0  0.01821862855

#define ALPHA_EM	(7.297e-3)

#define SQR(x)  (x)*(x)

#define ZCOL		(-1800.00000)
#define ZBT              9700.540039
#define ZDCH1            9708.599609
#define ZDCH2           10627.099609
#define ZMAG            10896.799805
#define ZDCH3           11166.099609
#define ZT              11880.179688
#define ZDCH4           11887.400391
#define ZHODO           11946.500000
#define ZLKR            12108.120000

extern double buffer[100];
extern int ibuffer[100];

int LKr_acc (int nrun, float pos_x, float pos_y, float par);
void calc_cm_mom_my(double p0[4], double p_lab[4], double p_cm[4]);
void dumpsmc(superMcEvent *mcevt);

void gloren(double beta[4],double pa[4],double pb[4]) {
/*
    ******************************************************************
    *                                                                *
    *       Routine to transform momentum and energy from the        *
    *       Lorentz frame A to the Lorentz frame B                   *
    *                                                                *
    *       PA(1)                                                    *
    *       PA(2)     Momentum components in frame A                 *
    *       PA(3)                                                    *
    *       PA(4)     Energy                                         *
    *       PB(..)   same quantities in frame B                      *
    *                                                                *
    *       BETA(1)    Components of velocity of frame B             *
    *       BETA(2)        as seen from frame A                      *
    *       BETA(3)                                                  *
    *       BETA(4)    1./SQRT(1.-BETA**2)                           *
    *                                                                *
    ******************************************************************
*/
  
  double betpa  = beta[1]*pa[1] + beta[2]*pa[2] + beta[3]*pa[3];
  double bpgam  = (betpa * beta[0]/(beta[0] + 1.0) - pa[0]) * beta[0];
  pb[1] = pa[1] + bpgam * beta[1];
  pb[2] = pa[2] + bpgam * beta[2];
  pb[3] = pa[3] + bpgam * beta[3];
  pb[0] = (pa[0] - betpa) * beta[0];
  return;
}


int user_superMcEvent(superBurst *sbur,superMcEvent *mcsevt) {
  char s[256];
  int i,j;

  superCmpEvent *sevt = &mcsevt->scmpevt;

  /* Ke2g / Kmu2g kinematics */
  double eelmax = (SQMKCH+MMU*MMU)/(2.0*MKCH);
  double egamma =
    (mcsevt->Npart>2 && mcsevt->part[2].type==16) ?
    mcsevt->part[0].pvertex[0] : 0.0;
  double eel    =
    (mcsevt->Npart>2 && mcsevt->part[2].type==16) ?
    mcsevt->part[0].pvertex[1] : eelmax;

  double x = 2.0*egamma/MKCH;
  double y = 2.0*eel/MKCH;
  HF2 (11111, x, y, 1.);

  /*
  printf ("@@@ %f %f\n", egamma, eel);
  return 0;
  */

  /* special treatment of Km2g (IB) ... @@@ */
  /*
  if (x>0.1 && y < 1.0-x + MMU*MMU/MKCH/MKCH/(1.0-x)) {
    //printf ("@@@ %f %f\n",x,y);
    //dumpsmc(mcsevt);
    return 0;
  }
  */

  /** straw prototype tests **/

  /*
  double zproto = 2110.0;
  if (mcsevt->part[1].pvertex[2]<zproto) {
    double xproto =
      mcsevt->part[1].pvertex[0] +
      mcsevt->part[1].p[1]/mcsevt->part[1].p[3]*
      (zproto-mcsevt->part[1].pvertex[2]);
    double yproto =
      mcsevt->part[1].pvertex[1] +
      mcsevt->part[1].p[2]/mcsevt->part[1].p[3]*
      (zproto-mcsevt->part[1].pvertex[2]);
    double xdch1 =
      mcsevt->part[1].pvertex[0] +
      mcsevt->part[1].p[1]/mcsevt->part[1].p[3]*
      (ZDCH1-mcsevt->part[1].pvertex[2]);
    double ydch1 =
      mcsevt->part[1].pvertex[1] +
      mcsevt->part[1].p[2]/mcsevt->part[1].p[3]*
      (ZDCH1-mcsevt->part[1].pvertex[2]);
    printf ("@proto %f %f %f %f %f %f\n",
	    mcsevt->part[1].p[0],
	    xproto, yproto,
	    xdch1, ydch1,
	    0.01*mcsevt->part[1].pvertex[2]);
  }
  */


  /* MC weighting */
  double pkgen  = sqrt(SQR(mcsevt->part[0].p[1])+
		       SQR(mcsevt->part[0].p[2])+
		       SQR(mcsevt->part[0].p[3]));

  double pmugen = sqrt(SQR(mcsevt->part[1].p[1])+
		       SQR(mcsevt->part[1].p[2])+
		       SQR(mcsevt->part[1].p[3]));

  double p2gen = sqrt(SQR(mcsevt->part[2].p[1])+
		      SQR(mcsevt->part[2].p[2])+
		      SQR(mcsevt->part[2].p[3]));

  double dxdzgen = mcsevt->part[0].p[1] / mcsevt->part[0].p[3];
  double dydzgen = mcsevt->part[0].p[2] / mcsevt->part[0].p[3];

  buffer[8]  = pkgen;
  buffer[31] = dxdzgen;
  buffer[32] = dydzgen;

  /* 4 Feb 2010 "crude" quadratic weights */
  double slope = 0.0;
  if (sbur->nrun>=20154 && sbur->nrun<=20256) slope = -0.05;
  if (sbur->nrun>=20268 && sbur->nrun<=20303) slope = -0.08;
  if (sbur->nrun>=20304 && sbur->nrun<=20324) slope = -0.02;
  if (sbur->nrun>=20332 && sbur->nrun<=20371) slope = +0.08;
  if (sbur->nrun>=20387 && sbur->nrun<=20486) slope = +0.07;
  if (sbur->nrun>=20487 && sbur->nrun<=20521) slope = +0.06;
  if (sbur->nrun>=20522 && sbur->nrun<=20612) slope = +0.01;
  if (sbur->nrun>=20613 && sbur->nrun<=20695) slope = +0.10;

  double wt_pk = 1 + slope*SQR(pkgen-74.0);

  /* Dec 2010 fine corrections to the weights (K+) */
  if (mcsevt->part[0].type>0) {

    // period 1
    if (sbur->nrun>=20114 && sbur->nrun<=20154) {
      wt_pk *= (1.0 - 0.005*(pkgen-74.0));
      if (pkgen>77.5) wt_pk *= (1.0+4.0*(pkgen-77.5));
    }
    if (sbur->nrun>=20155 && sbur->nrun<=20174) {
      wt_pk *= (1.0 + 0.065*(pkgen-74.0));
      if (pkgen>77.5) wt_pk *= (1.0+1.0*(pkgen-77.5));
    }
    if (sbur->nrun>=20175 && sbur->nrun<=20203) {
      wt_pk *= (1.0 + 0.10*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+0.5*(pkgen-77.0));
      if (pkgen>70.0 && pkgen<71.5) wt_pk *= (1.0-0.5*(71.5-pkgen));
    }

    // periods 2+3
    if (sbur->nrun>=20209 && sbur->nrun<=20226) {
      wt_pk *= (1.0 + 0.05*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+0.5*(pkgen-77.0));
      if (pkgen>70.0 && pkgen<71.5) wt_pk *= (1.0-0.5*(71.5-pkgen));
    }
    if (sbur->nrun>=20228 && sbur->nrun<=20256) {
      wt_pk *= (1.0 + 0.035*(pkgen-74.0));
      if (pkgen>76.8) wt_pk *= (1.0+0.7*(pkgen-76.8));
      if (pkgen>70.0 && pkgen<71.6) wt_pk *= (1.0-0.5*(71.6-pkgen));
    }
    if (sbur->nrun>=20268 && sbur->nrun<=20291) {
      wt_pk *= (1.0 + 0.04*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+6.0*(pkgen-77.0));
    }
    if (sbur->nrun>=20296 && sbur->nrun<=20303) {
      wt_pk *= (1.0 + 0.015*(pkgen-74.0));
      if (pkgen>76.9) wt_pk *= (1.0+5.0*(pkgen-76.9));
    }
    if (sbur->nrun>=20304 && sbur->nrun<=20324) {
      wt_pk *= (1.0 + 0.02*(pkgen-74.0));
      if (pkgen>77.2) wt_pk *= (1.0+1.3*(pkgen-77.2));
      if (pkgen>70.0 && pkgen<71.5) wt_pk *= (1.0-0.25*(71.5-pkgen));
    }

    // period 4
    if (sbur->nrun>=20332 && sbur->nrun<=20351) {
      if (pkgen>77.3) wt_pk *= (1.0+4.0*(pkgen-77.3));
      if (pkgen>70.0 && pkgen<71.0) wt_pk *= (1.0+1.0*(71.0-pkgen));
    }
    if (sbur->nrun>=20352 && sbur->nrun<=20371) {
      wt_pk *= (1.0 - 0.01*(pkgen-74.0));
      if (pkgen>77.3) wt_pk *= (1.0+6.0*(pkgen-77.3));
      if (pkgen>70.0 && pkgen<71.0) wt_pk *= (1.0+2.0*(71.0-pkgen));
    }
    if (sbur->nrun>=20387 && sbur->nrun<=20404) {
      wt_pk *= (1.0 - 0.015*(pkgen-74.0));
      if (pkgen>77.3) wt_pk *= (1.0+4.5*(pkgen-77.3));
      if (pkgen>70.0 && pkgen<71.0) wt_pk *= (1.0+4.0*(71.0-pkgen));
    }

    // period 5
    if (sbur->nrun>=20410 && sbur->nrun<=20424) {
      wt_pk *= (1.0 - 0.01*(pkgen-74.0));
      if (pkgen>77.5) wt_pk *= (1.0+12.0*(pkgen-77.5));
      if (pkgen<71.2) wt_pk *= (1.0+ 2.5*(71.2-pkgen));
    }
    if (sbur->nrun>=20438 && sbur->nrun<=20453) {
      wt_pk *= (1.0 - 0.015*(pkgen-74.0));
      if (pkgen>77.5) wt_pk *= (1.0+16.0*(pkgen-77.5));
      if (pkgen<71.1) wt_pk *= (1.0+ 5.0*(71.1-pkgen));
    }
    if (sbur->nrun>=20459 && sbur->nrun<=20478) {
      wt_pk *= (1.0 - 0.01*(pkgen-74.0));
      if (pkgen>77.4) wt_pk *= (1.0+10.0*(pkgen-77.4));
      if (pkgen<71.0) wt_pk *= (1.0+ 3.0*(71.0-pkgen));
    }
    if (sbur->nrun>=20482 && sbur->nrun<=20485) {
      wt_pk *= (1.0 + 0.00*(pkgen-74.0));
      if (pkgen>77.3) wt_pk *= (1.0+6.0*(pkgen-77.3));
      if (pkgen<71.0) wt_pk *= (1.0+5.0*(71.0-pkgen));
    }
  }

  /* Jan-Feb 2011 fine corrections to the weights (K-) */
  else {

    // period 1
    if (sbur->nrun>=20114 && sbur->nrun<=20154) {
      wt_pk *= (1.0 - 0.005*(pkgen-74.0));
      if (pkgen>77.5) wt_pk *= (1.0+4.0*(pkgen-77.5));
    }
    if (sbur->nrun>=20155 && sbur->nrun<=20174) {
      if (pkgen>77.5) wt_pk *= (1.0+1.0*(pkgen-77.5));
    }
    if (sbur->nrun>=20175 && sbur->nrun<=20203) {
      wt_pk *= (1.0 + 0.05*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+0.5*(pkgen-77.0));
      if (pkgen>70.5 && pkgen<71.5) wt_pk *= (1.0-0.5*(71.5-pkgen));
    }

    // periods 2+3
    if (sbur->nrun>=20209 && sbur->nrun<=20226) {
      wt_pk *= (1.0 + 0.03*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+0.5*(pkgen-77.0));
      if (pkgen>70.0 && pkgen<71.5) wt_pk *= (1.0-0.5*(71.5-pkgen));
    }
    if (sbur->nrun>=20228 && sbur->nrun<=20256) {
      wt_pk *= (1.0 + 0.035*(pkgen-74.0));
      if (pkgen>76.8) wt_pk *= (1.0+0.7*(pkgen-76.8));
      if (pkgen>70.0 && pkgen<71.6) wt_pk *= (1.0-0.5*(71.6-pkgen));
    }
    if (sbur->nrun>=20268 && sbur->nrun<=20291) {
      wt_pk *= (1.0 + 0.04*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+6.0*(pkgen-77.0));
    }
    if (sbur->nrun>=20296 && sbur->nrun<=20303) {
      wt_pk *= (1.0 + 0.05*(pkgen-74.0));
      if (pkgen>76.9) wt_pk *= (1.0+5.0*(pkgen-76.9));
    }

    // period 6
    if (sbur->nrun>=20487 && sbur->nrun<=20531) {
      wt_pk *= (1.0 + 0.02*(pkgen-74.0));
      if (pkgen>77.0) wt_pk *= (1.0+4.0*(pkgen-77.0));
      if (pkgen>70.0 && pkgen<71.5) wt_pk *= (1.0+0.4*(71.5-pkgen));
    }
  }

  if (wt_pk<0) wt_pk = 0.01;

  buffer[9]  = wt_pk;

  buffer[18] = pmugen;
  buffer[19] = 2.88e-6 + 1.69e-8 * pmugen;

  buffer[15] = pmugen;
  buffer[41] = mcsevt->part[1].p[1] / mcsevt->part[1].p[3];
  buffer[42] = mcsevt->part[1].p[2] / mcsevt->part[1].p[3];

  double dxdz = mcsevt->part[0].p[1]/mcsevt->part[0].p[3];
  double dydz = mcsevt->part[0].p[2]/mcsevt->part[0].p[3];



  x = mcsevt->part[5].pvertex[0];
  y = mcsevt->part[5].pvertex[1];
  double z = mcsevt->part[5].pvertex[2]/100.0;
  double r = sqrt(x*x+y*y);

  HF1 (990, z, 1.);

  /*
  double xxx = mcsevt->part[1].pvertex[0] +
    mcsevt->part[1].p[1]/mcsevt->part[1].p[3] *
    (ZDCH1-mcsevt->part[1].pvertex[2]);
  double yyy = mcsevt->part[1].pvertex[1] +
    mcsevt->part[1].p[2]/mcsevt->part[1].p[3] *
    (ZDCH1-mcsevt->part[1].pvertex[2]);
  double rrr = sqrt(xxx*xxx+yyy*yyy);
  */

  /*
  if (z>90 && z<97)  HF1 (991, r, 1.);
  if (z>97 && z<103) HF1 (992, r, 1.);
  HF2 (993, z, r, 1.);
  */

  // K2pi check
  /*
  double xg1 =
    mcsevt->part[3].pvertex[0] +
    mcsevt->part[3].p[1]/mcsevt->part[3].p[3]*
    (Geom->Lkr.z-mcsevt->part[3].pvertex[2]);
  double yg1 =
    mcsevt->part[3].pvertex[1] +
    mcsevt->part[3].p[2]/mcsevt->part[3].p[3]*
    (Geom->Lkr.z-mcsevt->part[3].pvertex[2]);
  double xg2 =
    mcsevt->part[4].pvertex[0] +
    mcsevt->part[4].p[1]/mcsevt->part[4].p[3]*
    (Geom->Lkr.z-mcsevt->part[4].pvertex[2]);
  double yg2 =
    mcsevt->part[4].pvertex[1] +
    mcsevt->part[4].p[2]/mcsevt->part[4].p[3]*
    (Geom->Lkr.z-mcsevt->part[4].pvertex[2]);

  buffer[91] = buffer[92] = -9999;
  if (LKr_acc (sbur->nrun, xg1, yg1, 8)) { // not in acc
    buffer[91] = xg2;
    buffer[92] = yg2;
  }
  if (LKr_acc (sbur->nrun, xg2, yg2, 8)) { // not in acc
    buffer[91] = xg1;
    buffer[92] = yg1;
  }
  */

  /* Ke2g geometry */
  double xg = (mcsevt->Npart>2) ?
    mcsevt->part[2].pvertex[0] +
    mcsevt->part[2].p[1]/mcsevt->part[2].p[3]*
    (Geom->Lkr.z-mcsevt->part[2].pvertex[2]) : 0.0;
  double yg = (mcsevt->Npart>2) ?
    mcsevt->part[2].pvertex[1] +
    mcsevt->part[2].p[2]/mcsevt->part[2].p[3]*
    (Geom->Lkr.z-mcsevt->part[2].pvertex[2]) : 0.0;
  buffer[11] = xg;
  buffer[12] = yg;
  buffer[13] = mcsevt->part[2].p[0];

  /** for mu->e decays **/
  /*
  double pe[4], pm[4], pk[4];
  double pe_in_m_frame[4], pm_in_k_frame[4], pk_in_m_frame[4];
  for (i=0; i<4; i++) { // float to double
    pe[i] = mcsevt->part[3].p[i];
    pm[i] = mcsevt->part[1].p[i];
    pk[i] = mcsevt->part[0].p[i];
  }

  calc_cm_mom_my(pm, pe, pe_in_m_frame); // electron in muon frame
  calc_cm_mom_my(pk, pm, pm_in_k_frame); // muon in kaon frame
  calc_cm_mom_my(pm, pk, pk_in_m_frame); // kaon in muon frame

  double costhe1 = CosThe(&pe_in_m_frame[1], &pk_in_m_frame[1]);
  double emax    = 0.5*(MMU*MMU+MEL*MEL)/MMU;
  double xx      = pe_in_m_frame[0]/emax;
  */

  //  dumpsmc(mcsevt);

  /*
  if (mcsevt->Npart==3 && mcsevt->part[1].dvertex[2]<8500) {
    printf ("@@@@ %f %f %f\n", xx, costhe1, costhe2);
  }
  */

  /*
  if (mcsevt->Npart==3 && mcsevt->part[1].dvertex[2]<7000
      && mcsevt->part[2].p[0]/mcsevt->part[1].p[0]>0.95) {
    printf ("@@@@ %f %f %f\n", xx, costhe1, costhe2);
  }
  */

  /*****************************************/

  HF1 (99996, sbur->nrun-20000, 1.0);
  HF1 (99997, mcsevt->part[0].dvertex[2]/100., 1.0);
  HF1 (99998, sbur->nrun-20000, buffer[9]);
  HF1 (99999, mcsevt->part[0].dvertex[2]/100., buffer[9]);

  //  if (egamma<0.005) return 0;

  double costhe =
    (mcsevt->part[1].p[1]*mcsevt->part[2].p[1] +
     mcsevt->part[1].p[2]*mcsevt->part[2].p[2] +
     mcsevt->part[1].p[3]*mcsevt->part[2].p[3]) /
    sqrt(SQR(mcsevt->part[1].p[1])+SQR(mcsevt->part[1].p[2])+
	 SQR(mcsevt->part[1].p[3])) /
    sqrt(SQR(mcsevt->part[2].p[1])+SQR(mcsevt->part[2].p[2])+
	 SQR(mcsevt->part[2].p[3]));

  HF1 (10000, egamma, 1.);
  HF1 (10005, eelmax-eel, 1.);
  HF2 (10010, x, y, 1.);
  HF1 (10020, costhe, 1.);
  HF1 (10021, acos(costhe), 1.);
  HF2 (10022, egamma, acos(costhe), 1.);
  HF2 (10100, mcsevt->part[1].pvertex[2], mcsevt->part[1].p[0], 1.);

  ibuffer[0] = 0; // Ke2 passed
  ibuffer[1] = 0; // Kmu2 passed
  ibuffer[2] = 0;

  buffer[21] = -999; // MM2(e)
  buffer[22] = -999; // Ptrack

  user_superCmpEvent(sbur,sevt);

  // K-->munuee
  /*
  double zzz =
    (SQR(mcsevt->part[2].p[0]+mcsevt->part[3].p[0])-
     SQR(mcsevt->part[2].p[1]+mcsevt->part[3].p[1])-
     SQR(mcsevt->part[2].p[2]+mcsevt->part[3].p[2])-
     SQR(mcsevt->part[2].p[3]+mcsevt->part[3].p[3])) / SQMKCH;
  printf (" ... zzz = %f %f\n", zzz, buffer[5]);
  if (ibuffer[0]) printf (" ... yyy = %f %f\n",zzz, buffer[5]);
  */

  //  if (ibuffer[2]) dumpsmc (mcsevt);

  /*
  if (ibuffer[0] && buffer[21]>-1.0 && buffer[21]<0.01) {
    dumpsmc(mcsevt);
    printf ("mm2 = %f\n", buffer[21]);
  }
  */  

  /*
  if (ibuffer[0]) {
    printf (" @@@@ %f %f %f %f\n",
	    mcsevt->part[0].dvertex[2]/100.,
	    mcsevt->part[1].dvertex[2]/100.,
	    xx, costhe1);
    dumpsmc(mcsevt);
  }
  */

  if (ibuffer[0]==1) { // Ke2 passed

    /*
    if (buffer[22]<20) {
      dumpsmc(mcsevt);
      printf ("... MM2 Ptrack = %f %f\n",buffer[21],buffer[22]);
    }
    */

    /*
    if (mcsevt->Npart>5) {
      //      dumpsmc(mcsevt);

      for (i=3; i<mcsevt->Npart-2; i++) {
	double Xlkr = mcsevt->part[3].pvertex[0] +
	  mcsevt->part[3].p[1]/mcsevt->part[3].p[3]*(Geom->Lkr.z-mcsevt->part[3].pvertex[2]);
	double Ylkr = mcsevt->part[3].pvertex[1] +
	  mcsevt->part[3].p[2]/mcsevt->part[3].p[3]*(Geom->Lkr.z-mcsevt->part[3].pvertex[2]);
	printf ("@@@@@ %f %f %f\n",Xlkr, Ylkr, mcsevt->part[3].p[0]);
      }
    }
    */

    HF1 (10001, egamma, 1.);
    HF1 (10006, eelmax-eel, 1.);
    HF2 (10011, x, y, 1.);
    HF2 (10101, mcsevt->part[1].pvertex[2], mcsevt->part[1].p[0], 1.);
  }

  if (ibuffer[1]==1) { // Km2 passed
    HF1 (10002, egamma, 1.);
  }

  double xx1 = mcsevt->part[1].pvertex[0] +
    mcsevt->part[1].p[1]/mcsevt->part[1].p[3]*
    (ZDCH1-mcsevt->part[1].pvertex[2]);
  double yy1 = mcsevt->part[1].pvertex[1] +
    mcsevt->part[1].p[2]/mcsevt->part[1].p[3]*
    (ZDCH1-mcsevt->part[1].pvertex[2]);

  /*
  if (mcsevt->part[1].p>13 && mcsevt->part[1].p[0]<15 &&
      mcsevt->part[1].pvertex[2]>2500)
    printf ("@@@@ %f %f\n", xx1, yy1);
  */

  /*
  if (mcsevt->Npart>=6 && abs(mcsevt->part[5].type)==64)
    dumpsmc(mcsevt);
  */

  /*
  if (mcsevt->Npart>2 && mcsevt->part[2].type!=64) {
    dumpsmc (mcsevt);
  }
  */

  /*
  printf ("### %f %f %f %f\n",
	  mcsevt->part[0].p[0], mcsevt->part[0].p[1],
	  mcsevt->part[0].p[2], mcsevt->part[0].p[3]);
  printf (" @@@@ %f %f\n",
	  mcsevt->part[0].p[3], mcsevt->part[0].p[1]/mcsevt->part[0].p[3]);
  */

  /*
  if (mcsevt->part[1].pvertex[2]<2000 || mcsevt->part[1].pvertex[2]>7000) return 0;
  double dz,dxdz,dydz,r;
  dz   = ZLKR - mcsevt->part[1].pvertex[2];
  dxdz =
    mcsevt->part[1].p[1]/mcsevt->part[1].p[3] -
    mcsevt->part[2].p[1]/mcsevt->part[2].p[3];
  dydz =
    mcsevt->part[1].p[2]/mcsevt->part[1].p[3] -
    mcsevt->part[2].p[2]/mcsevt->part[2].p[3];
  r = sqrt(SQR(dxdz*dz) + SQR(dydz*dz));
  printf (" @@@@ %f %f %f\n",mcsevt->part[2].p[0], r, sqrt(SQR(dxdz)+SQR(dydz)));
  */

  return 0;
}

/********** MC EVENT DUMPING ****************/
void dumpsmc(superMcEvent *mcevt) {
  int i, j, k, charge;
  double Xlkr, Ylkr, pkick, xx, yy, rr, eop;
  double Xdch[4], Ydch[4];
  superCmpEvent *evt = &mcevt->scmpevt;
  static long int Nevt;
  static float rrrr=0.;

  printf("EVENT # %d\n", ++Nevt);
  printf("TRACKS %d    VERTICES %d\n", evt->Ntrack,evt->Nvtx);
  for (i=0; i<evt->Ntrack; i++) {
    k = evt->track[i].iClus;
    eop = (k>=0) ? evt->cluster[k].energy/evt->track[i].p : -1.;
    printf("  %f (%f) (DCH1 xy %f; %f) (LKr xy %f; %f)\n",
           evt->track[i].p * evt->track[i].q,
           eop,
	   evt->track[i].bx + evt->track[i].bdxdz*(ZDCH1-Geom->DCH.bz),
	   evt->track[i].by + evt->track[i].bdydz*(ZDCH1-Geom->DCH.bz),
	   evt->track[i].x + evt->track[i].dxdz*(ZLKR-Geom->DCH.z),
	   evt->track[i].y + evt->track[i].dydz*(ZLKR-Geom->DCH.z));
  }
  printf("%d CLUSTERS\n", evt->Ncluster);
  for (i=0; i<evt->Ncluster; i++) {
    printf("  (%f %f) %f ",
	   evt->cluster[i].x, evt->cluster[i].y, evt->cluster[i].energy);
    for (j=0; j<evt->Ntrack; j++) if (evt->track[j].iClus==i) printf("#%d ",j);
    printf("\n");
  }
  printf("---\n");

  for (i=0; i<mcevt->Npart; i++) {

    double xxcol = mcevt->part[i].pvertex[0] +
      mcevt->part[i].p[1]/mcevt->part[i].p[3]*
      (-1800.0-mcevt->part[i].pvertex[2]);
    double yycol = mcevt->part[i].pvertex[1] +
      mcevt->part[i].p[2]/mcevt->part[i].p[3]*
      (-1800.0-mcevt->part[i].pvertex[2]);

    double xx1 = mcevt->part[i].pvertex[0] +
      mcevt->part[i].p[1]/mcevt->part[i].p[3]*
      (ZDCH1-mcevt->part[i].pvertex[2]);
    double yy1 = mcevt->part[i].pvertex[1] +
      mcevt->part[i].p[2]/mcevt->part[i].p[3]*
      (ZDCH1-mcevt->part[i].pvertex[2]);

    // coordinates at LKR plane w/o magnetic field
    xx = mcevt->part[i].pvertex[0] +
      mcevt->part[i].p[1]/mcevt->part[i].p[3]*
      (Geom->Lkr.z-mcevt->part[i].pvertex[2]);
    yy = mcevt->part[i].pvertex[1] +
      mcevt->part[i].p[2]/mcevt->part[i].p[3]*
      (Geom->Lkr.z-mcevt->part[i].pvertex[2]);

    pkick = 0;
    if (mcevt->part[i].pvertex[2]<Geom->Magnet.z &&
        mcevt->part[i].type!=16 &&
        mcevt->part[i].type!=4) {
      if (mcevt->part[i].type>0) charge = 1; else charge = -1;

      pkick = -charge*0.31; // 2007 P kick
    }

    Xlkr = xx +
      pkick/mcevt->part[i].p[3]*(Geom->Lkr.z-Geom->Magnet.z);
    Ylkr = yy;

    rr = sqrt(xx*xx+yy*yy);
    printf("%3d | %4.1f %4.1f %8.1f | %4.1f %4.1f %8.1f | %5.2f | xy %6.1f %6.1f | xy %6.1f %6.1f",
           mcevt->part[i].type,
           mcevt->part[i].pvertex[0],
           mcevt->part[i].pvertex[1],
           mcevt->part[i].pvertex[2],
           mcevt->part[i].dvertex[0],
           mcevt->part[i].dvertex[1],
           mcevt->part[i].dvertex[2],
           mcevt->part[i].p[0],
	   xx1, yy1, Xlkr, Ylkr);
  printf("\n");

  //  printf (" ---> %f %f %f %f\n", xxcol, yycol, xx1, yy1);

  }
  printf("---------------------END\n");
  return;
}
