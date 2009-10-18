//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 12:13                                    $
//
// Filename        $Workfile:: d_lowspeed.c                                  $
//
// Version         $Revision:: 14                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_lowspeed $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_lowspeed.h"
#include  "d_lowspeed.r"


void      dLowSpeedInit(void)
{
  LOWSpeedTxInit;
  LOWSpeedTimerInit;
  //ENABLEDebugOutput; 
}

void dLowSpeedStartTimer(void)
{
  ENABLEPWMTimerForLowCom;
}

void dLowSpeedStopTimer(void)
{
  DISABLEPWMTimerForLowCom; 
}

void dLowSpeedInitPins(UBYTE ChannelNumber)
{
  ENABLETxPins(ChannelNumber);  
}

UBYTE dLowSpeedSendData(UBYTE ChannelNumber, UBYTE *DataOutBuffer, UBYTE NumberOfTxByte)
{  
  UBYTE Status;
	
  TxData(ChannelNumber, Status, DataOutBuffer, NumberOfTxByte);
  return(Status);
}

void dLowSpeedReceiveData(UBYTE ChannelNumber, UBYTE *DataInBuffer, UBYTE ByteToRx)
{	
  RxData(ChannelNumber, DataInBuffer, ByteToRx); 
}

UBYTE dLowSpeedComTxStatus(UBYTE ChannelNumber)
{
  UBYTE Status; 

  STATUSTxCom(ChannelNumber, Status)

  return(Status);
}

UBYTE dLowSpeedComRxStatus(UBYTE ChannelNumber)
{
  UBYTE Status; 

  STATUSRxCom(ChannelNumber, Status)

  return(Status);
}

void      dLowSpeedExit(void)
{
  LOWSpeedExit;
}
