//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_hispeed.h                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_hisp $
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
