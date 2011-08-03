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
    OUTPUT * pOut = &(IOMapOutput.Outputs[Tmp]);
    pOut->Mode  = 0x00;
    pOut->Speed = 0x00;
    pOut->ActualSpeed = 0x00;
    pOut->TachoCnt = 0x00;
    pOut->RunState = 0x00;
    pOut->TachoLimit = 0x00;
    pOut->RegPParameter = DEFAULT_P_GAIN_FACTOR;
    pOut->RegIParameter = DEFAULT_I_GAIN_FACTOR;
    pOut->RegDParameter = DEFAULT_D_GAIN_FACTOR;
    pOut->Options = 0x00;
    pOut->MaxSpeed = DEFAULT_MAX_SPEED;
    pOut->MaxAcceleration = DEFAULT_MAX_ACCELERATION;
  }
  IOMapOutput.RegulationTime = REGULATION_TIME;
  IOMapOutput.RegulationOptions = 0;
  VarsOutput.TimeCnt = 0;
  dOutputInit();
}

void cOutputCtrl(void)
{
  UBYTE Tmp;

  for(Tmp = 0; Tmp < NO_OF_OUTPUTS; Tmp++)
  {
    OUTPUT * pOut = &(IOMapOutput.Outputs[Tmp]);
    if (pOut->Flags != 0)
	  {
		  if (pOut->Flags & UPDATE_RESET_ROTATION_COUNT)
      {
        pOut->Flags &= ~UPDATE_RESET_ROTATION_COUNT;
        dOutputResetRotationCaptureCount(Tmp);
      }      
      if (pOut->Flags & UPDATE_RESET_COUNT)
		  {
		    pOut->Flags &= ~UPDATE_RESET_COUNT;
		    dOutputResetTachoLimit(Tmp);		
		  }
      if (pOut->Flags & UPDATE_RESET_BLOCK_COUNT)
      {
        pOut->Flags &= ~UPDATE_RESET_BLOCK_COUNT;
		    dOutputResetBlockTachoLimit(Tmp);		
      }
      if (pOut->Flags & UPDATE_SPEED)
		  {
		    pOut->Flags &= ~UPDATE_SPEED;
		    if (pOut->Mode & MOTORON)
        {  	
		      dOutputSetSpeed (Tmp, pOut->RunState, pOut->Speed, pOut->SyncTurnParameter);
		    }		
		  }
		  if (pOut->Flags & UPDATE_MODE)
		  {
        pOut->Flags &= ~UPDATE_MODE;
		    if (pOut->Mode & BRAKE)
        {
          // Motor is Braked
          dOutputSetMode(Tmp, BRAKE);
	      }
        else
        {
          // Motor is floated
          dOutputSetMode(Tmp, 0x00);
        }
		    if (pOut->Mode & MOTORON)
        {	
		      if (pOut->Mode & REGULATED)
		      {
		        dOutputEnableRegulation(Tmp, pOut->RegMode);
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
      if (pOut->Flags & UPDATE_TACHO_LIMIT)
		  {
		    pOut->Flags &= ~UPDATE_TACHO_LIMIT;
		    dOutputSetTachoLimit(Tmp, pOut->TachoLimit, pOut->Options);
		  }	
      if (pOut->Flags & UPDATE_PID_VALUES)
		  {
	      pOut->Flags &= ~UPDATE_PID_VALUES;
		    dOutputSetPIDParameters(Tmp, pOut->RegPParameter, pOut->RegIParameter, pOut->RegDParameter);
		    dOutputSetMax(Tmp, pOut->MaxSpeed, pOut->MaxAcceleration);
		  }
	  }
  }
  dOutputSetRegulationTime(IOMapOutput.RegulationTime);
  dOutputSetRegulationOptions(IOMapOutput.RegulationOptions);
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
	  OUTPUT * pOut = &(IOMapOutput.Outputs[Tmp]);
	  pOut->ActualSpeed = TempCurrentMotorSpeed[Tmp];
    pOut->TachoCnt = TempTachoCount[Tmp];	
    pOut->BlockTachoCount = TempBlockTachoCount[Tmp];
	  pOut->RotationCount = TempRotationCount[Tmp];
    pOut->Overloaded = TempMotorOverloaded[Tmp];
    if (!(pOut->Flags & PENDING_UPDATES))
    {
      pOut->RunState = TempRunState[Tmp];
    }
	}
}

void      cOutputExit(void)
{
  dOutputExit();
}
