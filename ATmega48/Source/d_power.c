//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: d_power.c                                     $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_power. $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_power.h"
#include  "d_power.r"


void      dPowerInit(void)
{
  POWERInit;
}


void      dPowerRechargeable(UBYTE Mounted)
{
  if (Mounted)
  {
    IoFromAvr.Battery |=  0x8000;
  }
  else
  {
    IoFromAvr.Battery &= ~0x8000;
  }
}


UBYTE     dPowerReadOn(void)
{
  UBYTE   Result = TRUE;

  if (IoToAvr.Power == 0x5A)
  {
    Result = FALSE;
  }

  return (Result);
}


UBYTE     dPowerReadBoot(void)
{
  UBYTE   Result = FALSE;

  if ((IoToAvr.Power == 0xA5) && (IoToAvr.PwmFreq == 0x5A))
  {
    IoToAvr.Power   = 0x00;
    IoToAvr.PwmFreq = 0x00;
    Result          = TRUE;
  }

  return (Result);
}


void      dPowerSelect(void)
{
  POWERSelect;
}


UWORD     dPowerConvert(void)
{
  UWORD   Result;

  POWERConvert(Result);

  return (Result);
}


void      dPowerDeselect(void)
{
  POWERDeselect;
}


void      dPowerWriteOn(UBYTE On)
{
  if (On == TRUE)
  {
    POWEROn;
  }
  else
  {
    POWEROff;
  }
}


void      dPowerHigh(void)
{
  POWERHigh;
}


void      dPowerFloat(void)
{
  POWERFloat;
}


void      dPowerExit(void)
{
  POWERExit;
}
