//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_button.c                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_butt $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_button.h"
#include  "d_button.r"

static    UBYTE TimeTick;

void      dButtonInit(UBYTE Prescaler)
{
  TimeTick = Prescaler;
  BUTTONInit;
}

void      dButtonRead(UBYTE *pButton)
{
  BUTTONRead(pButton);
}

void      dButtonExit(void)
{
  BUTTONExit;
}
