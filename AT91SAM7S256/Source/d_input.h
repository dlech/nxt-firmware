//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-01-09 10:33                                    $
//
// Filename        $Workfile:: d_input.h                                     $
//
// Version         $Revision:: 12                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_inpu $
//
// Platform        C
//

#ifndef   D_INPUT
#define   D_INPUT

void      dInputInit(void);
void      dInputExit(void);

void      dInputGetRawAd(UWORD *pValues, UBYTE No);
void      dInputSetActive(UBYTE Port);
void      dInputSet9v(UBYTE Port);
void      dInputSetInactive(UBYTE Port);

void      dInputSetDirOutDigi0(UBYTE Port);
void      dInputSetDirOutDigi1(UBYTE Port);
void      dInputSetDirInDigi0(UBYTE Port);
void      dInputSetDirInDigi1(UBYTE Port);
void      dInputClearDigi0(UBYTE Port);
void      dInputClearDigi1(UBYTE Port);
void      dInputSetDigi0(UBYTE Port);
void      dInputSetDigi1(UBYTE Port);
void      dInputRead0(UBYTE Port, UBYTE *pData);
void      dInputRead1(UBYTE Port, UBYTE *pData);

UBYTE     dInputGetColor(UBYTE No, UWORD *pCol);

void      dInputColorTx(UBYTE Port, UBYTE Data);
void      dInputReadCal(UBYTE Port, UBYTE *pData);
UBYTE     dInputCheckColorStatus(UBYTE Port);
void      dInputGetAllColors(COLORSTRUCT *pRaw, UBYTE Status);
void      dInputSetColorClkInput(void);
void      dInputClearColor100msTimer(UBYTE No);
UBYTE     dInputChkColor100msTimer(UBYTE No);

void      dInputWaitUS(UBYTE usec);


#endif
