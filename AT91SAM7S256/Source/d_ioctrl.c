//
// Date init       14.12.2004
//
// Revision date   $Date:: 16-05-06 9:50                                     $
//
// Filename        $Workfile:: d_ioctrl.c                                    $
//
// Version         $Revision:: 11                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/d_ioctrl.c $
//
// Platform        C
//


#include  <string.h>
#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_ioctrl.h"
#include  "d_ioctrl.r"

/* Enum related to State */
enum
{
  RX_I2C      = 1,
  TX_I2C      = 2,
  UNLOCK_I2C  = 3,
  WAIT_I2C    = 4
};


static    UBYTE volatile State;

void      dIOCtrlInit(void)
{
  IOCTRLInit;
  State = UNLOCK_I2C;
}

void      dIOCtrlSetPower(UBYTE Power)
{
  INSERTPower(Power);
}

void      dIOCtrlSetPwm(UBYTE Pwm)
{
  INSERTPwm(Pwm);
}

void      dIOCtrlTransfer(void)
{
  UBYTE   B;

  CHECKTime(B);
  if (B)
  {
    switch(State)
    {
      case TX_I2C:
      {
        FULLDataTx;
        State = RX_I2C;
      }
      break;
      case RX_I2C:
      {
        FULLDataRx;
        State = TX_I2C;
      }
      break;
      case UNLOCK_I2C:
      {
        UNLOCKTx;
        State = WAIT_I2C;
      }
      break;
      case WAIT_I2C:
      {

        /* Intermediate state as unlock string is 47  */
        /* characters which is a little more than 1mS */
        State = TX_I2C;
      }
      break;
      default:
      {
        UNLOCKTx;
        State = WAIT_I2C;
      }
      break;
    }
    SETTime;
  }
}



void      dIOCtrlExit(void)
{
  IOCTRLExit;
}

