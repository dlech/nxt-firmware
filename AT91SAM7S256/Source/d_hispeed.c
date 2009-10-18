//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 12:13                                    $
//
// Filename        $Workfile:: d_hispeed.c                                   $
//
// Version         $Revision:: 8                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_hispeed. $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_hispeed.h"
#include  "d_hispeed.r"

void dHiSpeedInit(void)
{
  HIGHSPEEDInit;  
}

void dHiSpeedSendData(UBYTE *OutputBuffer, UBYTE BytesToSend)
{
  HIGHSPEEDSendDmaData(OutputBuffer,BytesToSend);
}

void dHiSpeedSetupUart(void)
{
  HIGHSPEEDSetupUart;
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
