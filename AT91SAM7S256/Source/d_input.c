//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-01-09 10:34                                    $
//
// Filename        $Workfile:: d_input.c                                     $
//
// Version         $Revision:: 12                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_inpu $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "c_input.h"
#include  "d_input.h"
#include  "d_input.r"


void      dInputInit(void)
{
  INPUTInit;
}

void      dInputSetColorClkInput(void)
{
  COLORClkInput;
}

void      dInputGetAllColors(COLORSTRUCT *pRaw, UBYTE Status)
{
  UPDATEAllColors(pRaw, Status);
}

void      dInputGetRawAd(UWORD *pValues, UBYTE No)
{
  INPUTGetVal(pValues, No);
}

void      dInputSetDirOutDigi0(UBYTE Port)
{
  INPUTSetOutDigi0(Port);
}

void      dInputSetDirOutDigi1(UBYTE Port)
{
  INPUTSetOutDigi1(Port);
}

void      dInputSetDirInDigi0(UBYTE Port)
{
  INPUTSetInDigi0(Port);
}

void      dInputSetDirInDigi1(UBYTE Port)
{
  INPUTSetInDigi1(Port);
}

void      dInputClearDigi0(UBYTE Port)
{
  INPUTClearDigi0(Port);
  INPUTSetOutDigi0(Port);
}

void      dInputClearDigi1(UBYTE Port)
{
  INPUTClearDigi1(Port);
  INPUTSetOutDigi1(Port);
}

void      dInputSetDigi0(UBYTE Port)
{
  INPUTSetDigi0(Port);
  INPUTSetOutDigi0(Port);
}

void      dInputSetDigi1(UBYTE Port)
{
  INPUTSetDigi1(Port);
  INPUTSetOutDigi1(Port);
}

void      dInputRead0(UBYTE Port, UBYTE *pData)
{
  INPUTReadDigi0(Port, pData);
}

void      dInputRead1(UBYTE Port, UBYTE * pData)
{
  INPUTReadDigi1(Port, pData);
}

void      dInputSetActive(UBYTE Port)
{
  INPUTSetActive(Port);
}

void      dInputSet9v(UBYTE Port)
{
  INPUTSet9v(Port);
}

void      dInputSetInactive(UBYTE Port)
{
  INPUTSetInactive(Port);
}

UBYTE     dInputGetColor(UBYTE No, UWORD *pCol)
{
  UBYTE   Status;
  UPDATELed(No, pCol, Status);
  return(Status);
}

void      dInputColorTx(UBYTE Port, UBYTE Data)
{
  COLORTx(Port, Data);
}

void      dInputReadCal(UBYTE Port, UBYTE *pData)
{
  CALDataRead(Port, pData);
}

UBYTE     dInputCheckColorStatus(UBYTE Port)
{
  UBYTE   Status;

  CHECKColorState(Port,Status);
  return(Status);
}

void      dInputClearColor100msTimer(UBYTE No)
{
  CLEARColor100msTimer(No);
}

UBYTE     dInputChkColor100msTimer(UBYTE No)
{
  UBYTE   State;
  COLOR100msStatus(No, State);
  return(State);
}

void      dInputExit(void)
{
  INPUTExit;
}

void      dInputClockHigh(UBYTE Port)
{
  INPUTClkHigh(Port);
}

void      dInputClockLow(UBYTE Port)
{
  INPUTClkLow(Port);
}

void      dInputWaitUS(UBYTE usec)
{
  rInputWaitUS(usec);
}
