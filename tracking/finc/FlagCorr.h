/* 
   Defines which bit to set into evt->FlagCorr when
   a routine is called
   The number is the bit number starting at 0.
   When call SBIT1 is used, 1 should be added
   Variables SC_SEL... are used when creating super-compact
   to find out which selections were used 
   to create neutral summaries.
*/
#define FC_SELCHARGED     0
#define FC_SEL2PI0        1
#define FC_SEL3PI0        2
#define FC_BLUEFIELD      3
#define FC_LKRPEDCOR      4
#define FC_LKRPOSCOR      5
#define FC_LKRSHARING     6  
#define FC_HODOTIME       7
#define FC_NHODTIME       8
#define FC_LKRTIME        9
#define FC_TAGTIME       10
#define FC_AKSFLAG       11
#define FC_MUONREC       12
#define FC_MUONREJECT    13
#define FC_GEOMCOR       14
#define FC_NEWCHARGED    15
#define FC_MAGNETCOR     16
#define FC_MUONTRACKREC  17
#define FC_MUONVTXREC    18
#define FC_LKRCALCOR     19
#define FC_LKRCALCOR1    20
#define FC_LKRCALCOR2    21
#define FC_LKRCALCOR3    22
#define FC_SEL2GAM       23
#define FC_MUONTRACKRECCOR 24
#define FC_LKRCALHI2K    25
#define FC_MCTAIL        26
#define FC_PEDINT        27
#define SC_SEL2PI0       29
#define SC_SEL3PI0       30
#define SC_SEL2GAM       31

/* for Kcharged override FC_SEL2PI0 and FC_SEL3PI0 */
#define FC_SEL3PIC        1
#define FC_SEL3PIN        2

