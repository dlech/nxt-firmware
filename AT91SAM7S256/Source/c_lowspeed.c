//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_lowspeed.c                                  $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_lows $
//
// Platform        C
//

#include  "stdconst.h"
#include  "modules.h"
#include  "c_lowspeed.iom"
#include  "c_input.iom"
#include  "c_lowspeed.h"
#include  "d_lowspeed.h"
#include <string.h>

static    IOMAPLOWSPEED   IOMapLowSpeed;
static    VARSLOWSPEED    VarsLowSpeed;
static    HEADER          **pHeaders;

const UBYTE LOWSPEED_CH_NUMBER[4] = {0x01, 0x02, 0x04, 0x08};

const     HEADER          cLowSpeed =
{
  0x000B0001L,
  "Low Speed",
  cLowSpeedInit,
  cLowSpeedCtrl,
  cLowSpeedExit,
  (void *)&IOMapLowSpeed,
  (void *)&VarsLowSpeed,
  (UWORD)sizeof(IOMapLowSpeed),
  (UWORD)sizeof(VarsLowSpeed),
  0x0000                      //Code size - not used so far
};

SBYTE      cLowSpeedFastI2C(UBYTE ch);

void      cLowSpeedInit(void* pHeader)
{
  pHeaders        = pHeader;

  dLowSpeedInit();
  IOMapLowSpeed.State = COM_CHANNEL_NONE_ACTIVE;
  IOMapLowSpeed.NoRestartOnRead = COM_CHANNEL_RESTART_ALL;
  IOMapLowSpeed.Speed = COM_CHANNEL_NONE_FAST;
  IOMapLowSpeed.pFunc = &cLowSpeedFastI2C;
  VarsLowSpeed.TimerState = TIMER_STOPPED;
}

void      cLowSpeedCompleteRead(UBYTE ch)
{
  for (VarsLowSpeed.InputBuf[ch].OutPtr = 0; VarsLowSpeed.InputBuf[ch].OutPtr < IOMapLowSpeed.InBuf[ch].BytesToRx; VarsLowSpeed.InputBuf[ch].OutPtr++)
  {
    IOMapLowSpeed.InBuf[ch].Buf[IOMapLowSpeed.InBuf[ch].InPtr] = VarsLowSpeed.InputBuf[ch].Buf[VarsLowSpeed.InputBuf[ch].OutPtr];
    IOMapLowSpeed.InBuf[ch].InPtr++;
    if (IOMapLowSpeed.InBuf[ch].InPtr >= SIZE_OF_LSBUF)
    {
      IOMapLowSpeed.InBuf[ch].InPtr = 0;
    }
    VarsLowSpeed.InputBuf[ch].Buf[VarsLowSpeed.InputBuf[ch].OutPtr] = 0;
  }
}

void      cLowSpeedLoadWriteBuffer(UBYTE ch)
{
  VarsLowSpeed.OutputBuf[ch].OutPtr = 0;
  memcpy(VarsLowSpeed.OutputBuf[ch].Buf, IOMapLowSpeed.OutBuf[ch].Buf, IOMapLowSpeed.OutBuf[ch].InPtr);
  VarsLowSpeed.OutputBuf[ch].InPtr = IOMapLowSpeed.OutBuf[ch].InPtr;
  IOMapLowSpeed.OutBuf[ch].OutPtr = IOMapLowSpeed.OutBuf[ch].InPtr;
/*            
  VarsLowSpeed.OutputBuf[ch].OutPtr = 0;
  for (VarsLowSpeed.OutputBuf[ch].InPtr = 0; VarsLowSpeed.OutputBuf[ch].InPtr < IOMapLowSpeed.OutBuf[ch].InPtr; VarsLowSpeed.OutputBuf[ch].InPtr++)
  {
    VarsLowSpeed.OutputBuf[ch].Buf[VarsLowSpeed.OutputBuf[ch].InPtr] = IOMapLowSpeed.OutBuf[ch].Buf[IOMapLowSpeed.OutBuf[ch].OutPtr];
    IOMapLowSpeed.OutBuf[ch].OutPtr++;
  }
*/
}

void cLowSpeedFinished(UBYTE ch, UBYTE bDone)
{
  IOMapLowSpeed.State = IOMapLowSpeed.State & ~LOWSPEED_CH_NUMBER[ch];
  if (bDone)
    IOMapLowSpeed.ChannelState[ch] = LOWSPEED_IDLE;
  if (IOMapLowSpeed.State == 0)
  {
    dLowSpeedStopTimer();
    VarsLowSpeed.TimerState = TIMER_STOPPED;
  }
}


SBYTE      cLowSpeedFastI2C(UBYTE ch)
{
  cLowSpeedLoadWriteBuffer(ch);
  SBYTE result = dLowSpeedFastI2C(ch, 
                                  *(VarsLowSpeed.OutputBuf[ch].Buf), 
                                  VarsLowSpeed.OutputBuf[ch].Buf+1, 
                                  VarsLowSpeed.OutputBuf[ch].InPtr-1, 
                                  &(IOMapLowSpeed.InBuf[ch].BytesToRx), 
                                  VarsLowSpeed.InputBuf[ch].Buf);
  if (result >= 0)
  {
    // finally copy the data from the VarsLowSpeed buffer into the IOMapLowSpeed buffer
    // and update our channel state and mode
    cLowSpeedCompleteRead(ch);
    IOMapLowSpeed.Mode[ch] = LOWSPEED_DATA_RECEIVED;
//    IOMapLowSpeed.ChannelState[ch] = LOWSPEED_DONE;
    cLowSpeedFinished(ch, TRUE);
  }
  else
  {
    IOMapLowSpeed.ChannelState[ch] = LOWSPEED_ERROR;
    IOMapLowSpeed.ErrorType[ch] = (UBYTE)result;
  }
  return result;
}

void      cLowSpeedCtrl(void)
{
  UBYTE Temp;
  UBYTE ChannelNumber = 0;
    	
  if (IOMapLowSpeed.State != 0)
  {
    for (ChannelNumber = 0; ChannelNumber < NO_OF_LOWSPEED_COM_CHANNEL; ChannelNumber++)
    {
      //Lowspeed com is activated
	    switch (IOMapLowSpeed.ChannelState[ChannelNumber])			
	    {
	      case LOWSPEED_IDLE:
		    {
		    }
		    break;
		
		    case LOWSPEED_INIT:
		    {
		      if ((pMapInput->Inputs[ChannelNumber].SensorType == LOWSPEED) || (pMapInput->Inputs[ChannelNumber].SensorType == LOWSPEED_9V))
          {
            if (IOMapLowSpeed.Speed & (COM_CHANNEL_ONE_FAST << ChannelNumber))
            {
              cLowSpeedFastI2C(ChannelNumber); 
            }
            else
            {
              if (VarsLowSpeed.TimerState == TIMER_STOPPED)
              {
                dLowSpeedStartTimer();
                VarsLowSpeed.TimerState = TIMER_RUNNING;
              }
              IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_LOAD_BUFFER;
              IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_NO_ERROR;
              VarsLowSpeed.ErrorCount[ChannelNumber] = 0;
              dLowSpeedInitPins(ChannelNumber);
            }
          }
          else
          {
            IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
            IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_CH_NOT_READY;
          }
		    }
		    break;
			
		    case LOWSPEED_LOAD_BUFFER:
		    {
		      if ((pMapInput->Inputs[ChannelNumber].SensorType == LOWSPEED) || (pMapInput->Inputs[ChannelNumber].SensorType == LOWSPEED_9V))
          {
            cLowSpeedLoadWriteBuffer(ChannelNumber);
            if (dLowSpeedSendData(ChannelNumber, VarsLowSpeed.OutputBuf[ChannelNumber].Buf, VarsLowSpeed.OutputBuf[ChannelNumber].InPtr))
            {
		          if (IOMapLowSpeed.InBuf[ChannelNumber].BytesToRx != 0)
              {
                dLowSpeedReceiveData(ChannelNumber, &VarsLowSpeed.InputBuf[ChannelNumber].Buf[0], IOMapLowSpeed.InBuf[ChannelNumber].BytesToRx, IOMapLowSpeed.NoRestartOnRead);
                VarsLowSpeed.RxTimeCnt[ChannelNumber] = 0;
              }
              IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_COMMUNICATING;
			        IOMapLowSpeed.Mode[ChannelNumber] = LOWSPEED_TRANSMITTING;
		        }
            else
            {
              IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
              IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_CH_NOT_READY;
            }
          }
          else
          {
            IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
            IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_CH_NOT_READY;
          }
	      }
		    break;
			
		    case LOWSPEED_COMMUNICATING:
		    {
		      if ((pMapInput->Inputs[ChannelNumber].SensorType == LOWSPEED) || (pMapInput->Inputs[ChannelNumber].SensorType == LOWSPEED_9V))
          {
            if (IOMapLowSpeed.Mode[ChannelNumber] == LOWSPEED_TRANSMITTING)
		        {
		          Temp = dLowSpeedComTxStatus(ChannelNumber);			// Returns 0x00 if not done, 0x01 if success, 0xFF if error
		
		          if (Temp == LOWSPEED_COMMUNICATION_SUCCESS)
		          {
		            if (IOMapLowSpeed.InBuf[ChannelNumber].BytesToRx != 0)
                {
                  IOMapLowSpeed.Mode[ChannelNumber] = LOWSPEED_RECEIVING;							    			
                }
                else
                {
                  IOMapLowSpeed.Mode[ChannelNumber] = LOWSPEED_DATA_RECEIVED;
                  IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_DONE;
                }
			        }
			        if (Temp == LOWSPEED_COMMUNICATION_ERROR)
			        {
			          //ERROR in Communication, No ACK received from SLAVE, retry send data 3 times!
		            VarsLowSpeed.ErrorCount[ChannelNumber]++;
			          if (VarsLowSpeed.ErrorCount[ChannelNumber] > MAX_RETRY_TX_COUNT)
			          {
			            IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
                  IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_TX_ERROR;
			          }
			          else
			          {
			            IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_LOAD_BUFFER;
			          }
		          }			
		        }
		        if (IOMapLowSpeed.Mode[ChannelNumber] == LOWSPEED_RECEIVING)
		        {
		          VarsLowSpeed.RxTimeCnt[ChannelNumber]++;
		          if (VarsLowSpeed.RxTimeCnt[ChannelNumber] > LOWSPEED_RX_TIMEOUT)
		          {
		            IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
                IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_RX_ERROR;
		          }
              Temp = dLowSpeedComRxStatus(ChannelNumber);
              if (Temp == LOWSPEED_COMMUNICATION_SUCCESS)
              {
                cLowSpeedCompleteRead(ChannelNumber);
                IOMapLowSpeed.Mode[ChannelNumber] = LOWSPEED_DATA_RECEIVED;
                IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_DONE;
              }
              if (Temp == LOWSPEED_COMMUNICATION_ERROR)
              {
                //There was and error in receiving data from the device
                for (VarsLowSpeed.InputBuf[ChannelNumber].OutPtr = 0; VarsLowSpeed.InputBuf[ChannelNumber].OutPtr < IOMapLowSpeed.InBuf[ChannelNumber].BytesToRx; VarsLowSpeed.InputBuf[ChannelNumber].OutPtr++)
                {
                  VarsLowSpeed.InputBuf[ChannelNumber].Buf[VarsLowSpeed.InputBuf[ChannelNumber].OutPtr] = 0;
                }
                IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
                IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_RX_ERROR;
              }              
		        }		        			
	        }
          else
          {
            IOMapLowSpeed.ChannelState[ChannelNumber] = LOWSPEED_ERROR;
            IOMapLowSpeed.ErrorType[ChannelNumber] = LOWSPEED_CH_NOT_READY;
          }          
        }
	      break;	
			
        case LOWSPEED_ERROR:
        {
          cLowSpeedFinished(ChannelNumber, FALSE);
        }
        break;
        case LOWSPEED_DONE:
	      {
                cLowSpeedFinished(ChannelNumber, TRUE);
	      }
	      break;
	      default:
	   	    break;
	    }
    }
  }	
}

void      cLowSpeedExit(void)
{
  dLowSpeedExit();
}
