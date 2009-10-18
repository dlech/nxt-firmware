//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:58                                     $
//
// Filename        $Workfile:: d_button.c                                    $
//
// Version         $Revision:: 4                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_button.c $
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
