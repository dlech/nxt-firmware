//
// Date init       14.12.2004
//
// Revision date   $Date:: 10-12-07 14:29                                    $
//
// Filename        $Workfile:: sam7s256.c                                    $
//
// Version         $Revision:: 3                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Sam7s256/Incl $
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
