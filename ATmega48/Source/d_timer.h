//
// Programmer      
//
// Date init       14.12.2004
//
// Reviser         $Author:: Dktochpe                                        $
//
// Revision date   $Date:: 16-06-05 14:32                                    $
//
// Filename        $Workfile:: d_timer.h                                     $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Ioctrl/Firmware/Source/d_timer. $
//
// Platform        C
//


#ifndef   D_TIMER
#define   D_TIMER

void      dTimerInit(void);
void      dTimerClear(void);
UBYTE     dTimerRead(void);
void      dTimerExit(void);

#define   TIMER_RESOLUTION              (8000000L / 256L)

#endif
