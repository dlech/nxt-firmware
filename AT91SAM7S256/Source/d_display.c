//
// Programmer
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 29-08-05 11:26                                    $
//
// Filename        $Workfile:: d_display.c                                   $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_display. $
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
