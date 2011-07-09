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
void      dHiSpeedSetupUart(UBYTE speed, UWORD mode, UBYTE umode);
void      dHiSpeedInitReceive(UBYTE *InputBuffer);
void      dHiSpeedReceivedData(UWORD *ByteCnt);
void      dHiSpeedExit(void);
void      dHiSpeedBytesToSend(UWORD *bts);

/*
int hs_send(U8 address, U8 control, U8 *data, int offset, int len, U16 *CRCTab);
int hs_recv(U8 *data, int len, U16 *CRCTab, int reset);
*/
#endif
