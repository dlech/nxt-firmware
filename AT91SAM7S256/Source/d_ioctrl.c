//
// Date init       14.12.2004
//
// Revision date   $Date:: 5-12-07 15:23                                     $
//
// Filename        $Workfile:: d_ioctrl.c                                    $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_ioct $
//
// Platform        C
//


#include  <string.h>
#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_ioctrl.h"
#include  "d_ioctrl.r"


void      dIOCtrlInit(void)
{
  IOCTRLInit;
}

void      dIOCtrlSetPower(UBYTE Power)
{
  INSERTPower(Power);
}

void      dIOCtrlSetPwm(UBYTE Pwm)
{
  INSERTPwm(Pwm);
}

void      dIOCtrlTransfer(void)
{
  I2CTransfer;
}

void      dIOCtrlExit(void)
{
  IOCTRLExit;
}

