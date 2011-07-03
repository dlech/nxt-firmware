//
// Date init       14.12.2004
//
// Revision date   $Date:: 24-04-08 14:33                                    $
//
// Filename        $Workfile:: d_bt.c                                        $
//
// Version         $Revision:: 3                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_bt.c $
//
// Platform        C
//


#include  "stdconst.h"
#include  "modules.h"
#include  "m_sched.h"
#include  "d_bt.h"
#include  "d_bt.r"
#include  <string.h>

enum
{
  BT_FAST_TIMEOUT   = 500,
  BT_CMD_TIMEOUT_2S = 2000,
  BT_TIMEOUT_30S    = 30000
};

#define   SETTimeout(TOut)              CmdTOut      = 0;\
                                        CmdTOutLimit = TOut
#define   RESETTimeout                  CmdTOut      = 0

static    UWORD CmdTOut;
static    UWORD CmdTOutLimit;

void      dBtInit(void)
{
  SETTimeout(0);
  BTInit;
  BTInitPIOPins;			
}

void      dBtSetBcResetPinLow(void)
{
  BTSetResetLow;			/* Set Reset pin to Bluecore chip low */
}

void      dBtSetBcResetPinHigh(void)
{
  BTSetResetHigh;			/* Set Reset pin to Bluecore chip high */
}

void      dBtStartADConverter(void)
{
  BTStartADConverter;
}

void      dBtInitReceive(UBYTE *InputBuffer, UBYTE Mode, UBYTE NoLengthBytes)
{
  BTInitReceiver(InputBuffer, Mode, NoLengthBytes);
}

void      dBtSetArm7CmdSignal(void)
{
  BT_SetArm7CmdPin;
}

void      dBtClearArm7CmdSignal(void)
{
  BT_ClearArm7CmdPin;	
}

UBYTE     dBtGetBc4CmdSignal(void)
{
  UWORD ADValue;

  BTReadADCValue(ADValue);

  if (ADValue > 0x200)
  {
    ADValue = 1;
  }
  else
  {
    ADValue = 0;
  }	
  return(ADValue);
}


UWORD     dBtTxEnd(void)
{
  UWORD   TxEnd;

  REQTxEnd(TxEnd);

  return(TxEnd);

}

UWORD     dBtCheckForTxBuf(void)
{
  UWORD   AvailBytes;

  AVAILOutBuf(AvailBytes);

  return(AvailBytes);
}

void      dBtSendMsg(UBYTE *OutputBuffer, UBYTE BytesToSend, UWORD MsgSize)
{

  /* Used for sending a complete message that can be placed in the buffer -     */
  /* or to send the first part of a message that cannot be placed in the buffer */
  /* once (bigger than the buffer)                                              */
  BTSendMsg(OutputBuffer,BytesToSend, MsgSize);
}

void      dBtSend(UBYTE *OutputBuffer, UBYTE BytesToSend)
{

  /* Used for continous stream of data to be send */
  BTSend(OutputBuffer, BytesToSend);
}

UWORD     dBtReceivedData(UWORD *pLength, UWORD *pBytesToGo, UBYTE NoLengthBytes)
{
  UWORD    RtnVal;

  RtnVal = TRUE;
  BTReceivedData(pLength, pBytesToGo, NoLengthBytes);
  if (*pLength)
  {
    SETTimeout(0);
  }
  else
  {
    if (CmdTOut < CmdTOutLimit)
    {
      CmdTOut++;
      if (CmdTOut >= CmdTOutLimit)
      {
        SETTimeout(0);
        RtnVal = FALSE;
      }
    }
  }
  return(RtnVal);
}

void      dBtResetTimeOut(void)
{
  RESETTimeout;
}

void      dBtClearTimeOut(void)
{
  SETTimeout(0);
}

void      dBtSendBtCmd(UBYTE Cmd, UBYTE Param1, UBYTE Param2, UBYTE *pBdAddr, UBYTE *pName, UBYTE *pCod, UBYTE *pPin)
{
  UBYTE   Tmp;
  UBYTE   SendData;
  UWORD   CheckSumTmp;
  UBYTE   BtOutBuf[128];
  UBYTE   BtOutCnt;

  SendData = 0;
  BtOutCnt = 0;
  switch (Cmd)
  {
    case MSG_BEGIN_INQUIRY:
    {
      BtOutBuf[BtOutCnt++] = MSG_BEGIN_INQUIRY;
      BtOutBuf[BtOutCnt++] = Param1;
      BtOutBuf[BtOutCnt++] = 0x00;
      BtOutBuf[BtOutCnt++] = Param2;
      BtOutBuf[BtOutCnt++] = 0x00;
      BtOutBuf[BtOutCnt++] = 0x00;
      BtOutBuf[BtOutCnt++] = 0x00;
      BtOutBuf[BtOutCnt++] = 0x00;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_CANCEL_INQUIRY:
    {
      BtOutBuf[BtOutCnt++] = MSG_CANCEL_INQUIRY;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_CONNECT:
    {
      BtOutBuf[BtOutCnt++] = MSG_CONNECT;
      memcpy(&(BtOutBuf[BtOutCnt]), pBdAddr, SIZE_OF_BDADDR);
      BtOutCnt            += SIZE_OF_BDADDR;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_OPEN_PORT:
    {
      BtOutBuf[BtOutCnt++] = MSG_OPEN_PORT;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_LOOKUP_NAME:
    {
      BtOutBuf[BtOutCnt++] = MSG_LOOKUP_NAME;
      memcpy(&(BtOutBuf[BtOutCnt]), pBdAddr, SIZE_OF_BDADDR);
      BtOutCnt            += SIZE_OF_BDADDR;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_ADD_DEVICE:
    {
      BtOutBuf[BtOutCnt++] = MSG_ADD_DEVICE;
      memcpy(&(BtOutBuf[BtOutCnt]), pBdAddr, SIZE_OF_BDADDR);
      BtOutCnt += SIZE_OF_BDADDR;
      memcpy(&(BtOutBuf[BtOutCnt]), pName, SIZE_OF_BT_NAME);
      BtOutCnt += SIZE_OF_BT_NAME;
      memcpy(&(BtOutBuf[BtOutCnt]), pCod, SIZE_OF_CLASS_OF_DEVICE);
      BtOutCnt += SIZE_OF_CLASS_OF_DEVICE;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_REMOVE_DEVICE:
    {
      BtOutBuf[BtOutCnt++] = MSG_REMOVE_DEVICE;
      memcpy(&(BtOutBuf[BtOutCnt]), pBdAddr, SIZE_OF_BDADDR);
      BtOutCnt            += SIZE_OF_BDADDR;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_DUMP_LIST:
    {
      BtOutBuf[BtOutCnt++] = MSG_DUMP_LIST;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_CLOSE_CONNECTION:
    {
      BtOutBuf[BtOutCnt++] = MSG_CLOSE_CONNECTION;
      BtOutBuf[BtOutCnt++] = Param1;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_ACCEPT_CONNECTION:
    {
      BtOutBuf[BtOutCnt++] = MSG_ACCEPT_CONNECTION;
      BtOutBuf[BtOutCnt++] = Param1;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_PIN_CODE:
    {
      BtOutBuf[BtOutCnt++] = MSG_PIN_CODE;
      memcpy(&(BtOutBuf[BtOutCnt]), pBdAddr, SIZE_OF_BDADDR);
      BtOutCnt            += SIZE_OF_BDADDR;
      memcpy(&(BtOutBuf[BtOutCnt]), pPin, SIZE_OF_BT_PINCODE);
      BtOutCnt            += SIZE_OF_BT_PINCODE;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_OPEN_STREAM:
    {
      BtOutBuf[BtOutCnt++] = MSG_OPEN_STREAM;
      BtOutBuf[BtOutCnt++] = Param1;

      SendData = 1;
      SETTimeout(BT_TIMEOUT_30S);
    }
    break;

    case MSG_START_HEART:
    {
      BtOutBuf[BtOutCnt++] = MSG_START_HEART;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_SET_DISCOVERABLE:
    {
      BtOutBuf[BtOutCnt++] = MSG_SET_DISCOVERABLE;
      BtOutBuf[BtOutCnt++] = Param1;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_CLOSE_PORT:
    {
      BtOutBuf[BtOutCnt++] = MSG_CLOSE_PORT;
      BtOutBuf[BtOutCnt++] = 0x03;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_SET_FRIENDLY_NAME:
    {
      BtOutBuf[BtOutCnt++] = MSG_SET_FRIENDLY_NAME;
      memcpy(&(BtOutBuf[BtOutCnt]), pName, SIZE_OF_BT_NAME);
      BtOutCnt += SIZE_OF_BT_NAME;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_LINK_QUALITY:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_LINK_QUALITY;
      BtOutBuf[BtOutCnt++] = Param1;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_SET_FACTORY_SETTINGS:
    {
      BtOutBuf[BtOutCnt++] = MSG_SET_FACTORY_SETTINGS;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_LOCAL_ADDR:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_LOCAL_ADDR;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_FRIENDLY_NAME:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_FRIENDLY_NAME;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_DISCOVERABLE:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_DISCOVERABLE;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_PORT_OPEN:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_PORT_OPEN;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_VERSION:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_VERSION;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_GET_BRICK_STATUSBYTE:
    {
      BtOutBuf[BtOutCnt++] = MSG_GET_BRICK_STATUSBYTE;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;

    case MSG_SET_BRICK_STATUSBYTE:
    {
      BtOutBuf[BtOutCnt++] = MSG_SET_BRICK_STATUSBYTE;
      BtOutBuf[BtOutCnt++] = Param1;
      BtOutBuf[BtOutCnt++] = Param2;

      SendData = 1;
      SETTimeout(BT_FAST_TIMEOUT);
    }
    break;
  }

  if (SendData == 1)
  {
    CheckSumTmp = 0;
    for(Tmp = 0; Tmp < BtOutCnt; Tmp++)
    {
      CheckSumTmp += BtOutBuf[Tmp];
    }
    CheckSumTmp = (UWORD) (1 + (0xFFFF - CheckSumTmp));
    BtOutBuf[BtOutCnt++] = (UBYTE)((CheckSumTmp & 0xFF00)>>8);
    BtOutBuf[BtOutCnt++] = (UBYTE)(CheckSumTmp & 0x00FF);
    BTSendMsg(BtOutBuf, BtOutCnt, (UWORD)BtOutCnt);
  }
}



void      dBtExit(void)
{
  BTExit;
}
