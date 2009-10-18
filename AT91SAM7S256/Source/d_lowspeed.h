//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 12:13                                    $
//
// Filename        $Workfile:: d_lowspeed.h                                  $
//
// Version         $Revision:: 7                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_lowspeed $
//
// Platform        C
//

#ifndef   D_LOWSPEED
#define   D_LOWSPEED

void      dLowSpeedInit(void);
void	    dLowSpeedStartTimer(void);
void      dLowSpeedStopTimer(void);
void	    dLowSpeedInitPins(UBYTE ChannelNumber);
UBYTE     dLowSpeedSendData(UBYTE ChannelNumber, UBYTE *DataOutBuffer, UBYTE NumberOfTxByte);
void      dLowSpeedReceiveData(UBYTE ChannelNumber, UBYTE *DataInBuffer, UBYTE ByteToRx);
UBYTE     dLowSpeedComTxStatus(UBYTE ChannelNumber);
UBYTE     dLowSpeedComRxStatus(UBYTE ChannelNumber);
void      dLowSpeedExit(void);

#endif
