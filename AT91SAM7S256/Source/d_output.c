//
// Date init       14.12.2004
//
// Revision date   $Date:: 3-02-09 14:46                                     $
//
// Filename        $Workfile:: d_output.c                                    $
//
// Version         $Revision:: 2                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_outp $
//
// Platform        C
//

#include  "stdconst.h"
#include  "m_sched.h"
#include  "d_output.h"
#include  "d_output.r"

#define MAXIMUM_SPEED_FW         100
#define MAXIMUM_SPEED_RW         -100

#define INPUT_SCALE_FACTOR       100

#define MAX_COUNT_TO_RUN         10000000

#define REG_MAX_VALUE            100

#define RAMP_TIME_INTERVAL       25           // Measured in 1 mS => 25 mS interval

#define RAMPDOWN_STATE_RAMPDOWN  0
#define RAMPDOWN_STATE_CONTINIUE 1

#define COAST_MOTOR_MODE         0

void dOutputRampDownSynch(UBYTE MotorNr);
SLONG dOutputBound(SLONG In, SLONG Limit);

typedef struct
{
  SBYTE MotorSetSpeed;                        // Motor setpoint in speed
  SBYTE MotorTargetSpeed;                     // Speed order for the movement
  SBYTE MotorActualSpeed;                     // Actual speed for motor (Calculated within the PID regulation)
  SBYTE TurnParameter;                        // Tell the turning parameter used
  UBYTE RegPParameter;                        // Current P parameter used within the regulation
  UBYTE RegIParameter;                        // Current I parameter used within the regulation
  UBYTE RegDParameter;                        // Current D parameter used within the regulation
  UBYTE RegulationTimeCount;                  // Time counter used to evaluate when the regulation should run again (100 mS)
  UBYTE MotorRunState;                        // Hold current motor state (Ramp-up, Running, Ramp-Down, Idle)
  UBYTE RegulationMode;                       // Hold current regulation mode (Position control, Synchronization mode)
  UBYTE MotorOverloaded;                      // Set if the motor speed in regulation is calculated to be above maximum
  UBYTE MotorRunForever;                      // Tell that the motor is set to run forever
  UWORD MotorRampDownCount;                   // Counter to tell if the ramp-down can reach it gaol and therefor need some additional help
  SWORD MotorRampDownIncrement;               // Tell the number of count between each speed adjustment during Ramp-Down
  UWORD MotorRampUpCount;                     // Used to speedup Ramp-Up if position regulation is not enabled
  SWORD MotorRampUpIncrement;                 // Tell the number of count between each speed adjustment during Ramp-up
  SWORD AccError;                             // Accumulated Error, used within the integrator of the PID regulation
  SWORD OldPositionError;                     // Used within position regulation
  SLONG DeltaCaptureCount;                    // Counts within last regulation time-periode
  SLONG CurrentCaptureCount;                  // Total counts since motor counts has been reset
  SLONG MotorTachoCountToRun;                 // Holds number of counts to run. 0 = Run forever
  SLONG MotorBlockTachoCount;                 // Hold CaptureCount for current movement
  SLONG MotorRampTachoCountOld;               // Used to hold old position during Ramp-Up
  SLONG MotorRampTachoCountStart;             // Used to hold position when Ramp-up started
  SLONG RotationCaptureCount;                 // Counter for additional rotation counter
}MOTORDATA;

typedef struct
{
  SLONG SyncTachoDif;
  SLONG SyncTurnParameter;
  SWORD SyncOldError;
  SWORD SyncAccError;
}SYNCMOTORDATA;

static    MOTORDATA         MotorData[3];
static    SYNCMOTORDATA     SyncData;
static    UBYTE             RegulationTime;

void      dOutputInit(void)
{
  UBYTE Temp;

  OUTPUTInit;
  ENABLECaptureMotorA;
  ENABLECaptureMotorB;
  ENABLECaptureMotorC;

  for (Temp = 0; Temp < 3; Temp++)
  {
    MOTORDATA * pMD = &(MotorData[Temp]);
    pMD->MotorSetSpeed = 0;
    pMD->MotorTargetSpeed = 0;
    pMD->MotorActualSpeed = 0;
    pMD->MotorRampUpCount = 0;
    pMD->MotorRampDownCount = 0;
    pMD->MotorRunState = 0;
    pMD->MotorTachoCountToRun = 0;
    pMD->MotorRunForever = 1;
    pMD->AccError = 0;
    pMD->RegulationTimeCount = 0;
    pMD->RegPParameter = DEFAULT_P_GAIN_FACTOR;
    pMD->RegIParameter = DEFAULT_I_GAIN_FACTOR;
    pMD->RegDParameter = DEFAULT_D_GAIN_FACTOR;
    pMD->RegulationMode = 0;
    pMD->MotorOverloaded = 0;
    INSERTMode(Temp, COAST_MOTOR_MODE);
    INSERTSpeed(Temp, pMD->MotorSetSpeed);
  }
}

/* This function is called every 1 mS and will go through all the motors and there dependencies */
/* Actual motor speed is only passed (updated) to the AVR controller form this function */
/* DeltacaptureCount used to count number of Tachocount within last 100 mS. Used with position control regulation */
/* CurrentCaptureCount used to tell total current position. Used to tell when movement has been obtained */
/* MotorBlockTachoCount tell current position within current movement. Reset when a new block is started from the VM */
/* RotationCaptureCount is additional counter for the rotationsensor. Uses it own value so it does conflict with other CaptureCount */
void dOutputCtrl(void)
{
  UBYTE MotorNr;
  SLONG NewTachoCount[3];

  TACHOCaptureReadResetAll(NewTachoCount[MOTOR_A], NewTachoCount[MOTOR_B], NewTachoCount[MOTOR_C]);

  for (MotorNr = 0; MotorNr < 3; MotorNr++)
  {
    MOTORDATA * pMD = &(MotorData[MotorNr]);
    pMD->DeltaCaptureCount += NewTachoCount[MotorNr];
    pMD->CurrentCaptureCount += NewTachoCount[MotorNr];
    pMD->MotorBlockTachoCount += NewTachoCount[MotorNr];
    pMD->RotationCaptureCount += NewTachoCount[MotorNr];
    pMD->RegulationTimeCount++;

    if (pMD->MotorRunState == MOTOR_RUN_STATE_RAMPUP)
    {
      dOutputRampUpFunction(MotorNr);
    }
    if (pMD->MotorRunState == MOTOR_RUN_STATE_RAMPDOWN)
    {
      dOutputRampDownFunction(MotorNr);
    }
    if (pMD->MotorRunState == MOTOR_RUN_STATE_RUNNING)
    {
      dOutputTachoLimitControl(MotorNr);
    }
    if (pMD->MotorRunState == MOTOR_RUN_STATE_IDLE)
    {
      dOutputMotorIdleControl(MotorNr);
    }
    if (pMD->MotorRunState == MOTOR_RUN_STATE_HOLD)
    {
      pMD->MotorSetSpeed = 0;
      pMD->MotorActualSpeed = 0;
      pMD->MotorTargetSpeed = 0;
      pMD->RegulationTimeCount = 0;
      pMD->DeltaCaptureCount = 0;
      pMD->MotorRunState = MOTOR_RUN_STATE_RUNNING;

    }
    if (pMD->RegulationTimeCount > RegulationTime)
    {
      pMD->RegulationTimeCount = 0;
      dOutputRegulateMotor(MotorNr);
      pMD->DeltaCaptureCount = 0;
    }
  }
  INSERTSpeed(MOTOR_A, MotorData[MOTOR_A].MotorActualSpeed);
  INSERTSpeed(MOTOR_B, MotorData[MOTOR_B].MotorActualSpeed);
  INSERTSpeed(MOTOR_C, MotorData[MOTOR_C].MotorActualSpeed);
}

void      dOutputExit(void)
{
  OUTPUTExit;
}

/* Called eveyr 1 mS */
/* Data mapping for controller (IO-Map is updated with these values) */
void dOutputGetMotorParameters(UBYTE *CurrentMotorSpeed, SLONG *TachoCount, SLONG *BlockTachoCount, UBYTE *RunState, UBYTE *MotorOverloaded, SLONG *RotationCount)
{
  UBYTE Tmp;

  for (Tmp = 0; Tmp < 3; Tmp++)
  {
    MOTORDATA * pMD = &(MotorData[Tmp]);
    CurrentMotorSpeed[Tmp] = pMD->MotorActualSpeed;
    TachoCount[Tmp] = pMD->CurrentCaptureCount;
    BlockTachoCount[Tmp] = pMD->MotorBlockTachoCount;
    RotationCount[Tmp] = pMD->RotationCaptureCount;
    RunState[Tmp] = pMD->MotorRunState;
    MotorOverloaded[Tmp] = pMD->MotorOverloaded;
  }
}

void dOutputSetMode(UBYTE MotorNr, UBYTE Mode)     //Set motor mode (break, Float)
{
  INSERTMode(MotorNr, Mode);
}

/* Update the regulation state for the motor */
/* Need to reset regulation parameter depending on current status of the motor */
/* AccError & OldPositionError used for position regulation and Sync Parameter are used for synchronization regulation */
void dOutputEnableRegulation(UBYTE MotorNr, UBYTE RegulationMode)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  pMD->RegulationMode = RegulationMode;

  if ((pMD->RegulationMode & REGSTATE_REGULATED) && (pMD->MotorSetSpeed == 0) && (pMD->MotorRunState != MOTOR_RUN_STATE_RAMPDOWN))
  {
    pMD->AccError = 0;
    pMD->OldPositionError = 0;
  }

  if (pMD->RegulationMode & REGSTATE_SYNCHRONE)
  {
    if (((pMD->MotorActualSpeed == 0) || (pMD->TurnParameter != 0) || (pMD->TurnParameter == 0)) && (pMD->MotorRunState != MOTOR_RUN_STATE_RAMPDOWN))
    {
      SyncData.SyncTachoDif = 0;

      SyncData.SyncAccError = 0;
      SyncData.SyncOldError = 0;
      SyncData.SyncTurnParameter = 0;
    }
  }
}

/* Disable current regulation if enabled */
void dOutputDisableRegulation(UBYTE MotorNr)
{
  MotorData[MotorNr].RegulationMode = REGSTATE_IDLE;
}

/* Calling this function with reset count which tell current position and which is used to tell if the wanted position is obtained */
/* Calling this function will reset current movement of the motor if it is running */
void dOutputResetTachoLimit(UBYTE MotorNr)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  pMD->CurrentCaptureCount = 0;
  pMD->MotorTachoCountToRun = 0;

  if (pMD->RegulationMode & REGSTATE_SYNCHRONE)
  {
    dOutputResetSyncMotors(MotorNr);
  }

  if (pMD->MotorRunForever == 1)
  {
    pMD->MotorRunForever = 0;                   // To ensure that we get the same functionality for all combination on motor durations
  }
}

/* MotorBlockTachoCount tells current position in current movement. */
/* Used within the synchronization to compare current motor position. Reset on every new movement from the VM */
void dOutputResetBlockTachoLimit(UBYTE MotorNr)
{
  MotorData[MotorNr].MotorBlockTachoCount = 0;
}

/* Additional counter add to help the VM application keep track of number of rotation for the rotation sensor */
/* This values can be reset independtly from the other tacho count values used with regulation and position control */
void dOutputResetRotationCaptureCount(UBYTE MotorNr)
{
  MotorData[MotorNr].RotationCaptureCount = 0;
}

/* Can be used to set new PID values */
void dOutputSetPIDParameters(UBYTE MotorNr, UBYTE NewRegPParameter, UBYTE NewRegIParameter, UBYTE NewRegDParameter)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  pMD->RegPParameter = NewRegPParameter;
  pMD->RegIParameter = NewRegIParameter;
  pMD->RegDParameter = NewRegDParameter;
}

/* Set new regulation time */
void dOutputSetRegulationTime(UBYTE NewRegulationTime)
{
  RegulationTime = NewRegulationTime;
}

/* Called to set TachoCountToRun which is used for position control for the model */
/* Must be called before motor start */
/* TachoCountToRun is calculated as a signed value */
void dOutputSetTachoLimit(UBYTE MotorNr, ULONG BlockTachoCntToTravel)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if (pMD->RegulationMode & REGSTATE_POSITION)
  {
    pMD->MotorRunForever = 0;
    pMD->MotorTachoCountToRun = BlockTachoCntToTravel;
  }
  else if (BlockTachoCntToTravel == 0)
  {
    pMD->MotorRunForever = 1;
  }
  else
  {
    pMD->MotorRunForever = 0;

    if (pMD->MotorSetSpeed == 0)
    {
      if (pMD->MotorTargetSpeed > 0)
      {
        pMD->MotorTachoCountToRun += BlockTachoCntToTravel;
      }
      else
      {
        pMD->MotorTachoCountToRun -= BlockTachoCntToTravel;
      }
    }
    else
    {
      if (pMD->MotorSetSpeed > 0)
      {
        pMD->MotorTachoCountToRun += BlockTachoCntToTravel;
      }
      else
      {
        pMD->MotorTachoCountToRun -= BlockTachoCntToTravel;
      }
    }
  }
}

/* This function is used for setting up the motor mode and motor speed */
void dOutputSetSpeed (UBYTE MotorNr, UBYTE NewMotorRunState, SBYTE Speed, SBYTE NewTurnParameter)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if ((pMD->MotorSetSpeed != Speed) || (pMD->MotorRunState != NewMotorRunState) ||
      (NewMotorRunState == MOTOR_RUN_STATE_IDLE) || (pMD->TurnParameter != NewTurnParameter))
  {
    if (pMD->MotorTargetSpeed == 0)
    {
      pMD->AccError = 0;
      pMD->OldPositionError = 0;
      pMD->RegulationTimeCount = 0;
      pMD->DeltaCaptureCount = 0;
      TACHOCountReset(MotorNr);
    }
    switch (NewMotorRunState)
    {
      case MOTOR_RUN_STATE_IDLE:
      {
        //pMD->MotorSetSpeed = 0;
        //pMD->MotorTargetSpeed = 0;
        //pMD->TurnParameter = 0;
        pMD->RegulationMode = REGSTATE_IDLE;
      }
      break;

      case MOTOR_RUN_STATE_RAMPUP:
      {
        if (pMD->MotorSetSpeed == 0)
        {
          pMD->MotorSetSpeed = Speed;
          pMD->TurnParameter = NewTurnParameter;
          pMD->MotorRampUpIncrement = 0;
          pMD->MotorRampTachoCountStart = pMD->CurrentCaptureCount;
          pMD->MotorRampUpCount = 0;
        }
        else
        {
          if (Speed > 0)
          {
            if (pMD->MotorSetSpeed >= Speed)
            {
              NewMotorRunState = MOTOR_RUN_STATE_RUNNING;
            }
            else
            {
              pMD->MotorSetSpeed = Speed;
              pMD->TurnParameter = NewTurnParameter;
              pMD->MotorRampUpIncrement = 0;
              pMD->MotorRampTachoCountStart = pMD->CurrentCaptureCount;
              pMD->MotorRampUpCount = 0;
            }
          }
          else
          {
            if (pMD->MotorSetSpeed <= Speed)
            {
              NewMotorRunState = MOTOR_RUN_STATE_RUNNING;
            }
            else
            {
              pMD->MotorSetSpeed = Speed;
              pMD->TurnParameter = NewTurnParameter;
              pMD->MotorRampUpIncrement = 0;
              pMD->MotorRampTachoCountStart = pMD->CurrentCaptureCount;
              pMD->MotorRampUpCount = 0;
            }
          }
        }
      }
      break;

      case MOTOR_RUN_STATE_RUNNING:
      {
        pMD->MotorSetSpeed = Speed;
        pMD->MotorTargetSpeed = Speed;
        pMD->TurnParameter = NewTurnParameter;

        if (pMD->MotorSetSpeed == 0)
        {
          NewMotorRunState = MOTOR_RUN_STATE_HOLD;
        }
      }
      break;

      case MOTOR_RUN_STATE_RAMPDOWN:
      {
        if (pMD->MotorTargetSpeed >= 0)
        {
          if (pMD->MotorSetSpeed <= Speed)
          {
            NewMotorRunState = MOTOR_RUN_STATE_RUNNING;
          }
          else
          {
            pMD->MotorSetSpeed = Speed;
            pMD->TurnParameter = NewTurnParameter;
            pMD->MotorRampDownIncrement = 0;
            pMD->MotorRampTachoCountStart = pMD->CurrentCaptureCount;
            pMD->MotorRampDownCount = 0;
          }
        }
        else
        {
          if (pMD->MotorSetSpeed >= Speed)
          {
            NewMotorRunState = MOTOR_RUN_STATE_RUNNING;
          }
          else
          {
            pMD->MotorSetSpeed = Speed;
            pMD->TurnParameter = NewTurnParameter;
            pMD->MotorRampDownIncrement = 0;
            pMD->MotorRampTachoCountStart = pMD->CurrentCaptureCount;
            pMD->MotorRampDownCount = 0;
          }
        }
      }
      break;
    }
    pMD->MotorRunState = NewMotorRunState;
    pMD->MotorOverloaded = 0;
  }
}

/* Function used for controlling the Ramp-up periode */
/* Ramp-up is done with 1 increment in speed every X number of TachoCount, where X depend on duration of the periode and the wanted speed */
void dOutputRampUpFunction(UBYTE MotorNr)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if (pMD->MotorTargetSpeed == 0)
  {
    if (pMD->MotorSetSpeed > 0)
    {
      pMD->MotorTargetSpeed = MIN_MOVEMENT_POWER;
    }
    else
    {
      pMD->MotorTargetSpeed = -MIN_MOVEMENT_POWER;
    }
  }
  else
  {
    if (pMD->MotorRampUpIncrement == 0)
    {
      SWORD delta = (SWORD)((pMD->MotorTachoCountToRun - pMD->MotorRampTachoCountStart) / (pMD->MotorSetSpeed - pMD->MotorTargetSpeed));
      if (pMD->MotorSetSpeed > 0)
      {
        pMD->MotorRampUpIncrement = delta;
      }
      else
      {
        pMD->MotorRampUpIncrement = -delta;
      }
      pMD->MotorRampTachoCountOld = pMD->CurrentCaptureCount;
    }
    if (pMD->MotorSetSpeed > 0)
    {
      if (pMD->CurrentCaptureCount > (pMD->MotorRampTachoCountOld + pMD->MotorRampUpIncrement))
      {
        pMD->MotorTargetSpeed += 1;
        pMD->MotorRampTachoCountOld = pMD->CurrentCaptureCount;
        pMD->MotorRampUpCount = 0;
      }
      else
      {
        if (!(pMD->RegulationMode & REGSTATE_REGULATED))
        {
          pMD->MotorRampUpCount++;
          if (pMD->MotorRampUpCount > 100)
          {
            pMD->MotorRampUpCount = 0;
            pMD->MotorTargetSpeed++;
          }
        }
      }
    }
    else
    {
      if (pMD->CurrentCaptureCount < (pMD->MotorRampTachoCountOld + pMD->MotorRampUpIncrement))
      {
        pMD->MotorTargetSpeed -= 1;
        pMD->MotorRampTachoCountOld = pMD->CurrentCaptureCount;
        pMD->MotorRampUpCount = 0;
      }
      else
      {
        if (!(pMD->RegulationMode & REGSTATE_REGULATED))
        {
          pMD->MotorRampUpCount++;
          if (pMD->MotorRampUpCount > 100)
          {
            pMD->MotorRampUpCount = 0;
            pMD->MotorTargetSpeed--;
          }
        }
      }
    }
  }
  if (pMD->MotorSetSpeed > 0)
  {
    if ((pMD->CurrentCaptureCount - pMD->MotorRampTachoCountStart) >= (pMD->MotorTachoCountToRun - pMD->MotorRampTachoCountStart))
    {
      pMD->MotorTargetSpeed = pMD->MotorSetSpeed;
      pMD->MotorRunState = MOTOR_RUN_STATE_IDLE;
    }
  }
  else
  {
    if ((pMD->CurrentCaptureCount + pMD->MotorRampTachoCountStart) <= (pMD->MotorTachoCountToRun + pMD->MotorRampTachoCountStart))
    {
      pMD->MotorTargetSpeed = pMD->MotorSetSpeed;
      pMD->MotorRunState = MOTOR_RUN_STATE_IDLE;
    }
  }
  if (pMD->MotorSetSpeed > 0)
  {
    if (pMD->MotorTargetSpeed > pMD->MotorSetSpeed)
    {
      pMD->MotorTargetSpeed = pMD->MotorSetSpeed;
    }
  }
  else
  {
    if (pMD->MotorTargetSpeed < pMD->MotorSetSpeed)
    {
      pMD->MotorTargetSpeed = pMD->MotorSetSpeed;
    }
  }
  if (pMD->RegulationMode == REGSTATE_IDLE)
  {
    pMD->MotorActualSpeed = pMD->MotorTargetSpeed;
  }
}

/* Function used for controlling the Ramp-down periode */
/* Ramp-down is done with 1 decrement in speed every X number of TachoCount, where X depend on duration of the periode and the wanted speed */
void dOutputRampDownFunction(UBYTE MotorNr)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if (pMD->MotorRampDownIncrement == 0)
  {
    if (pMD->MotorTargetSpeed > 0)
    {
      if ((pMD->MotorTargetSpeed > MIN_MOVEMENT_POWER) && (pMD->MotorSetSpeed == 0))
      {
        pMD->MotorRampDownIncrement = ((pMD->MotorTachoCountToRun - pMD->CurrentCaptureCount) / ((pMD->MotorTargetSpeed - pMD->MotorSetSpeed) - MIN_MOVEMENT_POWER));
      }
      else
      {
        pMD->MotorRampDownIncrement = ((pMD->MotorTachoCountToRun - pMD->CurrentCaptureCount) / (pMD->MotorTargetSpeed - pMD->MotorSetSpeed));
      }
    }
    else
    {
      if ((pMD->MotorTargetSpeed < -MIN_MOVEMENT_POWER) && (pMD->MotorSetSpeed == 0))
      {
        pMD->MotorRampDownIncrement = (-((pMD->MotorTachoCountToRun - pMD->CurrentCaptureCount) / ((pMD->MotorTargetSpeed - pMD->MotorSetSpeed) + MIN_MOVEMENT_POWER)));
      }
      else
      {
        pMD->MotorRampDownIncrement = (-((pMD->MotorTachoCountToRun - pMD->CurrentCaptureCount) / (pMD->MotorTargetSpeed - pMD->MotorSetSpeed)));
      }
    }
    pMD->MotorRampTachoCountOld = pMD->CurrentCaptureCount;
  }
  if (pMD->MotorTargetSpeed > 0)
  {
    if (pMD->CurrentCaptureCount > (pMD->MotorRampTachoCountOld + (SLONG)pMD->MotorRampDownIncrement))
    {
      pMD->MotorTargetSpeed--;
      if (pMD->MotorTargetSpeed < MIN_MOVEMENT_POWER)
      {
        pMD->MotorTargetSpeed = MIN_MOVEMENT_POWER;
      }
      pMD->MotorRampTachoCountOld = pMD->CurrentCaptureCount;
      pMD->MotorRampDownCount = 0;
      dOutputRampDownSynch(MotorNr);
    }
    else
    {
      if (!(pMD->RegulationMode & REGSTATE_REGULATED))
      {
        pMD->MotorRampDownCount++;
        if (pMD->MotorRampDownCount > (UWORD)(30 * pMD->MotorRampDownIncrement))
        {
          pMD->MotorRampDownCount = (UWORD)(20 * pMD->MotorRampDownIncrement);
          pMD->MotorTargetSpeed++;
        }
      }
    }
  }
  else
  {
    if (pMD->CurrentCaptureCount < (pMD->MotorRampTachoCountOld + (SLONG)pMD->MotorRampDownIncrement))
    {
      pMD->MotorTargetSpeed++;
      if (pMD->MotorTargetSpeed > -MIN_MOVEMENT_POWER)
      {
        pMD->MotorTargetSpeed = -MIN_MOVEMENT_POWER;
      }
      pMD->MotorRampTachoCountOld = pMD->CurrentCaptureCount;
      pMD->MotorRampDownCount = 0;
      dOutputRampDownSynch(MotorNr);
    }
    else
    {
      if (!(pMD->RegulationMode & REGSTATE_REGULATED))
      {
        pMD->MotorRampDownCount++;
        if (pMD->MotorRampDownCount > (UWORD)(30 * (-pMD->MotorRampDownIncrement)))
        {
          pMD->MotorRampDownCount = (UWORD)(20 * (-pMD->MotorRampDownIncrement));
          pMD->MotorTargetSpeed--;
        }
      }
    }
  }
  if ((pMD->RegulationMode & REGSTATE_SYNCHRONE) && (pMD->TurnParameter != 0))
  {
    dOutputSyncTachoLimitControl(MotorNr);
    if (pMD->MotorRunState == MOTOR_RUN_STATE_IDLE)
    {
      dOutputMotorReachedTachoLimit(MotorNr);
    }
  }
  else
  {
    if (pMD->MotorTargetSpeed > 0)
    {
      if (pMD->CurrentCaptureCount >= pMD->MotorTachoCountToRun)
      {
        dOutputMotorReachedTachoLimit(MotorNr);
      }
    }
    else
    {
      if (pMD->CurrentCaptureCount <= pMD->MotorTachoCountToRun)
      {
        dOutputMotorReachedTachoLimit(MotorNr);
      }
    }
  }
  if (pMD->RegulationMode == REGSTATE_IDLE)
  {
    pMD->MotorActualSpeed = pMD->MotorTargetSpeed;
  }
}

/* Function used to tell whether the wanted position is obtained */
void dOutputTachoLimitControl(UBYTE MotorNr)
{
  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if (pMD->RegulationMode & REGSTATE_POSITION)
  {
    /* No limit when doing absolute position regulation. */
    return;
  }
  if (pMD->MotorRunForever == 0)
  {
    if (pMD->RegulationMode & REGSTATE_SYNCHRONE)
    {
      dOutputSyncTachoLimitControl(MotorNr);
    }
    else
    {
      if (pMD->MotorSetSpeed > 0)
      {
        if ((pMD->CurrentCaptureCount >= pMD->MotorTachoCountToRun))
        {
          pMD->MotorRunState = MOTOR_RUN_STATE_IDLE;
          pMD->RegulationMode = REGSTATE_IDLE;
        }
      }
      else
      {
        if (pMD->MotorSetSpeed < 0)
        {
          if (pMD->CurrentCaptureCount <= pMD->MotorTachoCountToRun)
          {
            pMD->MotorRunState = MOTOR_RUN_STATE_IDLE;
            pMD->RegulationMode = REGSTATE_IDLE;
          }
        }
      }
    }
  }
  else
  {
    if (pMD->CurrentCaptureCount > MAX_COUNT_TO_RUN)
    {
      pMD->CurrentCaptureCount = 0;
    }
    if (pMD->MotorTargetSpeed != 0)
    {
      pMD->MotorTachoCountToRun = pMD->CurrentCaptureCount;
    }
  }
  if (pMD->RegulationMode == REGSTATE_IDLE)
  {
    pMD->MotorActualSpeed = pMD->MotorTargetSpeed;
  }
}

/* Function used to decrease speed slowly when the motor is set to idle */
void dOutputMotorIdleControl(UBYTE MotorNr)
{
  INSERTMode(MotorNr, COAST_MOTOR_MODE);

  MOTORDATA * pMD = &(MotorData[MotorNr]);

  if (pMD->MotorActualSpeed != 0)
  {
    if (pMD->MotorActualSpeed > 0)
    {
      pMD->MotorActualSpeed--;
    }
    else
    {
      pMD->MotorActualSpeed++;
    }
  }

  if (pMD->MotorTargetSpeed != 0)
  {
    if (pMD->MotorTargetSpeed > 0)
    {
      pMD->MotorTargetSpeed--;
    }
    else
    {
      pMD->MotorTargetSpeed++;
    }
  }

  if (pMD->MotorSetSpeed != 0)
  {
    if (pMD->MotorSetSpeed > 0)
    {
      pMD->MotorSetSpeed--;
    }
    else
    {
      pMD->MotorSetSpeed++;
    }
  }
}

/* Check if Value is between [-Limit:Limit], and change it if it is not the case. */
SLONG dOutputBound(SLONG Value, SLONG Limit)
{
  if (Value > Limit)
    return Limit;
  else if (Value < -Limit)
    return -Limit;
  else
    return Value;
}

/* Function called to evaluate which regulation princip that need to run and which MotorNr to use (I.E.: Which motors are synched together)*/
void dOutputRegulateMotor(UBYTE MotorNr)
{
  UBYTE SyncMotorOne;
  UBYTE SyncMotorTwo;

  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if (pMD->RegulationMode & REGSTATE_POSITION)
  {
    dOutputAbsolutePositionRegulation(MotorNr);
  }
  else if (pMD->RegulationMode & REGSTATE_REGULATED)
  {
    dOutputCalculateMotorPosition(MotorNr);
  }
  else
  {
    if (pMD->RegulationMode & REGSTATE_SYNCHRONE)
    {
      dOutputMotorSyncStatus(MotorNr, &SyncMotorOne, &SyncMotorTwo);

      if ((SyncMotorOne != 0xFF) &&(SyncMotorTwo != 0xFF))
      {
        dOutputSyncMotorPosition(SyncMotorOne, SyncMotorTwo);
      }
    }
  }
}

/* Absolute position regulation. */
void dOutputAbsolutePositionRegulation(UBYTE MotorNr)
{
  /* Inputs:
   *  - CurrentCaptureCount: current motor position.
   *  - MotorTachoCountToRun: wanted position.
   *
   *  - RegPParameter, RegIParameter, RegDParameter: PID parameters.
   *
   * Outputs:
   *  - MotorActualSpeed: power to be applied to motor.
   *  - MotorOverloaded: set if MotorActualSpeed reached maximum.
   *
   *  - OldPositionError: remember last error, used for D part.
   *  - AccError: Accumulated error, used for I part.
   */
  SLONG PositionError;
  SLONG PValue, DValue, IValue, TotalRegValue;

  MOTORDATA *pMD = &MotorData[MotorNr];

  PositionError = pMD->MotorTachoCountToRun - pMD->CurrentCaptureCount;
  PositionError = dOutputBound (PositionError, 32000);

  PValue = PositionError * (pMD->RegPParameter/REG_CONST_DIV);
  PValue = dOutputBound (PValue, REG_MAX_VALUE);

  DValue = (PositionError - pMD->OldPositionError) * (pMD->RegDParameter/REG_CONST_DIV);
  pMD->OldPositionError = PositionError;

  pMD->AccError = (pMD->AccError * 3 + PositionError) / 4;
  pMD->AccError = dOutputBound (pMD->AccError, 800);

  IValue = pMD->AccError * (pMD->RegIParameter/REG_CONST_DIV);
  IValue = dOutputBound (IValue, REG_MAX_VALUE);

  TotalRegValue = (PValue + IValue + DValue) / 2;
  if (TotalRegValue > MAXIMUM_SPEED_FW)
  {
    TotalRegValue = MAXIMUM_SPEED_FW;
    pMD->MotorOverloaded = 1;
  }
  else if (TotalRegValue < MAXIMUM_SPEED_RW)
  {
    TotalRegValue = MAXIMUM_SPEED_RW;
    pMD->MotorOverloaded = 1;
  }

  pMD->MotorActualSpeed = TotalRegValue;
}

/* Regulation function used when Position regulation is enabled */
/* The regulation form only control one motor at a time */
void dOutputCalculateMotorPosition(UBYTE MotorNr)
{
  SLONG PositionError;
  SLONG PValue;
  SLONG IValue;
  SLONG DValue;
  SLONG TotalRegValue;
  SLONG NewSpeedCount = 0;

  MOTORDATA * pMD = &(MotorData[MotorNr]);
  NewSpeedCount = (SWORD)((pMD->MotorTargetSpeed * MAX_CAPTURE_COUNT)/INPUT_SCALE_FACTOR);

  PositionError = (pMD->OldPositionError - pMD->DeltaCaptureCount) + NewSpeedCount;

  //Overflow control on PositionError
  PositionError = dOutputBound (PositionError, 32000);

  PValue = PositionError * (pMD->RegPParameter/REG_CONST_DIV);
  PValue = dOutputBound (PValue, REG_MAX_VALUE);

  DValue = (PositionError - pMD->OldPositionError) * (pMD->RegDParameter/REG_CONST_DIV);
  pMD->OldPositionError = (SWORD)PositionError;

  pMD->AccError = (pMD->AccError * 3) + (SWORD)PositionError;
  pMD->AccError = pMD->AccError / 4;
  pMD->AccError = dOutputBound (pMD->AccError, 800);

  IValue = pMD->AccError * (pMD->RegIParameter/REG_CONST_DIV);
  IValue = dOutputBound (IValue, REG_MAX_VALUE);

  TotalRegValue = (PValue + IValue + DValue) / 2;

  if (TotalRegValue > MAXIMUM_SPEED_FW)
  {
    TotalRegValue = MAXIMUM_SPEED_FW;
    pMD->MotorOverloaded = 1;
  }
  if (TotalRegValue < MAXIMUM_SPEED_RW)
  {
    TotalRegValue = MAXIMUM_SPEED_RW;
    pMD->MotorOverloaded = 1;
  }
  pMD->MotorActualSpeed = (SBYTE)TotalRegValue;
}

/* Regulation function used when syncrhonization regulation is enabled */
/* The regulation form controls two motors at a time */
void dOutputSyncMotorPosition(UBYTE MotorOne, UBYTE MotorTwo)
{
  SLONG TempTurnParameter;
  SLONG PValue;
  SLONG IValue;
  SLONG DValue;
  SLONG CorrectionValue;
  SLONG MotorSpeed;

  MOTORDATA * pOne = &(MotorData[MotorOne]);
  MOTORDATA * pTwo = &(MotorData[MotorTwo]);
  SyncData.SyncTachoDif = (SLONG)((pOne->MotorBlockTachoCount) - (pTwo->MotorBlockTachoCount));

  if (pOne->TurnParameter != 0)
  {
    if ((pOne->MotorBlockTachoCount != 0) || (pTwo->MotorBlockTachoCount != 0))
    {
      if (pOne->MotorTargetSpeed >= 0)
      {
        if (pOne->TurnParameter > 0)
        {
          TempTurnParameter = (SLONG)(((SLONG)pTwo->TurnParameter * (SLONG)pTwo->MotorTargetSpeed)/100);
        }
        else
        {
          TempTurnParameter = (SLONG)(((SLONG)pOne->TurnParameter * (SLONG)pOne->MotorTargetSpeed)/100);
        }
      }
      else
      {
        if (pOne->TurnParameter > 0)
        {
          TempTurnParameter = (SLONG)(((SLONG)pOne->TurnParameter * (-(SLONG)pOne->MotorTargetSpeed))/100);
        }
        else
        {
          TempTurnParameter = (SLONG)(((SLONG)pTwo->TurnParameter * (-(SLONG)pTwo->MotorTargetSpeed))/100);
        }
      }
    }
    else
    {
      TempTurnParameter = pOne->TurnParameter;
    }
  }
  else
  {
    TempTurnParameter = 0;
  }

  SyncData.SyncTurnParameter += (SLONG)(((TempTurnParameter * (MAX_CAPTURE_COUNT))/INPUT_SCALE_FACTOR)*2);
  //SyncTurnParameter should ophold difference between the two motors.

  SyncData.SyncTachoDif += SyncData.SyncTurnParameter;
  SyncData.SyncTachoDif = dOutputBound (SyncData.SyncTachoDif, 500);

  PValue = SyncData.SyncTachoDif * (pOne->RegPParameter/REG_CONST_DIV);

  DValue = (SyncData.SyncTachoDif - SyncData.SyncOldError) * (pOne->RegDParameter/REG_CONST_DIV);
  SyncData.SyncOldError = (SWORD)SyncData.SyncTachoDif;

  SyncData.SyncAccError += (SWORD)SyncData.SyncTachoDif;
  SyncData.SyncAccError = dOutputBound (SyncData.SyncAccError, 900);

  IValue = SyncData.SyncAccError * (pOne->RegIParameter/REG_CONST_DIV);

  CorrectionValue = (PValue + IValue + DValue) / 4;

  MotorSpeed = pOne->MotorTargetSpeed - CorrectionValue;
  MotorSpeed = dOutputBound (MotorSpeed, MAXIMUM_SPEED_FW);

  if (pOne->TurnParameter != 0)
  {
    if (pOne->MotorTargetSpeed > 0)
    {
      MotorSpeed = dOutputBound (MotorSpeed, pOne->MotorTargetSpeed);
    }
    else
    {
      MotorSpeed = dOutputBound (MotorSpeed, -pOne->MotorTargetSpeed);
    }
  }
  pOne->MotorActualSpeed = (SBYTE)MotorSpeed;

  MotorSpeed = pTwo->MotorTargetSpeed + CorrectionValue;
  MotorSpeed = dOutputBound (MotorSpeed, MAXIMUM_SPEED_FW);

  if (pOne->TurnParameter != 0)
  {
    if (pTwo->MotorTargetSpeed > 0)
    {
      MotorSpeed = dOutputBound (MotorSpeed, pTwo->MotorTargetSpeed);
    }
    else
    {
      MotorSpeed = dOutputBound (MotorSpeed, -pTwo->MotorTargetSpeed);
    }
  }
  pTwo->MotorActualSpeed = (SBYTE)MotorSpeed;
}

//Called when the motor is ramping down
void dOutputMotorReachedTachoLimit(UBYTE MotorNr)
{
  MOTORDATA * pOne = &(MotorData[MotorNr]);
  if (pOne->RegulationMode & REGSTATE_SYNCHRONE)
  {
    UBYTE MotorOne, MotorTwo;
    MotorOne = MotorNr;
    MotorTwo = 0xFF;
    UBYTE i;
    for(i = MOTOR_A; i <= MOTOR_C; i++) {
      if (i == MotorOne)
        continue;
      if (MotorData[i].RegulationMode & REGSTATE_SYNCHRONE) {
        MotorTwo = i;
        break;
      }
    }
    pOne->MotorSetSpeed = 0;
    pOne->MotorTargetSpeed = 0;
    pOne->MotorActualSpeed = 0;
    pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
    pOne->RegulationMode  = REGSTATE_IDLE;
    if (MotorTwo != 0xFF) {
      MOTORDATA * pTwo = &(MotorData[MotorTwo]);
      pTwo->MotorSetSpeed = 0;
      pTwo->MotorTargetSpeed = 0;
      pTwo->MotorActualSpeed = 0;
      pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;
      pTwo->RegulationMode = REGSTATE_IDLE;
    }
  }
  else
  {
    if (pOne->MotorSetSpeed == 0)
    {
      pOne->MotorTargetSpeed = 0;
      pOne->MotorActualSpeed = 0;
    }
    pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
    pOne->RegulationMode = REGSTATE_IDLE;
  }
}

/* Function used for control tacho limit when motors are synchronised */
/* Special control is needed when the motor are turning */
void dOutputSyncTachoLimitControl(UBYTE MotorNr)
{
  UBYTE MotorOne, MotorTwo;

  MotorOne = MotorNr;
  MotorTwo = 0xFF;
  // Synchronisation is done two times, as this function is called for each
  // motor.  This is the same behaviour as previous code.
  UBYTE i;
  for(i = MOTOR_A; i <= MOTOR_C; i++) {
    if (i == MotorOne)
      continue;
    if (MotorData[i].RegulationMode & REGSTATE_SYNCHRONE) {
      MotorTwo = i;
      break;
    }
  }
  if (MotorTwo == 0xFF)
    MotorOne = 0xFF;

  if ((MotorOne != 0xFF) && (MotorTwo != 0xFF))
  {
    MOTORDATA * pOne = &(MotorData[MotorOne]);
    MOTORDATA * pTwo = &(MotorData[MotorTwo]);
    if (pOne->TurnParameter != 0)
    {
      if (pOne->TurnParameter > 0)
      {
        if (pTwo->MotorTargetSpeed >= 0)
        {
          if ((SLONG)(pTwo->CurrentCaptureCount >= pTwo->MotorTachoCountToRun))
          {
            pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
            pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;

            pOne->CurrentCaptureCount = pTwo->CurrentCaptureCount;
            pOne->MotorTachoCountToRun = pTwo->MotorTachoCountToRun;
          }
        }
        else
        {
          if ((SLONG)(pOne->CurrentCaptureCount <= pOne->MotorTachoCountToRun))
          {
            pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
            pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;

            pTwo->CurrentCaptureCount = pOne->CurrentCaptureCount;
            pTwo->MotorTachoCountToRun = pOne->MotorTachoCountToRun;
          }
        }
      }
      else
      {
        if (pOne->MotorTargetSpeed >= 0)
        {
          if ((SLONG)(pOne->CurrentCaptureCount >= pOne->MotorTachoCountToRun))
          {
            pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
            pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;

            pTwo->CurrentCaptureCount = pOne->CurrentCaptureCount;
            pTwo->MotorTachoCountToRun = pOne->MotorTachoCountToRun;
          }
        }
        else
        {
          if ((SLONG)(pTwo->CurrentCaptureCount <= pTwo->MotorTachoCountToRun))
          {
            pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
            pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;

            pOne->CurrentCaptureCount = pTwo->CurrentCaptureCount;
            pOne->MotorTachoCountToRun = pTwo->MotorTachoCountToRun;
          }
        }
      }
    }
    else
    {
      if (pOne->MotorSetSpeed > 0)
      {
        if ((pOne->CurrentCaptureCount >= pOne->MotorTachoCountToRun) || (pTwo->CurrentCaptureCount >= pTwo->MotorTachoCountToRun))
        {
          pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
          pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;
        }
      }
      else
      {
        if (pOne->MotorSetSpeed < 0)
        {
          if ((pOne->CurrentCaptureCount <= pOne->MotorTachoCountToRun) || (pTwo->CurrentCaptureCount <= pTwo->MotorTachoCountToRun))
          {
            pOne->MotorRunState = MOTOR_RUN_STATE_IDLE;
            pTwo->MotorRunState = MOTOR_RUN_STATE_IDLE;
          }
        }
      }
    }
  }
}

/* Function which can evaluate which motor are synched */
void dOutputMotorSyncStatus(UBYTE MotorNr, UBYTE *SyncMotorOne, UBYTE *SyncMotorTwo)
{
  if (MotorNr < MOTOR_C)
  {
    if (MotorNr == MOTOR_A)
    {
      *SyncMotorOne = MotorNr;
      *SyncMotorTwo = *SyncMotorOne + 1;
      if (MotorData[*SyncMotorTwo].RegulationMode & REGSTATE_SYNCHRONE)
      {
        //Synchronise motor A & B
      }
      else
      {
        *SyncMotorTwo = *SyncMotorOne + 2;
        if (MotorData[*SyncMotorTwo].RegulationMode & REGSTATE_SYNCHRONE)
        {
          //Synchronise motor A & C
        }
        else
        {
          //Only Motor A has Sync setting => Do nothing, treat motor as motor without regulation
          *SyncMotorTwo = 0xFF;
        }
      }
    }
    if (MotorNr == MOTOR_B)
    {
      *SyncMotorOne = MotorNr;
      *SyncMotorTwo = *SyncMotorOne + 1;
      if (MotorData[*SyncMotorTwo].RegulationMode & REGSTATE_SYNCHRONE)
      {
        if (!(MotorData[MOTOR_A].RegulationMode & REGSTATE_SYNCHRONE))
        {
          //Synchronise motor B & C
        }
      }
      else
      {
        //Only Motor B has Sync settings or Motor is sync. with Motor A and has therefore already been called
        *SyncMotorTwo = 0xFF;
      }
    }
  }
  else
  {
    *SyncMotorOne = 0xFF;
    *SyncMotorTwo = 0xFF;
  }
}
/* Function which is called when motors are synchronized and the motor position is reset */
void dOutputResetSyncMotors(UBYTE MotorNr)
{
  UBYTE MotorOne, MotorTwo;

  MotorOne = MotorNr;
  MotorTwo = 0xFF;
  UBYTE i;
  for(i = MOTOR_A; i <= MOTOR_C; i++) {
    if (i == MotorOne)
      continue;
    if (MotorData[i].RegulationMode & REGSTATE_SYNCHRONE) {
      MotorTwo = i;
      break;
    }
  }
  if (MotorTwo == 0xFF)
    MotorOne = 0xFF;

  MOTORDATA * pMD = &(MotorData[MotorNr]);
  if ((MotorOne != 0xFF) && (MotorTwo != 0xFF))
  {
    MOTORDATA * pTwo = &(MotorData[MotorTwo]);
    pMD->CurrentCaptureCount = 0;
    pMD->MotorTachoCountToRun = 0;
    pTwo->CurrentCaptureCount = 0;
    pTwo->MotorTachoCountToRun = 0;
  }
  else
  {
    pMD->CurrentCaptureCount = 0;
    pMD->MotorTachoCountToRun = 0;
  }
}

/* Function which is called when motors are synchronized and motor is ramping down */
void dOutputRampDownSynch(UBYTE MotorNr)
{
  UBYTE MotorOne, MotorTwo;

  MotorOne = MotorNr;
  MotorTwo = 0xFF;
  UBYTE i;
  for(i = MOTOR_A; i <= MOTOR_C; i++) {
    if (i == MotorOne)
      continue;
    if (MotorData[i].RegulationMode & REGSTATE_SYNCHRONE) {
      MotorTwo = i;
      break;
    }
  }
  if (MotorTwo == 0xFF)
    MotorOne = 0xFF;

  if ((MotorOne != 0xFF) && (MotorTwo != 0xFF))
  {
    MOTORDATA * pOne = &(MotorData[MotorOne]);
    MOTORDATA * pTwo = &(MotorData[MotorTwo]);
    if (pOne->TurnParameter != 0)
    {
      if (pOne->TurnParameter > 0)
      {
        if (pOne->MotorTargetSpeed >= 0)
        {
          if (pTwo->MotorActualSpeed < 0)
          {
            pTwo->MotorTargetSpeed--;
          }
        }
        else
        {
          if (pTwo->MotorActualSpeed > 0)
          {
            pTwo->MotorTargetSpeed++;
          }
        }
      }
      else
      {
        if (pOne->MotorTargetSpeed >= 0)
        {
          if (pTwo->MotorActualSpeed < 0)
          {
            pTwo->MotorTargetSpeed--;
          }
        }
        else
        {
          if (pTwo->MotorActualSpeed > 0)
          {
            pTwo->MotorTargetSpeed++;
          }
        }
      }
    }
  }
}

