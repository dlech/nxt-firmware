//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: d_output.h                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/d_outp $
//
// Platform        C
//

#ifndef   D_OUTPUT
#define   D_OUTPUT

#define   NEW_MOTOR

#ifdef    NEW_MOTOR

//Constant reffering to new motor
#define REG_CONST_DIV           32            // Constant which the PID constants value will be divided with
#define DEFAULT_P_GAIN_FACTOR		96//3                            
#define DEFAULT_I_GAIN_FACTOR		32//1                       
#define DEFAULT_D_GAIN_FACTOR		32//1
#define MIN_MOVEMENT_POWER      10
#define MAX_CAPTURE_COUNT       100

#else

//Constant reffering to Old motor
#define REG_CONST_DIV           1            // Constant which the PID constants value will be divided with
#define DEFAULT_P_GAIN_FACTOR		3                            
#define DEFAULT_I_GAIN_FACTOR		1                       
#define DEFAULT_D_GAIN_FACTOR		1
#define MIN_MOVEMENT_POWER      30
#define MAX_CAPTURE_COUNT       80            

#endif

#define DEFAULT_MAX_SPEED        80
#define DEFAULT_MAX_ACCELERATION 20

#define REGULATION_TIME         100          // Measured in 1 mS => 100 mS regulation interval
//#define REGULATION_TIME         10           // Measured in 1 mS, regulation interval

//Constant reffering to RegMode parameter
#define REGSTATE_IDLE           0x00
#define REGSTATE_REGULATED      0x01
#define REGSTATE_SYNCHRONE      0x02
#define REGSTATE_POSITION       0x04

//Constant reffering to RunState parameter
#define MOTOR_RUN_STATE_IDLE      0x00
#define MOTOR_RUN_STATE_RAMPUP    0x10
#define MOTOR_RUN_STATE_RUNNING   0x20
#define MOTOR_RUN_STATE_RAMPDOWN  0x40
#define MOTOR_RUN_STATE_HOLD      0x60

// Constants related to Regulation Options
#define REGOPTION_NO_SATURATION 0x01 // Do not limit intermediary regulation results

enum
{
  MOTOR_A,
  MOTOR_B,
  MOTOR_C
};

void      dOutputInit(void);
void      dOutputExit(void);

void      dOutputCtrl(void);
void      dOutputGetMotorParameters(UBYTE *CurrentMotorSpeed, SLONG *TachoCount, SLONG *BlockTachoCount, UBYTE *RunState, UBYTE *MotorOverloaded, SLONG *RotationCount);
void      dOutputSetMode(UBYTE MotorNr, UBYTE Mode);
void      dOutputSetSpeed (UBYTE MotorNr, UBYTE NewMotorRunState, SBYTE Speed, SBYTE TurnParameter);
void      dOutputEnableRegulation(UBYTE MotorNr, UBYTE RegulationMode);
void	    dOutputDisableRegulation(UBYTE MotorNr);
void      dOutputSetTachoLimit(UBYTE MotorNr, ULONG TachoCntToTravel, UBYTE Options);
void      dOutputResetTachoLimit(UBYTE MotorNr);
void      dOutputResetBlockTachoLimit(UBYTE MotorNr);
void      dOutputResetRotationCaptureCount(UBYTE MotorNr);
void      dOutputSetPIDParameters(UBYTE MotorNr, UBYTE NewRegPParameter, UBYTE NewRegIParameter, UBYTE NewRegDParameter); 
void      dOutputSetMax(UBYTE MotorNr, SBYTE NewMaxSpeed, SBYTE NewMaxAcceleration);
void      dOutputSetRegulationTime(UBYTE NewRegulationTime);
void      dOutputSetRegulationOptions(UBYTE NewRegulationOptions);

void 	    dOutputRegulateMotor(UBYTE MotorNr);
void      dOutputCalculateRampUpParameter(UBYTE MotorNr, ULONG NewTachoLimit);
void      dOutputRampDownFunction(UBYTE MotorNr);
void      dOutputRampUpFunction(UBYTE MotorNr);
void      dOutputTachoLimitControl(UBYTE MotorNr);
void      dOutputAbsolutePositionRegulation(UBYTE MotorNr);
void      dOutputCalculateMotorPosition(UBYTE MotorNr);
void      dOutputSyncMotorPosition(UBYTE MotorOne, UBYTE MotorTwo);
void      dOutputMotorReachedTachoLimit(UBYTE MotorNr);
void      dOutputMotorIdleControl(UBYTE MotorNr);
void      dOutputSyncTachoLimitControl(UBYTE MotorNr);
void      dOutputMotorSyncStatus(UBYTE MotorNr, UBYTE *SyncMotorOne, UBYTE *SyncMotorTwo);
void      dOutputResetSyncMotors(UBYTE MotorNr);
void      dOutputUpdateRegulationTime(UBYTE rt);

#endif
