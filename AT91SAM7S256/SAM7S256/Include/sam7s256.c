//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 10:08                                    $
//
// Filename        $Workfile:: sam7s256.c                                    $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Sam7s256/Include/ $
//
// Platform        C
//

void main(void)
{
  while(TRUE)
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
}
