#include "stdio.h"
#include <math.h>
#include "lkraccep_2007.hh"

int LKr_acc(int nrun, float pos_x, float pos_y, float par)
{
  int flag_masked;
  //float cell_length;
  float xcorner,ycorner;
  
#define cell_length 1.975
  
  // printf("Inside LKr_acc %d %f %f %f \n",nrun,pos_x,pos_y,par);
  
  flag_masked=0;
  // LKr RF is left-handed, the calculation was done in a right-handed system
  // --> the x sign has to be changed
  
  pos_x=-pos_x;
  
  // Standard acceptance cut, taking into account the missing CPD used to test
  // the new LKr read-out
  
  if ( sqrt(pow(pos_x,2)+ pow(pos_y,2))<15.) flag_masked=1;
  if ( fabs(pos_x)>7*8*cell_length-par) flag_masked=1;
  if ( fabs(pos_y)>113) flag_masked=1;
  if ( fabs(pos_x)+fabs(pos_y)> 159.8) flag_masked=1;
  
  // CPD 49, always masked during 2007 run
  xcorner   =+4*8*1.975;
  ycorner   =+6*8*1.975;
  if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
       && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
       && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
       && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
       && (pos_x > xcorner-par) 
       && (pos_x < xcorner+8*cell_length+par)
       && (pos_y > ycorner-par) 
       && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  // CPD 62, always masked during 2007 run
  xcorner   =+4*8*1.975;
  ycorner   =-7*8*1.975;
  if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
       && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
       && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
       && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
       && (pos_x > xcorner-par) 
       && (pos_x < xcorner+8*cell_length+par)
       && (pos_y > ycorner-par) 
       && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  // CPD 193, always masked during 2007 run
  xcorner   =-5*8*1.975;
  ycorner   =+6*8*1.975;
  if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
       && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
       && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
       && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
       && (pos_x > xcorner-par) 
       && (pos_x < xcorner+8*cell_length+par)
       && (pos_y > ycorner-par) 
       && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  // CPD 206, always masked during 2007 run
  xcorner   =-5*8*1.975;
  ycorner   =-7*8*1.975;
  if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
       && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
       && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
       && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
       && (pos_x > xcorner-par) 
       && (pos_x < xcorner+8*cell_length+par)
       && (pos_y > ycorner-par) 
       && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  // CPD 221, always masked during 2007 run
  xcorner   =-6*8*1.975;
  ycorner   =-6*8*1.975;
  if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
      ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	     pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
      ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
       && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
       && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
       && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
       && (pos_x > xcorner-par) 
       && (pos_x < xcorner+8*cell_length+par)
       && (pos_y > ycorner-par) 
       && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  if (nrun>=20157 && nrun<=20157){
    // CPD 80, masked from run 20157 to run 20157
    xcorner   = 2*8*1.975;
    ycorner   = 7*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  if (nrun>=20000 && nrun<=20183){
    // CPD 21, masked from run 20000 to run 20183, as requested by Andreas
    xcorner   = 6*8*1.975;
    ycorner   = 2*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  if (nrun>=20410 && nrun<=20413){
    // CPD 211, masked from run 20410 to run 20413, as requested by Evgueni
    xcorner   = -6*8*1.975;
    ycorner   =  4*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  if (nrun>=20000 && nrun<=20174){
    // CPD 236, masked from run 20000 to run 20174
    xcorner   =-7*8*1.975;
    ycorner   =-5*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20345 && nrun<=20349){
    // CPD 162, masked from run 20345 to run 20349
    xcorner   =-3*8*1.975;
    ycorner   = 5*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20367 && nrun<=20379){
    // CPD  35, masked from run 20367 to run 20379
    xcorner   = 5*8*1.975;
    ycorner   = 4*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20412 && nrun<=20414){
    // CPD  127, masked from run 20412 to run 20414
    xcorner   = 0*8*1.975;
    ycorner   =-8*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20451 && nrun<=20456){
    // CPD  231, masked from run 20451 to run 20456
    xcorner   =-7*8*1.975;
    ycorner   = 0*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20546 && nrun<=20557){
    // CPD  164, masked from run 20546 to the end
    xcorner   =-3*8*1.975;
    ycorner   = 3*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20558 && nrun<=20615){
    // CPD  164, masked from run 20546 to the end
    xcorner   =-3*8*1.975;
    ycorner   = 3*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  115, masked from run 20558 to run 20615
    xcorner   = 0*8*1.975;
    ycorner   = 4*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20616 && nrun<=20635){
    // CPD  164, masked from run 20546 to the end
    xcorner   =-3*8*1.975;
    ycorner   = 3*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  93, masked from run 20616 to the end
    xcorner   = 2*8*1.975;
    ycorner   =-6*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  } else if (nrun>=20636 && nrun<=20999){
    // CPD  164, masked from run 20546 to the end
    xcorner   =-3*8*1.975;
    ycorner   = 3*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  93, masked from run 20616 to the end
    xcorner   = 2*8*1.975;
    ycorner   =-6*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) || 
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  140, masked from run 20636 to the end
    xcorner   =-1*8*1.975;
    ycorner   =-5*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  143, masked from run 20636 to the end
    xcorner   =-1*8*1.975;
    ycorner   =-8*8*1.975;
    if   (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
	   && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
	   && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
	   && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
	   && (pos_x > xcorner-par) 
	   && (pos_x < xcorner+8*cell_length+par)
	   && (pos_y > ycorner-par) 
	   && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  188, masked from run 20636 to the end
    xcorner   =-4*8*1.975;
    ycorner   =-5*8*1.975;
    if   (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
	   && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
	   && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
	   && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
	   && (pos_x > xcorner-par) 
	   && (pos_x < xcorner+8*cell_length+par)
	   && (pos_y > ycorner-par) 
	   && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    // CPD  197, masked from run 20636 to the end
    xcorner   =-5*8*1.975;
    ycorner   = 2*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||     
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  
  if (nrun>=20252 && nrun<=20280){
    // CPD 215, masked from run 20252 to run 20280, added by A. Winhart 23.09.2010
    xcorner   = -6*8*1.975;
    ycorner   =  0*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  if (nrun>=20321 && nrun<=20324){
    // CPD 228, masked from run 20321 to run 20324,  added by A. Winhart 23.09.2010
    xcorner   = -7*8*1.975;
    ycorner   =  3*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  if (nrun>=20321 && nrun<=20324){
    // CPD 229, masked from run 20321 to run 20324,  added by A. Winhart 23.09.2010
    xcorner   = -7*8*1.975;
    ycorner   =  2*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }
  if (nrun==20291){
    // CPD 230, masked for run 20291,  added by A. Winhart 23.09.2010
    xcorner   = -7*8*1.975;
    ycorner   =  1*8*1.975;
    if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
	       pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
         && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
         && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
         && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
         && (pos_x > xcorner-par) 
         && (pos_x < xcorner+8*cell_length+par)
         && (pos_y > ycorner-par) 
         && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
  }

  ////////////////////////////////////////////////////////////////////
  //  Many CPDs were missing in P8, added by A. Winhart 04.02.2011  //
  ////////////////////////////////////////////////////////////////////

  if (nrun>=21082 && nrun<=21120) {
    
    // The two upper and two lower rows of CPDs are missing completely during P8
    if ( fabs(pos_y)>6*8*cell_length-par) flag_masked=1;

    if (nrun>=21090){
      // CPD 109, masked from run 21090 to run 21120
      xcorner   =  1*8*1.975;
      ycorner   = -6*8*1.975;
      if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
	   && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
	   && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
	   && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
	   && (pos_x > xcorner-par) 
	   && (pos_x < xcorner+8*cell_length+par)
	   && (pos_y > ycorner-par) 
	   && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    }
    if (nrun>=21110){
      // CPD 195, masked from run 21110 to run 21120
      xcorner   = -5*8*1.975;
      ycorner   =  4*8*1.975;
      if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
	   && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
	   && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
	   && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
	   && (pos_x > xcorner-par) 
	   && (pos_x < xcorner+8*cell_length+par)
	   && (pos_y > ycorner-par) 
	   && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    }
    if (nrun==21120){
      // CPD 204, masked for run 21120
      xcorner   = -5*8*1.975;
      ycorner   = -5*8*1.975;
      if (((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner-2*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner+10*cell_length)),2)) < (par-2*cell_length))) ||
	  ((sqrt(pow((pos_x-(xcorner+10*cell_length)),2)+
		 pow((pos_y-(ycorner-2*cell_length)),2)) < (par-2*cell_length))) ||
	  ((pos_y+pos_x > ycorner+xcorner- 2*cell_length-par)
	   && (pos_y+pos_x < ycorner+xcorner+18*cell_length+par)  
	   && (pos_y-pos_x > ycorner-xcorner-10*cell_length-par)  
	   && (pos_y-pos_x < ycorner-xcorner+10*cell_length+par)  
	   && (pos_x > xcorner-par) 
	   && (pos_x < xcorner+8*cell_length+par)
	   && (pos_y > ycorner-par) 
	   && (pos_y < ycorner+8*cell_length+par))) flag_masked=1;
    }
  }
  
  return flag_masked;

}

/* Fortran wrapper for LKR_acc*/

int lkr_acc_(int *nrun, float *x, float *y,float *par) {
  return (LKr_acc(*nrun,*x,*y,*par));
}



int accep07_(int *nrun, float *x,float *y) {
  int masked;
  float par=10.; //Fixed from Mauro - means 10cm cut around the dead CPDs
  masked = LKr_acc( *nrun, *x, *y, par);
    if(masked) {
    return 0; 
  } else {
    return 1;
  }  
}
