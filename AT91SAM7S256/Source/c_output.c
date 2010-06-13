//
// Date init       14.12.2004
//
// Revision date   $Date:: 14-11-07 12:40                                    $
//
// Filename        $Workfile:: c_output.c                                    $
//
// Version         $Revision:: 1                                             $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_outp $
//
// Platform        C
//

#include  <stdio.h>
#include  "stdbool.h"
#include  "stdconst.h"
#include  "modules.h"
#include  "c_output.iom"
#include  "c_output.h"
#include  "d_output.h"
#include  "c_display.iom"

static    IOMAPOUTPUT   IOMapOutput;
static    VARSOUTPUT    VarsOutput;

const     HEADER       cOutput =
{
  0x00020001L,
  "Output",
  cOutputInit,
  cOutputCtrl,
  cOutputExit,
  (void *)&IOMapOutput,
  (void *)&VarsOutput,
  (UWORD)sizeof(IOMapOutput),
  (UWORD)sizeof(VarsOutput),
  0x0000                      //Code size - not used so far
};


void      cOutputInit(void* pHeader)
{
  UBYTE   Tmp;

  for(Tmp = 0; Tmp < NO_OF_OUTPUTS; Tmp++)
  {
    IOMapOutput.Outputs[Tmp].Mode  = 0x00;
    IOMapOutput.Outputs[Tmp].Speed = 0x00;
	  IOMapOutput.Outputs[Tmp].ActualSpeed = 0x00;
	  IOMapOutput.Outputs[Tmp].TachoCnt = 0x00;
	  IOMapOutput.Outputs[Tmp].RunState = 0x00;
	  IOMapOutput.Outputs[Tmp].TachoLimit = 0x00;
    IOMapOutput.Outputs[Tmp].RegPParameter = DEFAULT_P_GAIN_FACTOR;
    IOMapOutput.Outputs[Tmp].RegIParameter = DEFAULT_I_GAIN_FACTOR;
    IOMapOutput.Outputs[Tmp].RegDParameter = DEFAULT_D_GAIN_FACTOR;
  }
  VarsOutput.TimeCnt = 0;
  dOutputInit();
}

void cOutputCtrl(void)
{
  UBYTE Tmp;

  for(Tmp = 0; Tmp < NO_OF_OUTPUTS; Tmp++)
  {
    if (IOMapOutput.Outputs[Tmp].Flags != 0)
	  {
		  if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_RESET_ROTATION_COUNT)
      {
        IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_RESET_ROTATION_COUNT;
        dOutputResetRotationCaptureCount(Tmp);
      }      
      if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_RESET_COUNT)
		  {
		    IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_RESET_COUNT;
		    dOutputResetTachoLimit(Tmp);		
		  }
      if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_RESET_BLOCK_COUNT)
      {
        IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_RESET_BLOCK_COUNT;
		    dOutputResetBlockTachoLimit(Tmp);		
      }
      if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_SPEED)
		  {
		    IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_SPEED;
		    if (IOMapOutput.Outputs[Tmp].Mode & MOTORON)
        {  	
		      dOutputSetSpeed (Tmp, IOMapOutput.Outputs[Tmp].RunState, IOMapOutput.Outputs[Tmp].Speed, IOMapOutput.Outputs[Tmp].SyncTurnParameter);
		    }		
		  }
      if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_TACHO_LIMIT)
		  {
		    IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_TACHO_LIMIT;
		    dOutputSetTachoLimit(Tmp, IOMapOutput.Outputs[Tmp].TachoLimit);
		  }	
		  if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_MODE)
		  {
        IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_MODE;
		    if (IOMapOutput.Outputs[Tmp].Mode & BRAKE)
        {
          // Motor is Braked
          dOutputSetMode(Tmp, BRAKE);
	      }
        else
        {
          // Motor is floated
          dOutputSetMode(Tmp, 0x00);
        }
		    if (IOMapOutput.Outputs[Tmp].Mode & MOTORON)
        {	
		      if (IOMapOutput.Outputs[Tmp].Mode & REGULATED)
		      {
		        dOutputEnableRegulation(Tmp, IOMapOutput.Outputs[Tmp].RegMode);
		      }
		      else
		      {
		        dOutputDisableRegulation(Tmp);
		      }
		    }
		    else
		    {
		      dOutputSetSpeed(Tmp, 0x00, 0x00, 0x00);
          dOutputDisableRegulation(Tmp);		
		    }	
		  }		  		  		
      if (IOMapOutput.Outputs[Tmp].Flags & UPDATE_PID_VALUES)
		  {
	      IOMapOutput.Outputs[Tmp].Flags &= ~UPDATE_PID_VALUES;
		    dOutputSetPIDParameters(Tmp, IOMapOutput.Outputs[Tmp].RegPParameter, IOMapOutput.Outputs[Tmp].RegIParameter, IOMapOutput.Outputs[Tmp].RegDParameter);
		  }
	  }
  }
  dOutputCtrl();
  cOutputUpdateIomap();
}

void cOutputUpdateIomap(void)
{
	UBYTE TempCurrentMotorSpeed[NO_OF_OUTPUTS];
	UBYTE TempRunState[NO_OF_OUTPUTS];	
  UBYTE TempMotorOverloaded[NO_OF_OUTPUTS];
	SLONG TempTachoCount[NO_OF_OUTPUTS];
  SLONG TempBlockTachoCount[NO_OF_OUTPUTS];
  SLONG TempRotationCount[NO_OF_OUTPUTS];

  UBYTE Tmp;
  
	dOutputGetMotorParameters(TempCurrentMotorSpeed, TempTachoCount, TempBlockTachoCount, TempRunState, TempMotorOverloaded,TempRotationCount);

	for(Tmp = 0; Tmp < NO_OF_OUTPUTS; Tmp++)
  {
	  IOMapOutput.Outputs[Tmp].ActualSpeed = TempCurrentMotorSpeed[Tmp];
	  IOMapOutput.Outputs[Tmp].TachoCnt = TempTachoCount[Tmp];	
    IOMapOutput.Outputs[Tmp].BlockTachoCount = TempBlockTachoCount[Tmp];
    IOMapOutput.Outputs[Tmp].RotationCount = TempRotationCount[Tmp];
	  IOMapOutput.Outputs[Tmp].Overloaded = TempMotorOverloaded[Tmp];
    if (!(IOMapOutput.Outputs[Tmp].Flags & PENDING_UPDATES))
    {
      IOMapOutput.Outputs[Tmp].RunState = TempRunState[Tmp];
    }
	}
}

void      cOutputExit(void)
{
  dOutputExit();
}
