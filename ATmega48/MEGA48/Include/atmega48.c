//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 17-02-05 11:26                                    $
//
// Filename        $Workfile:: atmega48.c                                    $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Mega48/Include/ $
//
// Platform        C
//


void main(void)
{
  HARDWAREInit;
  mSchedInit();
  while(TRUE == mSchedCtrl())
  {
    OSWatchdogWrite;
  }
  mSchedExit();
  HARDWAREExit;
}

