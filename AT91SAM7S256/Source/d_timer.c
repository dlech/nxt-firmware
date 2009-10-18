//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 10:18                                    $
//
// Filename        $Workfile:: d_timer.c                                     $
//
// Version         $Revision:: 3                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_timer.c  $
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
  ULONG   Tmp;

  TIMERRead(Tmp);

  return (Tmp);
}


void      dTimerExit(void)
{
  TIMERExit;
}

