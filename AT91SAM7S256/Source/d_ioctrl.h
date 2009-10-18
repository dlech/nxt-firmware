//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:50                                     $
//
// Filename        $Workfile:: d_ioctrl.h                                    $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_ioctrl.h $
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
