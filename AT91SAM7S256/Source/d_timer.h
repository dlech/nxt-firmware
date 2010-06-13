//
// Date init       14.12.2004
//
// Revision date   $Date: 23-04-08 11:15 $
//
// Filename        $Workfile:: d_timer.h                                     $
//
// Version         $Revision: 2 $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_time $
//
// Platform        C
//
 

#ifndef   D_TIMER
#define   D_TIMER

void      dTimerInit(void);
ULONG     dTimerRead(void);
ULONG     dTimerReadNoPoll(void);
ULONG     dTimerReadHiRes(void);

ULONG     dTimerGetNextMSTickCnt(void);
#define  dTimerReadTicks() (*AT91C_PITC_PIIR)

void      dTimerExit(void);

#endif



