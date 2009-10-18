//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 16-06-05 14:32                                    $
//
// Filename        $Workfile:: d_timer.c                                     $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_timer. $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_timer.h"
#include  "d_timer.r"

UBYTE     Timer;


void      dTimerInit(void)
{
  TIMERInit;
}


void      dTimerClear(void)
{
  Timer = TIMERRead;
}


UBYTE     dTimerRead(void)
{
  return ((UBYTE)(TIMERRead - Timer));
}


void      dTimerExit(void)
{
  TIMERExit;
}
