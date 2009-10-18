//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 10:06                                    $
//
// Filename        $Workfile:: d_input.c                                     $
//
// Version         $Revision:: 14                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_input.c  $
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
}

void      dInputClearDigi1(UBYTE Port)
{
  INPUTClearDigi1(Port);
}

void      dInputSetDigi0(UBYTE Port)
{
  INPUTSetDigi0(Port);
}

void      dInputSetDigi1(UBYTE Port)
{
  INPUTSetDigi1(Port);
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


void      dInputExit(void)
{
  INPUTExit;
}

