//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_bt.h                                        $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_bt.h $
//
// Platform        C
//

#ifndef   D_BT
#define   D_BT

#define   STREAM_MODE                   1
#define   CMD_MODE                      2

void      dBtInit(void);
void      dBtExit(void);
void      dBtStartADConverter(void);
void      dBtSetArm7CmdSignal(void);
void      dBtClearArm7CmdSignal(void);
void      dBtInitReceive(UBYTE *InputBuffer, UBYTE Mode);
void      dBtSetBcResetPinLow(void);
void      dBtSetBcResetPinHigh(void);
void      dBtSendBtCmd(UBYTE Cmd, UBYTE Param1, UBYTE Param2, UBYTE *pBdAddr, UBYTE *pName, UBYTE *pCod, UBYTE *pPin);
void      dBtSendMsg(UBYTE *pData, UBYTE Length, UWORD MsgSize);
void      dBtSend(UBYTE *pData, UBYTE Length);
void      dBtResetTimeOut(void);
void      dBtClearTimeOut(void);
UBYTE     dBtGetBc4CmdSignal(void);
UWORD     dBtTxEnd(void);
UWORD     dBtReceivedData(UWORD *pLength, UWORD *pBytesToGo);
UWORD     dBtCheckForTxBuf(void);

#endif
