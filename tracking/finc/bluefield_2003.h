      INTEGER NBBLUE
      PARAMETER (NBBLUE=5174)
      REAL BBLUE
      INTEGER NZ_BLUE,NX_BLUE,NY_BLUE,IADBX_BLUE
      REAL Z_BLUE,XMIN_BLUE,YMIN_BLUE,STEPX_BLUE,STEPY_BLUE
      COMMON/BLUEFLD_2003/NZ_BLUE,NX_BLUE(54),NY_BLUE(54),
     +               IADBX_BLUE(54),
     +               Z_BLUE(54),XMIN_BLUE(54),YMIN_BLUE(54),
     +               STEPX_BLUE(54),STEPY_BLUE(54),BBLUE(NBBLUE)

/*
*
* The common for the bluefield for 2003
*
*
* $Log: bluefield_2003.h,v $
* Revision 1.1  2003/08/29 07:59:30  andrew
* The common block for the new bluefield for 2003
*
*
*
*
*/
