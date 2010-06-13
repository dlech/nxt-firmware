//
// Date init       14.12.2004
//
// Revision date   $Date: 23-04-08 11:15 $
//
// Filename        $Workfile:: d_timer.c                                     $
//
// Version         $Revision: 2 $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_time $
//
// Platform        C
//


#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_timer.h"
#include  "d_timer.r"


void      dTimerInit(void)
{
  TIMERInit;
}


ULONG     dTimerRead(void)
{
  ULONG   V;

  TIMERReadAlt(V)
  return (V);
}

ULONG     dTimerReadNoPoll(void)
{
  return (Timer1mS);
}

ULONG     dTimerReadHiRes(void)
{

//  return ((*AT91C_PITC_PIIR)/3); following code is equivalent and about five times faster, see Hacker's Delight or exact division
  ULONG tmp= ((*AT91C_PITC_PIIR)*2863311531);
  if(tmp > 2863311531)
    return tmp - 2863311531;
  else if(tmp > 1431655766)
    return tmp - 1431655766;
  else
    return tmp;
}

ULONG dTimerGetNextMSTickCnt(void) {
  return NextTimerValue;
}

void      dTimerExit(void)
{
  TIMERExit;
}

