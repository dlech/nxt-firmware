//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 16-06-05 14:32                                    $
//
// Filename        $Workfile:: d_input.c                                     $
//
// Version         $Revision:: 3                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_input. $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_input.h"
#include  "d_input.r"


void      dInputInit(void)
{
  INPUTInit;
}


void      dInputSelect(UBYTE No)
{
  INPUTSelect(No);
}


void      dInputConvert(UBYTE No)
{
  INPUTConvert(No);
}


void      dInputDeselect(UBYTE No)
{
  INPUTDeselect(No);
}


void      dInputExit(void)
{
  INPUTExit;
}
