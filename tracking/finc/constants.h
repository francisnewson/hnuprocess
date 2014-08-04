/*
  Particles masses given in GeV
*/
#define MASS_K0     0.497672
#define MASS_PI0    0.1349766
#define MASS_PIONC  0.13957018     
#define MASS_ELEC   0.000510998902
#define MASS_MUON   0.1056583568
#define MASS_PROTON 0.938271998
#define MASS_LAMBDA 1.115683
#define MASS_KAONC          0.493677
#define TAU_KAONC           12.384
#define CTAU_KAONC          371.3



/* 
   K0 lifetimes in ns
*/
#define TAU_K0S     0.08935
#define TAU_K0L    51.7
/*
  K0 lifetimes in cm 
*/
#define CTAU_K0S    2.678646
#define CTAU_K0L    1549.927

/*
  Charged/Neutral decay regions
*/
#define Z_DECAY_NEUTRAL 607.2
#define Z_DECAY_CHARGED 609.4
#define CTAU_ORIGIN 607.
/*
  Charged/Neutral decay regions for 2002
*/
#define Z_DECAY_2002 600.
#define CTAU_ORIGIN_2002 600.

/* K0 definitions in compact */

/* 
   to be able to use these values in fortran, write in decimal 
   19-11-98 IWS
*/
/*
#define KS 0x1
#define KL 0x2

#define CHAR 0x0
#define NEUT 0x4

#define TAGTAG 0x8
#define TAGVTX 0x10
*/
#define KS 1
#define KL 2

#define CHAR 0
#define NEUT 4

#define TAGTAG 8
#define TAGVTX 16

#define NEUT3pi0     65536
#define NEUT2gam    131072
#define DALITZ      262144 
#define RAREDECAY   524288 
#define FOURTRACKS 1048576 

/* Define a type of vertex for Dalitz */
#define VTX_DALITZ 256
/* Define flags to trigger non-epsilon summaries */
#define EPSSUM1 1
#define EPSSUM2 2
#define EPSSUM3 3
#define SPESUM 16
#define SPESUM_NEUTRAL 16
#define SPESUM_DALITZ  32
#define SPESUM_RARE    64
#define SPESUM_4TRACKS 128

/* Flags to compute downscaling factors */
#define MBOXTRIG 1
#define ETOTTRIG 2

/* Define bit 0 for sevt->RECstatus: new DCH rec. */
#define VTXFRRDVX 1
/*
  New geometry variables from Marco - 20-11-98
*/
/* hodo vertical z */
#define HODVZPOS 11946.5
/* hodo horizontal z */
#define HODHZPOS 12026.5
/* neutral hodo z */
#define NHOZPOS 12127.6
/* x cell size */
#define LKRXCELL 1.97384
/* projectivity z point */
#define LKRZPROJ 10996.0
/* LKr gap */
#define LKRYGAP 0.055
/* z eta target 1997 */
#define ETAZPOS1997 635.0
/* z eta targs 1998 */
#define ETA1ZPOS 653.65
#define ETA2ZPOS 2115.25
/* z offset of shower max in Lkr Calo*/
#define SHOWER_MAX 30.

#define RUN_MIN_1997 4001.
#define RUN_MAX_1997 6000.

#define RUN_MIN_1998 6001.
#define RUN_MAX_1998 8000.

#define RUN_MIN_1999 8001.
#define RUN_MAX_1999 9999.

#define RUN_MIN_2000 10001.
#define RUN_MAX_2000 10999.

#define RUN_MIN_DIL_2000 10103.
#define RUN_MAX_DIL_2000 10316.

#define RUN_MIN_HKS_2000 10439.
#define RUN_MAX_HKS_2000 10687.

#define RUN_MIN_2001 11001.
#define RUN_MAX_2001 12999.

#define RUN_MIN_2002 13001.
#define RUN_MAX_2002 14999.

/* overlap  between 2003/2004  because many  correction are  valid for
   both and are using RUN_MAX_2003 as upper limit */
#define RUN_MIN_2003 15001.
#define RUN_MAX_2003 17999.

#define RUN_MIN_2004 16001.
#define RUN_MAX_2004 17999.

#define RUN_MIN_2007 20000. 
#define RUN_MAX_2007 22000.


#define BINT_SCALE 10.

/* hevt->flag mnemonic masks */
#define HF_NEUTRAL         0x1
#define HF_LKRDOWNSCALED   0x2
#define HF_LKRDNSC         0x2
#define HF_NTRACKS         0x3c
#define HF_GHOST_TRACK     0x40
#define HF_GHOSTTR         0x40
#define HF_DTSTAMP         0xf00
#define HF_CLUSTER         0xf000
#define HF_CLINTIM         0xf0000
#define HF_TRINTIM         0xf00000

/* hevt->flag offsets */
#define HF_NTRACKS_OFFSET  2
#define HF_GHOSTTR_OFFSET  6
#define HF_DTSTAMP_OFFSET  8
#define HF_CLUSTER_OFFSET  12
#define HF_CLINTIM_OFFSET  16
#define HF_TRINTIM_OFFSET  20

/*
 * $Log: constants.h,v $
 * Revision 2.10  2008/02/03 20:59:55  venelin
 * Added definitions for 2007 min and max run number
 *
 * Revision 2.9  2005/05/09 11:16:53  venelin
 * Charged HC update - numbers of tracks/clusters in time window written to
 * hevt->flag
 *
 * Revision 2.8  2005/03/07 15:50:24  venelin
 * Added bit-coded information to ->flag for
 *  TS of the previous event
 *  Number of extra LKR clusters (assoc R=25cm)
 *
 * Revision 2.7  2004/10/19 15:11:09  clemenci
 * added RUN_MIN_2004 and RUN_MAX_2004
 *
 * Revision 2.6  2004/07/29 17:21:16  clemenci
 * added 2 defines for the meaning of hevt->flag variable
 *
 * Revision 2.5  2004/07/27 23:28:26  clemenci
 * added definitions for hevt->flags bit masks
 *
 * Revision 2.4  2003/07/03 10:22:43  andrew
 * constants.h:
 * added new constants for charged kaons
 *
 * calcScmpEvent.c:
 * added a call to recDCHverteices3SC to reconstruct 3 track vertices
 *
 * Revision 2.3  2003/05/13 13:48:54  andrew
 * Introduced RUN_MIN_2003 RUN_MAX_2003
 * added changes from Marcella for EOB and DCHFE
 *
 * Revision 2.2  2003/03/05 09:10:40  andrew
 * Updates constants to current PDG values, thanks to Manfred J.
 * In an attempt to immitate Heny Choice all values are to full accuracy
 *
 *
 */
