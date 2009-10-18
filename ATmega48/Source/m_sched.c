//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 2-09-05 14:37                                     $
//
// Filename        $Workfile:: m_sched.c                                     $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/m_sched. $
//
// Platform        C
//


#define   INCLUDE_OS


#include  "stdconst.h"
#include  "m_sched.h"
#include  "c_armcomm.h"


UBYTE     Run;


void      mSchedInit(void)
{
  Run = FALSE;

  cArmCommInit();
  Run = TRUE;
}


UBYTE     mSchedCtrl(void)
{
  Run = cArmCommCtrl();

  return (Run);
}


void      mSchedExit(void)
{
  Run = FALSE;

  cArmCommExit();
}

