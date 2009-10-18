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

    //Coast both motors
    pMapOutPut->Outputs[0].Mode = MOTORON;
    pMapOutPut->Outputs[1].Mode = MOTORON;

    pMapOutPut->Outputs[0].Speed = 0;
    pMapOutPut->Outputs[0].TachoLimit = 0;
    pMapOutPut->Outputs[0].RunState = MOTOR_RUN_STATE_IDLE;
    pMapOutPut->Outputs[0].RegMode = REGULATION_MODE_IDLE;

    pMapOutPut->Outputs[1].Speed = 0;
    pMapOutPut->Outputs[1].TachoLimit = 0;
    pMapOutPut->Outputs[1].RunState = MOTOR_RUN_STATE_IDLE;
    pMapOutPut->Outputs[1].RegMode = REGULATION_MODE_IDLE;

    pMapOutPut->Outputs[0].Flags = UPDATE_MODE | UPDATE_SPEED;
    pMapOutPut->Outputs[1].Flags = UPDATE_MODE | UPDATE_SPEED;

    //Drop out of ongoing state machine
    State = 255;
  }

  switch(State)
  {
    case 0:
    {
      //Initialize
      pMapOutPut->Outputs[0].Flags = UPDATE_RESET_COUNT;
      pMapOutPut->Outputs[1].Flags = UPDATE_RESET_COUNT;
      pMapOutPut->Outputs[0].RunState = MOTOR_RUN_STATE_IDLE;
      pMapOutPut->Outputs[1].RunState = MOTOR_RUN_STATE_IDLE;

      State++;
    }
    break;

    case 1:
    {
      //Kick off further states only if Enter button is pressed
      if ((pMapButton->State[BTN4] & PRESSED_EV))
      {
        //Clear pressed event so UI doesn't re-use it.
        pMapButton->State[BTN4] &= ~PRESSED_EV;

        pMapOutPut->Outputs[0].Mode = MOTORON | BRAKE | REGULATED;
        pMapOutPut->Outputs[1].Mode = MOTORON | BRAKE | REGULATED;

        pMapOutPut->Outputs[0].Speed = 50;
        pMapOutPut->Outputs[0].TachoLimit = 1152;
        pMapOutPut->Outputs[0].RunState = MOTOR_RUN_STATE_RUNNING;
        pMapOutPut->Outputs[0].SyncTurnParameter = 25;
        //pMapOutPut->Outputs[0].RegMode = REGULATION_MODE_MOTOR_SPEED;
        pMapOutPut->Outputs[0].RegMode = REGULATION_MODE_MOTOR_SYNC;

        pMapOutPut->Outputs[1].Speed = 50;
        pMapOutPut->Outputs[1].TachoLimit = 1152;
        pMapOutPut->Outputs[1].RunState = MOTOR_RUN_STATE_RUNNING;
        //pMapOutPut->Outputs[1].SyncTurnParameter = -7;
        //pMapOutPut->Outputs[1].RegMode = REGULATION_MODE_MOTOR_SPEED;
        pMapOutPut->Outputs[1].RegMode = REGULATION_MODE_MOTOR_SYNC;

        pMapOutPut->Outputs[0].Flags = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;
        pMapOutPut->Outputs[1].Flags = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;

        State++;
      }
    }
    break;

    case 2:
    {
      if (pMapOutPut->Outputs[0].RunState == MOTOR_RUN_STATE_IDLE)
      {
        pMapOutPut->Outputs[0].Mode = MOTORON;
        pMapOutPut->Outputs[1].Mode = MOTORON;

        pMapOutPut->Outputs[0].Speed = 0;
        pMapOutPut->Outputs[0].TachoLimit = 0;
        pMapOutPut->Outputs[0].RunState = MOTOR_RUN_STATE_IDLE;
        pMapOutPut->Outputs[0].RegMode = REGULATION_MODE_IDLE;

        pMapOutPut->Outputs[1].Speed = 0;
        pMapOutPut->Outputs[1].TachoLimit = 0;
        pMapOutPut->Outputs[1].RunState = MOTOR_RUN_STATE_IDLE;
        pMapOutPut->Outputs[1].RegMode = REGULATION_MODE_IDLE;

        pMapOutPut->Outputs[0].Flags = UPDATE_MODE | UPDATE_SPEED;
        pMapOutPut->Outputs[1].Flags = UPDATE_MODE | UPDATE_SPEED;
        State++;
      }
    }
    break;

    case 3:
    {
        pMapOutPut->Outputs[0].Mode = MOTORON | BRAKE | REGULATED;
        pMapOutPut->Outputs[1].Mode = MOTORON | BRAKE | REGULATED;

        pMapOutPut->Outputs[0].Speed = 50;
        pMapOutPut->Outputs[0].TachoLimit = 1152;
        pMapOutPut->Outputs[0].RunState = MOTOR_RUN_STATE_RUNNING;
        //pMapOutPut->Outputs[0].SyncTurnParameter = 5;
        //pMapOutPut->Outputs[0].RegMode = REGULATION_MODE_MOTOR_SPEED;
        pMapOutPut->Outputs[0].RegMode = REGULATION_MODE_MOTOR_SYNC;

        pMapOutPut->Outputs[1].Speed = 50;
        pMapOutPut->Outputs[1].TachoLimit = 1152;
        pMapOutPut->Outputs[1].RunState = MOTOR_RUN_STATE_RUNNING;
        //pMapOutPut->Outputs[1].RegMode = REGULATION_MODE_MOTOR_SPEED;
        pMapOutPut->Outputs[1].RegMode = REGULATION_MODE_MOTOR_SYNC;

        pMapOutPut->Outputs[0].Flags = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;
        pMapOutPut->Outputs[1].Flags = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;

        State = 2;
    }
    break;

    default:
      break;
  };

  //Busy loop to ensure return on 1ms boundary
  BUSY_WAIT_NEXT_MS;

  IOMapCmd.Tick = dTimerRead();
  MyTick++;

  return;
}

void      cCmdExit(void)
{
  return;
}
