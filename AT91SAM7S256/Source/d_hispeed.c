//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_hispeed.c                                   $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_hisp $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_hispeed.h"
#include  "d_hispeed.r"
#include  <string.h>

const ULONG SPEED_TO_BAUD[16] = {
  1200L,
  2400L,
  3600L,
  4800L,
  7200L,
  9600L,
  14400L,
  19200L,
  28800L,
  38400L,
  57600L,
  76800L,
  115200L,
  230400L,
  460800L,
  BAUD_RATE
};

void dHiSpeedInit(void)
{
  HIGHSPEEDInit;  
}

void dHiSpeedSendData(UBYTE *OutputBuffer, UBYTE BytesToSend)
{
  HIGHSPEEDSendDmaData(OutputBuffer,BytesToSend);
}

void dHiSpeedSetupUart(UBYTE speed, UWORD mode, UBYTE umode)
{
  ULONG baud = SPEED_TO_BAUD[speed];
  HIGHSPEEDSetupUart(speed, baud, ((unsigned int)mode), umode);
}

void dHiSpeedInitReceive(UBYTE *InputBuffer)
{
  HIGHSPEEDInitReceiver(InputBuffer);  
}

void dHiSpeedReceivedData(UWORD *ByteCnt)
{
  HIGHSPEEDReceivedData(ByteCnt);  
}

void dHiSpeedExit(void)
{
  HIGHSPEEDExit;
}

void dHiSpeedBytesToSend(UWORD *bts)
{
  BYTESToSend(*bts);
}
