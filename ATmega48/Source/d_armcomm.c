//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 28-10-05 13:46                                    $
//
// Filename        $Workfile:: d_armcomm.c                                   $
//
// Version         $Revision:: 5                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_armcom $
//
// Platform        C
//



#include  "stdconst.h"
#include  "m_sched.h"
#include  "c_armcomm.h"
#include  "d_armcomm.r"


void      dArmCommInit(void)
{
  ARMCOMMInit;
}


UBYTE     dArmCommCheck(void)
{
  return (ARMCOMMCheck);
}


UBYTE     dArmCommCopyRight(void)
{
  return (ARMCOMMCopyRight);
}


void      dArmCommExit(void)
{
  ARMCOMMExit;
}
