//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 10:06                                    $
//
// Filename        $Workfile:: d_input.h                                     $
//
// Version         $Revision:: 6                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_input.h  $
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

#endif
