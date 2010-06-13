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
