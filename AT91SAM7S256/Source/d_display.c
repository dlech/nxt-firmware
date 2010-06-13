//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dkandlun                                        $
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_display.c                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_disp $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_display.h"
#include  "d_display.r"


void      dDisplayInit(void)
{
  DISPLAYInit;
}


void      dDisplayOn(UBYTE On)
{
  if (On)
  {
    DISPLAYOn;
  }
  else
  {
    DISPLAYOff;
  }
}


UBYTE     dDisplayUpdate(UWORD Height,UWORD Width,UBYTE *pImage)
{
  return (DISPLAYUpdate(Height,Width,pImage));
}


void      dDisplayExit(void)
{
  DISPLAYExit;
}
