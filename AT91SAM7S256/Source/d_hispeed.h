//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 12:13                                    $
//
// Filename        $Workfile:: d_hispeed.h                                   $
//
// Version         $Revision:: 5                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_hispeed. $
//
// Platform        C
//

#ifndef   D_HISPEED
#define   D_HISPEED

void      dHiSpeedInit(void);
void      dHiSpeedSendData(UBYTE *OutputBuffer, UBYTE BytesToSend);
void      dHiSpeedSetupUart(void);
void      dHiSpeedInitReceive(UBYTE *InputBuffer);
void      dHiSpeedReceivedData(UWORD *ByteCnt);
void      dHiSpeedExit(void);

#endif
