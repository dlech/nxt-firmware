//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 17-08-05 8:20                                     $
//
// Filename        $Workfile:: d_output.c                                    $
//
// Version         $Revision:: 13                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_output $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_output.h"
#include  "d_output.r"

static    SBYTE Dutycycle[NOS_OF_AVR_OUTPUTS];
static    UBYTE Frequency;
static    UBYTE LastOutputMode;


void      dOutputInit(void)
{
  UBYTE   Tmp;

  OUTPUTInit;
  for (Tmp = 0;Tmp < NOS_OF_AVR_OUTPUTS;Tmp++)
  {
    Dutycycle[Tmp] = 0;
    OUTPUTWrite(Tmp,Dutycycle[Tmp]);
  }
  LastOutputMode = 0x00;
}


void      dOutputUpdate(UBYTE Brake)
{
  UBYTE   Tmp;
  UBYTE   TmpMask;

  Tmp = IoToAvr.PwmFreq;
  if (Frequency != Tmp)
  {
    if ((Tmp >= 1) && (Tmp <= 32))
    {
      Frequency = Tmp;
      OUTPUTFreq(Frequency);
      for (Tmp = 0;Tmp < NOS_OF_AVR_OUTPUTS;Tmp++)
      {
        Dutycycle[Tmp] = 0;
      }
    }
  }

  TmpMask = IoToAvr.OutputMode;

  for (Tmp = 0;Tmp < NOS_OF_AVR_OUTPUTS;Tmp++)
  {
    if (Brake == TRUE)
    {
      TmpMask               |= (0x01 << Tmp);
      IoToAvr.PwmValue[Tmp]  = 0;
    }
    if ((Dutycycle[Tmp] != IoToAvr.PwmValue[Tmp]) || ((LastOutputMode ^ TmpMask) & (0x01 << Tmp)))
    {
      OUTPUTWriteBrakeMask(TmpMask);
      Dutycycle[Tmp] = IoToAvr.PwmValue[Tmp];
      OUTPUTWrite(Tmp,Dutycycle[Tmp]);
    }
  }
  LastOutputMode = TmpMask;
  OUTPUTUpdate;
}


void      dOutputExit(void)
{
  OUTPUTExit;
}
