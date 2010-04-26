//
// File Description:
// This file contains an alternate implementation of c_cmd for testing purposes.
// It implements the minimal standard interface for the module, and serves as
//  an example of output module control via C code.
//

void      cCmdInit(void* pHeader)
{
  pHeaders        = pHeader;

  IOMapCmd.Awake = TRUE;

  dTimerInit();
  IOMapCmd.Tick = dTimerRead();

  return;
}

//Test: Start at speed 100 when enter is pressed; then progressively ramp down every half second until -100.
void      cCmdCtrl(void)
{
  static UBYTE State = 0;
  static ULONG MyTick = 0;

  if (pMapButton->State[BTN1] & PRESSED_EV)
  {
    pMapButton->State[BTN1] &= ~PRESSED_EV;

    State = 1;
  }

  switch(State)
  {
    case 0:
    {
      //Initialize
      pMapInput->Inputs[0].SensorType = LOWSPEED;
    }
    break;

    case 1:
    {
      if (pMapLowSpeed->ChannelState[0] == LOWSPEED_IDLE)
      {
        pMapLowSpeed->OutBuf[0].InPtr = 0;
        pMapLowSpeed->OutBuf[0].OutPtr = 0;

        pMapLowSpeed->OutBuf[0].Buf[pMapLowSpeed->OutBuf[0].InPtr] = 0x88;   // I2C adress = 1000100X
        pMapLowSpeed->OutBuf[0].InPtr++;
        pMapLowSpeed->OutBuf[0].Buf[pMapLowSpeed->OutBuf[0].InPtr] = 0x00;  // Selecting register to write into
        pMapLowSpeed->OutBuf[0].InPtr++;
        pMapLowSpeed->OutBuf[0].Buf[pMapLowSpeed->OutBuf[0].InPtr] = 0x88;  // Data to set into register => Setting Control register
        pMapLowSpeed->OutBuf[0].InPtr++;

        pMapLowSpeed->InBuf[0].BytesToRx = 0;
        pMapLowSpeed->ChannelState[0]    = LOWSPEED_INIT;
        pMapLowSpeed->State              = COM_CHANNEL_ONE_ACTIVE;

        State = 2;
      }
    }
    break;

    case 2:
    {
      if (pMapLowSpeed->ChannelState[0] == LOWSPEED_IDLE)
      {
        pMapLowSpeed->OutBuf[0].InPtr = 0;
        pMapLowSpeed->OutBuf[0].OutPtr = 0;

        pMapLowSpeed->OutBuf[0].Buf[pMapLowSpeed->OutBuf[0].InPtr] = 0x88;  // I2C adress = 1000100X
        pMapLowSpeed->OutBuf[0].InPtr++;
        pMapLowSpeed->OutBuf[0].Buf[pMapLowSpeed->OutBuf[0].InPtr] = 0x04;  // Start register to read from
        pMapLowSpeed->OutBuf[0].InPtr++;

        pMapLowSpeed->InBuf[0].BytesToRx = 2;                               // Read 2 bytes from I2C unit
        pMapLowSpeed->ChannelState[0]    = LOWSPEED_INIT;
        pMapLowSpeed->State              = COM_CHANNEL_ONE_ACTIVE;

        State = 3;
      }
    }
    break;

    case 3:
    {

    }
    break;

    default:
      break;
  };

  //Busy loop to ensure return on 1ms boundary
  while (IOMapCmd.Tick == dTimerRead());

  IOMapCmd.Tick = dTimerRead();
  MyTick++;

  return;
}

void      cCmdExit(void)
{
  return;
}
