//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_ioctrl.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_ioct $
//
// Platform        C
//

#ifndef   D_AVRCOMM
#define   D_AVRCOMM

void      dIOCtrlInit(void);
void      dIOCtrlExit(void);

void      dIOCtrlSetPower(UBYTE Power);
void      dIOCtrlSetPwm(UBYTE Pwm);
void      dIOCtrlTransfer(void);

#endif
