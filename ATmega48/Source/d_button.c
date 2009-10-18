//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 28-10-05 13:46                                    $
//
// Filename        $Workfile:: d_button.c                                    $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_button $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_button.h"
#include  "d_button.r"


void      dButtonInit(void)
{
  BUTTONInit;
}


void      dButtonUpdate(void)
{
  IoFromAvr.Buttons = BUTTONRead;
}


void      dButtonExit(void)
{
  BUTTONExit;
}
