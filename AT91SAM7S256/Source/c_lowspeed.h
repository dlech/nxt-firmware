//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_lowspeed.h                                  $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_lows $
//
// Platform        C
//

#ifndef   C_LOWSPEED
#define   C_LOWSPEED

#define   LOWSPEED_RX_TIMEOUT		         100
#define   LOWSPEED_COMMUNICATION_SUCCESS 0x01
#define   LOWSPEED_COMMUNICATION_ERROR   0xFF
#define   SIZE_OF_LSBUFDATA              16
#define   NO_OF_LOWSPEED_COM_CH          4

enum
{
  LOWSPEED_CHANNEL1,
  LOWSPEED_CHANNEL2,
  LOWSPEED_CHANNEL3,
  LOWSPEED_CHANNEL4
};

enum
{
  TIMER_STOPPED,
  TIMER_RUNNING
};

typedef   struct
{
  UBYTE   Buf[SIZE_OF_LSBUFDATA];
  UBYTE   InPtr;
  UBYTE   OutPtr;
}LSDATA;

typedef   struct
{
  LSDATA  OutputBuf[NO_OF_LOWSPEED_COM_CH];
  LSDATA  InputBuf[NO_OF_LOWSPEED_COM_CH];
  UBYTE   RxTimeCnt[NO_OF_LOWSPEED_COM_CH];
  UBYTE   ErrorCount[NO_OF_LOWSPEED_COM_CH];  
  UBYTE   Tmp;
  UBYTE   TimerState;  
}VARSLOWSPEED;

void      cLowSpeedInit(void* pHeader);
void      cLowSpeedCtrl(void);
void      cLowSpeedExit(void);

extern    const HEADER cLowSpeed;

#endif
