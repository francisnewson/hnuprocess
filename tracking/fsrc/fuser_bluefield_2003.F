ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                             c
c     Set of routines prepared by Jean-Baptiste Cheze                         c
c     to correct for the magnetic field inside the blue tube                  c
c     using the measurements performed during winter 98                       c
c     The results of the correction are put in volatiles variables            c
c                                                                             c
c     for each track with i=track index                                       c
c     TRACK_ANAVAR(i,1) = corrected TRACK_PQ(i)                               c 
c     TRACK_ANAVAR(i,2) = corrected TRACK_BX(i)                               c
c     TRACK_ANAVAR(i,3) = corrected TRACK_BY(i)                               c
c     TRACK_ANAVAR(i,4) = corrected TRACK_BDXDZ(i)                            c
c     TRACK_ANAVAR(i,5) = corrected TRACK_BDYDZ(i)                            c
c     for the vertex:                                                         c
c     VERTEX_ANAVAR(ivertex,1) = corrected VERTEX_X(ivertex)                  c
c     VERTEX_ANAVAR(ivertex,2) = corrected VERTEX_Y(ivertex)                  c
c     VERTEX_ANAVAR(ivertex,3) = corrected VERTEX_Z(ivertex)                  c
c     VERTEX_ANAVAR(ivertex,4) = corrected VERTEX_CDA(ivertex)                c
c     VERTEX_ANAVAR(ivertex,5) = corrected VERTEX_PPIPI(ivertex)              c
c     VERTEX_ANAVAR(ivertex,6) = corrected VERTEX_MPIPI(ivertex)              c
c                                                                             c
c     USER_BLUEINIT  should be called in INIT routine                         c
c     USER_BLUEEVENT should be called in EVENT routine                        c

c Modifications: 21-10-98 IWS input to correction changed to VERTEX_CORR      c
c =============               and TRACK_CORR and corrections put in           c
c                31-10-98 IWS Bug introduced 21-10-98 corrected (X,Y were     c
c                             filled instead ob BX,BY)                        c
c                06-11-98 IWS Remove vertex calculation for TRACKCORR         c
c                             + use TRACKCORR_P to compute VERTEX_ANAVAR      c
c                17-11-98 IWS comment 2 lines about momentum                  c
c                18-11-98 IWS use same PION mass as in compact (constants.h)  c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
#include "compactF77.h"
#include "FlagCorr.h"          /* Which bit to set */
#include "constants.h"
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c                                                                             c
      subroutine USER_BLUEINIT                                             
c                                                                             c
c     This routine has to be called at init (from USER_INIT):                 c
c     it reads the file bluefield.data and fills data in a common             c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      IMPLICIT NONE
#include "bluefield_2003.h"  
      INTEGER IRC
      REAL HEAD
      INTEGER NHEAD
      PARAMETER (NHEAD=379)
      DIMENSION HEAD(NHEAD)
      REAL QQ
      DIMENSION QQ(NBBLUE+NHEAD)
      INTEGER I,L
      character*100 BLUE_TUBE
* ------------------------------------------------------------
* ------------------------------------------------------------
*
*
#include "blue_tube_location.h"

      OPEN(UNIT=17,FILE=BLUE_TUBE,STATUS='OLD', IOSTAT=IRC)
      IF (IRC.NE.0) THEN
        PRINT *,' ============================='
        PRINT *,' Problem with OPEN in user_blue_init_03'
        PRINT *, BLUE_TUBE
        PRINT *,' ============================='
        STOP
      ELSE
         PRINT *,'File for blue tube mag. field opened sucessfully'
      ENDIF
      IRC=0
      READ(17,*,ERR=9999) (QQ(I),I=1,NHEAD+NBBLUE)
      DO I=1,NHEAD 
         HEAD(I)=QQ(I)
      ENDDO
      DO I=1,NBBLUE
         BBLUE(I)=QQ(NHEAD+I)
      ENDDO
      CLOSE(17)
      NZ_BLUE=HEAD(1)
      L=1
      IADBX_BLUE(1)=0
      DO I=1,NZ_BLUE
         NX_BLUE(I)=HEAD(L+1)
         XMIN_BLUE(I)=HEAD(L+2)
         STEPX_BLUE(I)=HEAD(L+3)
         NY_BLUE(I)=HEAD(L+4)
         YMIN_BLUE(I)=HEAD(L+5)
         STEPY_BLUE(I)=HEAD(L+6)
         Z_BLUE(I)=-HEAD(L+7)
         IF(I.NE.NZ_BLUE)THEN
            IADBX_BLUE(I+1)=IADBX_BLUE(I)+2*NX_BLUE(I)*NY_BLUE(I)
         ENDIF
         L=L+7
      ENDDO
      RETURN
 9999 WRITE(6,*) 'EoF found on input file - Field map not properly read'
      RETURN
      END  

      SUBROUTINE FDCALINT_VT(XX,XXS,BINTCAL_VT,IOUT_MAP,
     +                       NBLUEFIELD)
      IMPLICIT NONE
#include "bluefield_2003.h"
      INTEGER I
      REAL BINTCAL_VT
      DIMENSION BINTCAL_VT(6)
      REAL ZPL1
*------------------------------------------------------------
      INTEGER NCROSS,NBLUEFIELD,IOUT_MAP
      INTEGER NPRINT,NPRINTMAX
      REAL FF
      DIMENSION FF(3)
      REAL BX,BY
c 57 = 54+3, 3 = extra-points:vertex,kevlar window,DCH1
      DIMENSION BX(57),BY(57)
      REAL BINTX,BINTY,BINT2X,BINT2Y,ZXY,XXY,YXY
      DIMENSION BINTX(57),BINTY(57),BINT2X(57),BINT2Y(57),ZXY(57)
      DIMENSION XXY(57),YXY(57)
      REAL XX,XXS,XXLOC
      DIMENSION XX(3),XXS(2),XXLOC(3)
      DATA NPRINT,NPRINTMAX/0,10/
C ZPL1 IS THE Z COORDINATE OF THE PLANE 1 CHAMBER 1
      DATA ZPL1/21700.6/
C --------------------------------->
      NPRINT=NPRINT+1
C --------------------------------->
C Calculation of field integrals
C z origin at Kl target
C FF IN KGAUSS
      NCROSS=1
      IF(NBLUEFIELD.EQ.2)THEN
         CALL BLUEFLD_GET(XX,FF,1,IOUT_MAP)
      ELSE
         FF(1)=0.00012
         FF(2)=-0.00015
      ENDIF
      BX(NCROSS)=FF(1)
      BY(NCROSS)=FF(2)
      ZXY(NCROSS)=XX(3)
      NCROSS=1
      DO I=NZ_BLUE,1,-1
c 21531 = Z coord of kevlar window
         IF(Z_BLUE(I).GT.XX(3)-21531.)THEN
            NCROSS=NCROSS+1
            XXLOC(3)=Z_BLUE(I)+21531.
            XXLOC(1)=XX(1)+XXS(1)*(XXLOC(3)-XX(3))
            XXLOC(2)=XX(2)+XXS(2)*(XXLOC(3)-XX(3))
            IF(NBLUEFIELD.EQ.2)THEN
               CALL BLUEFLD_GET(XXLOC,FF,1,IOUT_MAP)
            ELSE
               FF(1)=0.00012
               FF(2)=-0.00015
            ENDIF
            BX(NCROSS)=FF(1)
            BY(NCROSS)=FF(2)
            ZXY(NCROSS)=XXLOC(3)
            XXY(NCROSS)=XXLOC(1)
            YXY(NCROSS)=XXLOC(2)
            if(i.eq.1)then
               ncross=ncross+1
               BX(NCROSS)=FF(1)
               BY(NCROSS)=FF(2)
               ZXY(NCROSS)=21531.
               XXY(NCROSS)=XXLOC(1)
               YXY(NCROSS)=XXLOC(2)
            endif  
         ENDIF
      ENDDO
      BINTX(1)=0.0
      BINTY(1)=0.0
      DO I=2,NCROSS
         BINTX(I)=BINTX(I-1)+(BX(I-1)+BX(I))*(ZXY(I)-ZXY(I-1))*0.5
         BINTY(I)=BINTY(I-1)-(BY(I-1)+BY(I))*(ZXY(I)-ZXY(I-1))*0.5
      ENDDO     
      BINT2X(1)=0.0
      BINT2Y(1)=0.0
      DO I=2,NCROSS
         BINT2X(I)=BINT2X(I-1)+(BINTX(I-1)+BINTX(I))*
     +        (ZXY(I)-ZXY(I-1))*0.5
         BINT2Y(I)=BINT2Y(I-1)+(BINTY(I-1)+BINTY(I))*
     +        (ZXY(I)-ZXY(I-1))*0.5
      ENDDO     
      NCROSS=NCROSS+1
      BINTX(NCROSS)=BINTX(NCROSS-1)
      BINTY(NCROSS)=BINTY(NCROSS-1)
      BINT2X(NCROSS)=BINT2X(NCROSS-1)+
     +     BINTX(NCROSS)*(ZPL1-21531.)
      BINT2Y(NCROSS)=BINT2Y(NCROSS-1)+
     +     BINTY(NCROSS)*(ZPL1-21531.)
C     FIELD BETWEEN THE KEVLAR WINDOW AND CHAMBER 1 IS IGNORED
      BINTCAL_VT(1)=BINT2Y(NCROSS)
      BINTCAL_VT(2)=BINTY(NCROSS)
      BINTCAL_VT(3)=BY(1)
      BINTCAL_VT(4)=BINT2X(NCROSS)
      BINTCAL_VT(5)=BINTX(NCROSS)
      BINTCAL_VT(6)=BX(1)
 999  CONTINUE
      RETURN
      END


      SUBROUTINE BLUEFLD_GET(X,F,IFRAME,IOUT_MAP)
      IMPLICIT NONE
#include "bluefield_2003.h"
      INTEGER IFRAME,IOUT_MAP
      REAL X,F
      DIMENSION X(3),F(3)
*     ------------------------------------------------------------
      INTEGER INTERUP,INTERDW,ICAL,ILOC,IMAP,JMAP,K1,K2,K3,K4
      INTEGER NDIM,I
      REAL XLOC,YLOC,ZLOC,DIFFX,DIFFY,DIFFZ,CO11,CO21,CO12,CO22
      REAL FLOC
      DIMENSION FLOC(2,2)
*     ------------------------------------------------------------
*     IOUT_MAP=0
      IF(IFRAME.EQ.1)THEN
C     Origin of Z=X(3) coordinate is KL target
         ZLOC=X(3)-9531.-12000.
      ENDIF
      INTERUP=0
      INTERDW=0
C     Downstream the Kevlar Window : Blue field equals 0
      IF(ZLOC.GE.0.0)THEN
         F(1)=0.0
         F(2)=0.0
         F(3)=0.0
         GO TO 999
      ENDIF
C     UPSTREAM the last plane where field has been measured
C     The field is taken to be equal to the field measured in the last plane
*     
C     AND between the first plane measured close to the kevlar window 
C     and the kevlar window itself . Field is equal to the field 
C     measured in the first plane
      IF(ZLOC.LT.Z_BLUE(54))THEN
         INTERUP=54
      ELSE IF(ZLOC.GE.Z_BLUE(1))THEN
         INTERUP=1
      ENDIF 
      DO I=NZ_BLUE,2,-1
         IF(ZLOC.GE.Z_BLUE(I).AND.ZLOC.LT.Z_BLUE(I-1))THEN
            INTERUP=I
            INTERDW=I-1
         ENDIF
      ENDDO
*     CALCULATE THE FIELD AT ZBLUE(INTERUP)  which is the point upstream
*     toward the KS target for ICAL=1
*     CALCULATE THE FIELD AT ZBLUE(INTERDW)  which is the point downstream
*     toward the Kevlar Window  for ICAL=2
      DO ICAL=1,2
         IF(ICAL.EQ.1)THEN
            ILOC=INTERUP
         ELSE 
            ILOC=INTERDW
            IF(ILOC.EQ.0)THEN
               F(1)=FLOC(1,1)
               F(2)=FLOC(2,1)
               F(3)=0.0
               GO TO 999
            ENDIF
         ENDIF
         XLOC=X(1)
         YLOC=X(2)
         IF(XLOC.LT.XMIN_BLUE(ILOC))THEN
            XLOC=XMIN_BLUE(ILOC)
         ENDIF
         IF(YLOC.LT.YMIN_BLUE(ILOC))THEN
            YLOC=YMIN_BLUE(ILOC)
         ENDIF
         IMAP=(XLOC-XMIN_BLUE(ILOC))/STEPX_BLUE(ILOC)+1
         JMAP=(YLOC-YMIN_BLUE(ILOC))/STEPY_BLUE(ILOC)+1
         IF(IMAP.GE.NX_BLUE(ILOC))THEN
            IMAP=NX_BLUE(ILOC)-1
            XLOC=XMIN_BLUE(ILOC)+IMAP*STEPX_BLUE(ILOC)
         ENDIF
         IF(JMAP.GE.NY_BLUE(ILOC))THEN
            JMAP=NY_BLUE(ILOC)-1
            YLOC=YMIN_BLUE(ILOC)+JMAP*STEPY_BLUE(ILOC)
         ENDIF
         K1=(JMAP-1)*NX_BLUE(ILOC)+IMAP+IADBX_BLUE(ILOC)
         K2=K1+1
         K3=K1+NX_BLUE(ILOC)
         K4=K3+1
         DIFFX=XLOC-(XMIN_BLUE(ILOC)+STEPX_BLUE(ILOC)*(IMAP-1))
         DIFFY=YLOC-(YMIN_BLUE(ILOC)+STEPY_BLUE(ILOC)*(JMAP-1))
         DIFFX=DIFFX/STEPX_BLUE(ILOC)
         DIFFY=DIFFY/STEPY_BLUE(ILOC)
         CO22=DIFFX*DIFFY
         CO11=1.-DIFFX-DIFFY+CO22
         CO21=DIFFX-CO22
         CO12=DIFFY-CO22
         FLOC(1,ICAL)=CO11*BBLUE(K1)+CO21*BBLUE(K2)+
     +        CO12*BBLUE(K3)+CO22*BBLUE(K4)
         NDIM=NX_BLUE(ILOC)*NY_BLUE(ILOC)
         FLOC(2,ICAL)=CO11*BBLUE(K1+NDIM)+CO21*BBLUE(K2+NDIM)+
     +        CO12*BBLUE(K3+NDIM)+CO22*BBLUE(K4+NDIM)
C     write(iqtype,*) ' ical k1234 floc',ICAL,K1,K2,K3,K4,
C     +                FLOC(1,ICAL),FLOC(2,ICAL)
      ENDDO
      DIFFZ=ZLOC-Z_BLUE(INTERUP)
      F(1)=FLOC(1,1)+DIFFZ*(FLOC(1,2)-FLOC(1,1))/
     +     (Z_BLUE(INTERDW)-Z_BLUE(INTERUP))
      F(2)=FLOC(2,1)+DIFFZ*(FLOC(2,2)-FLOC(2,1))/
     +     (Z_BLUE(INTERDW)-Z_BLUE(INTERUP))
      F(3)=0.0
 999  CONTINUE
      F(1)=F(1)/100000.
      F(2)=F(2)/100000.
      RETURN
      END

      SUBROUTINE Blue_Tack (nchar,tmom,Vxyz,vpnt,vdir)
c.
c.  Author: Alan Norton
c.  Corrects a track for the blue tube field.   !  revised 15/11/04 <<<<
c.  The track charge is input as nchar = charge[+1,-1], tmom = momentum,
c.  vpnt = (x,y) at Dch1 and vdir = (dxdz,dydz) at Dch1.
c.  Vxyz (x,y,z) is an estimate of the vertex.
C.  On output vpnt and vdir are replaced by effective values which can
c.  be linearly extrapolated back to the vertex region.
c.
c.------------------------------------------------------------------------
c
      IMPLICIT NONE
      INTEGER    nchar,imap
      REAL*4     tmom, Vxyz(3),vpnt(2),vdir(2)
     +,          charmom,Vtem(3),binteg(6)
c
      charmom = float(nchar) * tmom
      Vtem(1) = Vxyz(1)
      Vtem(2) = Vxyz(2)
      Vtem(3) = Vxyz(3) + 12000.
c
      Call FDCALINT_VT (Vtem,vdir,binteg,imap,2)
      vpnt(1) = vpnt(1) - 0.0003*binteg(1)/charmom
      vdir(1) = vdir(1) - 0.0003*binteg(2)/charmom
      vpnt(2) = vpnt(2) - 0.0003*binteg(4)/charmom
      vdir(2) = vdir(2) - 0.0003*binteg(5)/charmom      
      RETURN
      END
*--------1---------2---------3---------4---------5---------6---------712.......8


C
C
C   $Log: fuser_bluefield_2003.F,v $
C   Revision 1.4  2003/11/20 17:54:15  andrew
C   Fix typo in bluefieled correction (Silvia/Ivan)
C
C   Revision 1.3  2003/09/02 09:32:11  andrew
C   removed the quoted arround BLUE_TUBE, so that the preprocessor can get at it
C
C   Revision 1.2  2003/09/02 08:58:47  andrew
C   removed the 2003 form the BLUE_FIELD input file
C
C   Revision 1.1  2003/08/29 07:59:07  andrew
C   New bluefield routines for 2003
C
C
C
