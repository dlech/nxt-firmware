//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_lowspeed.h                                  $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_lows $
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
void      dLowSpeedReceiveData(UBYTE ChannelNumber, UBYTE *DataInBuffer, UBYTE ByteToRx, UBYTE NoRestart);
UBYTE     dLowSpeedComTxStatus(UBYTE ChannelNumber);
UBYTE     dLowSpeedComRxStatus(UBYTE ChannelNumber);
void      dLowSpeedExit(void);
SBYTE     dLowSpeedFastI2C(UBYTE port, UBYTE address, UBYTE *write_data, UBYTE write_len, UBYTE *pReadLen, UBYTE *data_out);

#endif
