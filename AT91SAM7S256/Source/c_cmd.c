//
// Date init       14.12.2004
//
// Revision date   $Date:: 28-03-07 14:53                                    $
//
// Filename        $Workfile:: c_cmd.c                                       $
//
// Version         $Revision:: 67                                            $
//
// Archive         $Archive:: /LMS2006/Sys01/Main/Firmware/Source/c_cmd.c    $
//
// Platform        C
//

//
// File Description:
// This file contains the virtual machine implementation to run bytecode
// programs compatible with LEGO MINDSTORMS NXT Software 1.0.
//
// This module (c_cmd) is also responsible for reading the system timer
// (d_timer) and returning on 1 ms timer boundaries.
//

#include "stdconst.h"
#include "modules.h"

#include "c_cmd.iom"
#include "c_output.iom"
#include "c_input.iom"
#include "c_loader.iom"
#include "c_ui.iom"
#include "c_sound.iom"
#include "c_button.iom"
#include "c_display.iom"
#include "c_comm.iom"
#include "c_lowspeed.iom"

#include "c_cmd.h"
#include "c_cmd_bytecodes.h"
#include "d_timer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static  IOMAPCMD  IOMapCmd;
static  VARSCMD   VarsCmd;
static  HEADER    **pHeaders;

const     HEADER  cCmd =
{
  0x00010001L,
  "Command",
  cCmdInit,
  cCmdCtrl,
  cCmdExit,
  (void *)&IOMapCmd,
  (void *)&VarsCmd,
  (UWORD)sizeof(IOMapCmd),
  (UWORD)sizeof(VarsCmd),
  0x0000                      //Code size - not used so far
};

#if ENABLE_VM

// c_cmd_drawing.inc is just another C source file
// (the graphics implementation was split off for practical file management reasons)
#include "c_cmd_drawing.inc"


//
//Function pointers to sub-interpreters
//This table is indexed by arity
//Unary operations can have arity of 1 or 2 (some need a destination)
//All instructions taking 4 or more operands are handled as "Other"
//
static pInterp InterpFuncs[INTERP_COUNT] =
{
  cCmdInterpNoArg,
  cCmdInterpUnop1,
  cCmdInterpUnop2,
  cCmdInterpBinop,
  cCmdInterpOther
};

//
//Function pointers to SysCall implementations
//See interpreter for OP_SYSCALL
//
static pSysCall SysCallFuncs[SYSCALL_COUNT] =
{
  cCmdWrapFileOpenRead,
  cCmdWrapFileOpenWrite,
  cCmdWrapFileOpenAppend,
  cCmdWrapFileRead,
  cCmdWrapFileWrite,
  cCmdWrapFileClose,
  cCmdWrapFileResolveHandle,
  cCmdWrapFileRename,
  cCmdWrapFileDelete,
  cCmdWrapSoundPlayFile,
  cCmdWrapSoundPlayTone,
  cCmdWrapSoundGetState,
  cCmdWrapSoundSetState,
  cCmdWrapDrawText,
  cCmdWrapDrawPoint,
  cCmdWrapDrawLine,
  cCmdWrapDrawCircle,
  cCmdWrapDrawRect,
  cCmdWrapDrawPicture,
  cCmdWrapSetScreenMode,
  cCmdWrapReadButton,
  cCmdWrapCommLSWrite,
  cCmdWrapCommLSRead,
  cCmdWrapCommLSCheckStatus,
  cCmdWrapRandomNumber,
  cCmdWrapGetStartTick,
  cCmdWrapMessageWrite,
  cCmdWrapMessageRead,
  cCmdWrapCommBTCheckStatus,
  cCmdWrapCommBTWrite,
  cCmdWrapCommBTRead,
  cCmdWrapKeepAlive,
  cCmdWrapIOMapRead,
  cCmdWrapIOMapWrite
};

//
//Next set of arrays are lookup tables for IOM access bytecodes
//
TYPE_CODE IO_TYPES_IN[IO_IN_FIELD_COUNT] =
{
  //IO_IN0
  TC_UBYTE, //IO_IN_TYPE
  TC_UBYTE, //IO_IN_MODE
  TC_UWORD, //IO_IN_ADRAW
  TC_UWORD, //IO_IN_NORMRAW
  TC_SWORD, //IO_IN_SCALED_VAL
  TC_UBYTE, //IO_IN_INVALID_DATA

  //IO_IN1
  TC_UBYTE, //IO_IN_TYPE
  TC_UBYTE, //IO_IN_MODE
  TC_UWORD, //IO_IN_ADRAW
  TC_UWORD, //IO_IN_NORMRAW
  TC_SWORD, //IO_IN_SCALED_VAL
  TC_UBYTE, //IO_IN_INVALID_DATA

  //IO_IN2
  TC_UBYTE, //IO_IN_TYPE
  TC_UBYTE, //IO_IN_MODE
  TC_UWORD, //IO_IN_ADRAW
  TC_UWORD, //IO_IN_NORMRAW
  TC_SWORD, //IO_IN_SCALED_VAL
  TC_UBYTE, //IO_IN_INVALID_DATA

  //IO_IN3
  TC_UBYTE, //IO_IN_TYPE
  TC_UBYTE, //IO_IN_MODE
  TC_UWORD, //IO_IN_ADRAW
  TC_UWORD, //IO_IN_NORMRAW
  TC_SWORD, //IO_IN_SCALED_VAL
  TC_UBYTE, //IO_IN_INVALID_DATA
};

TYPE_CODE IO_TYPES_OUT[IO_OUT_FIELD_COUNT] =
{
  //IO_OUT0
  TC_UBYTE, //IO_OUT_FLAGS
  TC_UBYTE, //IO_OUT_MODE
  TC_SBYTE, //IO_OUT_SPEED
  TC_SBYTE, //IO_OUT_ACTUAL_SPEED
  TC_SLONG, //IO_OUT_TACH_COUNT
  TC_ULONG, //IO_OUT_TACH_LIMIT
  TC_UBYTE, //IO_OUT_RUN_STATE
  TC_SBYTE, //IO_OUT_TURN_RATIO
  TC_UBYTE, //IO_OUT_REG_MODE
  TC_UBYTE, //IO_OUT_OVERLOAD
  TC_UBYTE, //IO_OUT_REG_P_VAL
  TC_UBYTE, //IO_OUT_REG_I_VAL
  TC_UBYTE, //IO_OUT_REG_D_VAL
  TC_SLONG, //IO_OUT_BLOCK_TACH_COUNT
  TC_SLONG, //IO_OUT_ROTATION_COUNT

  //IO_OUT1
  TC_UBYTE, //IO_OUT_FLAGS
  TC_UBYTE, //IO_OUT_MODE
  TC_SBYTE, //IO_OUT_SPEED
  TC_SBYTE, //IO_OUT_ACTUAL_SPEED
  TC_SLONG, //IO_OUT_TACH_COUNT
  TC_ULONG, //IO_OUT_TACH_LIMIT
  TC_UBYTE, //IO_OUT_RUN_STATE
  TC_SBYTE, //IO_OUT_TURN_RATIO
  TC_UBYTE, //IO_OUT_REG_MODE
  TC_UBYTE, //IO_OUT_OVERLOAD
  TC_UBYTE, //IO_OUT_REG_P_VAL
  TC_UBYTE, //IO_OUT_REG_I_VAL
  TC_UBYTE, //IO_OUT_REG_D_VAL
  TC_SLONG, //IO_OUT_BLOCK_TACH_COUNT
  TC_SLONG, //IO_OUT_ROTATION_COUNT

  //IO_OUT2
  TC_UBYTE, //IO_OUT_FLAGS
  TC_UBYTE, //IO_OUT_MODE
  TC_SBYTE, //IO_OUT_SPEED
  TC_SBYTE, //IO_OUT_ACTUAL_SPEED
  TC_SLONG, //IO_OUT_TACH_COUNT
  TC_ULONG, //IO_OUT_TACH_LIMIT
  TC_UBYTE, //IO_OUT_RUN_STATE
  TC_SBYTE, //IO_OUT_TURN_RATIO
  TC_UBYTE, //IO_OUT_REG_MODE
  TC_UBYTE, //IO_OUT_OVERLOAD
  TC_UBYTE, //IO_OUT_REG_P_VAL
  TC_UBYTE, //IO_OUT_REG_I_VAL
  TC_UBYTE, //IO_OUT_REG_D_VAL
  TC_SLONG, //IO_OUT_BLOCK_TACH_COUNT
  TC_SLONG, //IO_OUT_ROTATION_COUNT
};


TYPE_CODE * IO_TYPES[2] =
{
  IO_TYPES_IN,
  IO_TYPES_OUT
};

//Actual pointers filled in during cCmdInit()
void * IO_PTRS_IN[IO_IN_FIELD_COUNT];
void * IO_PTRS_OUT[IO_OUT_FIELD_COUNT];

void ** IO_PTRS[2] =
{
  IO_PTRS_IN,
  IO_PTRS_OUT
};


//cCmdHandleRemoteCommands is the registered handler for "direct" command protocol packets
//It is only intended to be called via c_comm's main protocol handler
UWORD cCmdHandleRemoteCommands(UBYTE * pInBuf, UBYTE * pOutBuf, UBYTE * pLen)
{
  NXT_STATUS RCStatus = NO_ERR;
  //Response packet length.  Always includes RCStatus byte.
  ULONG ResponseLen = 1;
  //Boolean flag to send a response.  TRUE unless overridden below.
  ULONG SendResponse = TRUE;
  //Boolean flag if we are handling a reply telegram.  FALSE unless overridden.
  ULONG IncomingReply = FALSE;
  ULONG i, FirstPort, LastPort;
  UWORD LStatus;
  UWORD Count, QueueID;
  UBYTE * pData;

  //Illegal call, give up
  if (pInBuf == NULL || pLen == NULL)
  {
    NXT_BREAK;
    return (0xFFFF);
  }

  //No output buffer provided, so skip any work related to returning a response
  if (pOutBuf == NULL)
    SendResponse = FALSE;

  //If first byte identifies this as a reply telegram, we have different work to do.
  if (pInBuf[0] == 0x02)
  {
    IncomingReply = TRUE;
    //Reply telegrams never get responses, even if caller provided a buffer.
    SendResponse = FALSE;
  }

  //Advance pInBuf past command type byte
  pInBuf++;

  if (!IncomingReply)
  {
    switch(pInBuf[0])
    {
      case RC_START_PROGRAM:
      {
        //Check that file exists.  If not, return error
        //!!! Should return standard loader file error in cases like this??
        //!!! Proper solution would also check file mode to avoid confusing errors
        if (LOADER_ERR(LStatus = pMapLoader->pFunc(FINDFIRST, (&pInBuf[1]), NULL, NULL)) != SUCCESS)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        //Close file handle returned by FINDFIRST
        pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(LStatus), NULL, NULL);

        //File must exist, so inform UI to attempt execution in the usual way (enables consistent feedback)
        pMapUi->Flags |= UI_EXECUTE_LMS_FILE;
        strncpy((PSZ)(pMapUi->LMSfilename), (PSZ)(&pInBuf[1]), FILENAME_LENGTH + 1);
      }
      break;

      case RC_STOP_PROGRAM:
      {
        if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
        {
          RCStatus = ERR_NO_PROG;
          break;
        }

        IOMapCmd.DeactivateFlag = TRUE;
      }
      break;

      case RC_PLAY_SOUND_FILE:
      {
        if (LOADER_ERR(pMapLoader->pFunc(FINDFIRST, (&pInBuf[2]), NULL, NULL)) != SUCCESS)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        //Close file handle returned by FINDFIRST
        pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(LStatus), NULL, NULL);

        if (pInBuf[1] == FALSE)
          pMapSound->Mode = SOUND_ONCE;
        else //Any non-zero value treated as TRUE
          pMapSound->Mode = SOUND_LOOP;

        strncpy((PSZ)pMapSound->SoundFilename, (PSZ)(&pInBuf[2]), FILENAME_LENGTH + 1);
        pMapSound->Flags |= SOUND_UPDATE;
      }
      break;

      case RC_PLAY_TONE:
      {
        pMapSound->Mode = SOUND_TONE;
        //!!! Range check valid values?
        memcpy((PSZ)(&(pMapSound->Freq)), (PSZ)(&pInBuf[1]), 2);
        memcpy((PSZ)(&(pMapSound->Duration)), (PSZ)(&pInBuf[3]), 2);

        pMapSound->Flags |= SOUND_UPDATE;
      }
      break;

      case RC_SET_OUT_STATE:
      {
        //Don't do anything if illegal port specification is made
        if (pInBuf[1] >= NO_OF_OUTPUTS && pInBuf[1] != 0xFF)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        //0xFF is protocol defined to mean "all ports".
        if (pInBuf[1] == 0xFF)
        {
          FirstPort = 0;
          LastPort = NO_OF_OUTPUTS - 1;
        }
        else
          FirstPort = LastPort = pInBuf[1];

        for (i = FirstPort; i <= LastPort; i++)
        {
          pMapOutPut->Outputs[i].Speed             = pInBuf[2];
          pMapOutPut->Outputs[i].Mode              = pInBuf[3];
          pMapOutPut->Outputs[i].RegMode           = pInBuf[4];
          pMapOutPut->Outputs[i].SyncTurnParameter = pInBuf[5];
          pMapOutPut->Outputs[i].RunState          = pInBuf[6];
          memcpy((PSZ)(&(pMapOutPut->Outputs[i].TachoLimit)), (PSZ)(&pInBuf[7]), 4);

          pMapOutPut->Outputs[i].Flags |= UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;
        }
      }
      break;

      case RC_SET_IN_MODE:
      {
        i = pInBuf[1];

        //Don't do anything if illegal port specification is made
        //!!! Should check against legal Types and Modes? (bitmask for Modes?)
        if (i >= NO_OF_INPUTS)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        pMapInput->Inputs[i].SensorType = pInBuf[2];
        pMapInput->Inputs[i].SensorMode = pInBuf[3];

        //Set InvalidData flag automatically since type may have changed
        pMapInput->Inputs[i].InvalidData = TRUE;
      }
      break;

      case RC_GET_OUT_STATE:
      {
        if (SendResponse == TRUE)
        {
          i = pInBuf[1];

          //Return error and all zeros if illegal port specification is made
          if (i >= NO_OF_OUTPUTS)
          {
            RCStatus = ERR_RC_ILLEGAL_VAL;
            memset(&(pOutBuf[ResponseLen]), 0, 22);
            ResponseLen += 22;
            break;
          }

          //Echo port
          pOutBuf[ResponseLen] = i;
          ResponseLen++;

          //Power
          pOutBuf[ResponseLen] = pMapOutPut->Outputs[i].Speed;
          ResponseLen++;

          //Mode
          pOutBuf[ResponseLen] = pMapOutPut->Outputs[i].Mode;
          ResponseLen++;

          //RegMode
          pOutBuf[ResponseLen] = pMapOutPut->Outputs[i].RegMode;
          ResponseLen++;

          //TurnRatio
          pOutBuf[ResponseLen] = pMapOutPut->Outputs[i].SyncTurnParameter;
          ResponseLen++;

          //RunState
          pOutBuf[ResponseLen] = pMapOutPut->Outputs[i].RunState;
          ResponseLen++;

          //TachoLimit ULONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapOutPut->Outputs[i].TachoLimit)), 4);
          ResponseLen += 4;

          //TachoCount SLONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapOutPut->Outputs[i].TachoCnt)), 4);
          ResponseLen += 4;

          //BlockTachoCount SLONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapOutPut->Outputs[i].BlockTachoCount)), 4);
          ResponseLen += 4;

          //RotationCount SLONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapOutPut->Outputs[i].RotationCount)), 4);
          ResponseLen += 4;

          NXT_ASSERT(ResponseLen == 23);
        }
      }
      break;

      case RC_GET_IN_VALS:
      {
        if (SendResponse == TRUE)
        {
          i = pInBuf[1];

          //Return error and all zeros if illegal port specification is made
          if (i >= NO_OF_INPUTS)
          {
            RCStatus = ERR_RC_ILLEGAL_VAL;
            memset(&(pOutBuf[ResponseLen]), 0, 13);
            ResponseLen += 13;
            break;
          }

          //Echo port
          pOutBuf[ResponseLen] = i;
          ResponseLen++;

          //Set "Valid?" boolean
          if (pMapInput->Inputs[i].InvalidData)
            pOutBuf[ResponseLen] = FALSE;
          else
            pOutBuf[ResponseLen] = TRUE;

          ResponseLen++;

          //Set "Calibrated?" boolean
          //!!! "Calibrated?" is a placeholder in the protocol.  Always FALSE for now.
          pOutBuf[ResponseLen] = FALSE;
          ResponseLen++;

          pOutBuf[ResponseLen] = pMapInput->Inputs[i].SensorType;
          ResponseLen++;

          pOutBuf[ResponseLen] = pMapInput->Inputs[i].SensorMode;
          ResponseLen++;

          //Set Raw, Normalized, and Scaled values
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapInput->Inputs[i].ADRaw)), 2);
          ResponseLen += 2;

          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapInput->Inputs[i].SensorRaw)), 2);
          ResponseLen += 2;

          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapInput->Inputs[i].SensorValue)), 2);
          ResponseLen += 2;

          //!!! Return normalized raw value in place of calibrated value for now -- see comment above
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pMapInput->Inputs[i].SensorRaw)), 2);
          ResponseLen += 2;

          NXT_ASSERT(ResponseLen == 14);
        }
      }
      break;

      case RC_RESET_IN_VAL:
      {
        i = pInBuf[1];

        //Don't do anything if illegal port specification is made
        if (i >= NO_OF_INPUTS)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        //Clear SensorValue to zero.  Leave Raw and Normalized as-is, since they never accumulate running values.
        pMapInput->Inputs[i].SensorValue = 0;
      }
      break;

      case RC_MESSAGE_WRITE:
      {
        QueueID = pInBuf[1];
        Count = pInBuf[2];
        pData = &(pInBuf[3]);

        //If Count is illegal or MsgData is not null-terminated,
        // we can't accept it as a valid string
        if (Count == 0 || Count > MAX_MESSAGE_SIZE || pData[Count - 1] != 0x00)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        RCStatus = cCmdMessageWrite(QueueID, pData, Count);

        //ERR_MEM here means we must compact the dataspace and retry message write
        if (RCStatus == ERR_MEM)
        {
          cCmdDSCompact();
          RCStatus = cCmdMessageWrite(QueueID, pData, Count);
        }
      }
      break;

      case RC_RESET_POSITION:
      {
        i = pInBuf[1];

        //Don't do anything if illegal port specification is made
        if (i >= NO_OF_OUTPUTS)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        //pInBuf[2] is a selector
        //FALSE: Position relative to start of last program
        //TRUE: Position relative to start of last motor control block
        if (pInBuf[2] == FALSE)
        {
          pMapOutPut->Outputs[i].Flags |= UPDATE_RESET_ROTATION_COUNT;
        }
        else
        {
          pMapOutPut->Outputs[i].Flags |= UPDATE_RESET_BLOCK_COUNT;
        }
      }
      break;

      case RC_GET_BATT_LVL:
      {
        if (SendResponse == TRUE)
        {
          //Return BatteryVoltage directly from IOMapUI, in mV
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)&(pMapUi->BatteryVoltage), 2);
          ResponseLen += 2;
        }
      }
      break;

      case RC_STOP_SOUND:
      {
        //Tell sound module to stop playback, no questions asked
        pMapSound->State = SOUND_STOP;
      }
      break;

      case RC_KEEP_ALIVE:
      {
        pMapUi->Flags |= UI_RESET_SLEEP_TIMER;

        if (SendResponse == TRUE)
        {
          //Convert to milliseconds to match external conventions
          i = (pMapUi->SleepTimeout * 60 * 1000);
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)&i, 4);
          ResponseLen += 4;
        }
      }
      break;

      case RC_LS_GET_STATUS:
      {
        if (SendResponse == TRUE)
        {
          i = pInBuf[1];

          //Don't do anything if illegal port specification is made
          if (i >= NO_OF_LOWSPEED_COM_CHANNEL)
          {
            RCStatus = ERR_RC_ILLEGAL_VAL;
            break;
          }

          RCStatus = cCmdLSCheckStatus(i);

          pOutBuf[ResponseLen] = cCmdLSCalcBytesReady(i);
          ResponseLen++;
        }
      }
      break;

      case RC_LS_WRITE:
      {
        i = pInBuf[1];
        Count = pInBuf[2];

        //Don't do anything if illegal port specification is made
        if (i >= NO_OF_LOWSPEED_COM_CHANNEL)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        RCStatus = cCmdLSWrite(i, Count, &(pInBuf[4]), pInBuf[3]);
      }
      break;

      case RC_LS_READ:
      {
        if (SendResponse == TRUE)
        {
          i = pInBuf[1];

          //Don't do anything if illegal port specification is made
          if (i >= NO_OF_LOWSPEED_COM_CHANNEL)
          {
            RCStatus = ERR_RC_ILLEGAL_VAL;
            break;
          }

          //Get channel status and number of bytes available to read
          RCStatus = cCmdLSCheckStatus(i);
          Count = cCmdLSCalcBytesReady(i);

          pOutBuf[ResponseLen] = (UBYTE)Count;
          ResponseLen++;

          //If channel is ready and has data ready for us, put the data into outgoing buffer
          if (!IS_ERR(RCStatus) && Count > 0)
          {
            RCStatus = cCmdLSRead(i, (UBYTE)Count, &(pOutBuf[ResponseLen]));
            ResponseLen += Count;
          }

          //Pad remaining data bytes with zeroes
          Count = 16 - Count;
          memset(&(pOutBuf[ResponseLen]), 0, Count);
          ResponseLen += Count;
        }
      }
      break;

      case RC_GET_CURR_PROGRAM:
      {
        if (SendResponse == TRUE)
        {
          //If there's no active program, return error and empty name buffer
          if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
          {
            RCStatus = ERR_NO_PROG;
            memset(&(pOutBuf[ResponseLen]), 0, FILENAME_LENGTH + 1);
          }
          //Else, copy out stashed program name
          else
          {
            strncpy((PSZ)(&(pOutBuf[ResponseLen])), (PSZ)(VarsCmd.ActiveProgName), FILENAME_LENGTH + 1);
          }

          //Regardless, we've copied out a filename's worth of bytes...
          ResponseLen += FILENAME_LENGTH + 1;
        }
      }
      break;

      case RC_MESSAGE_READ:
      {
        if (SendResponse == TRUE)
        {
          QueueID = pInBuf[1];

          //Fill in response with remote mailbox number so remote device knows where to store this message.
          pOutBuf[ResponseLen] = pInBuf[2];
          ResponseLen++;

          RCStatus = cCmdMessageGetSize(QueueID, &Count);
          pOutBuf[ResponseLen] = Count;
          ResponseLen++;

          if (!IS_ERR(RCStatus) && Count > 0)
          {
            pData = &(pOutBuf[ResponseLen]);
            RCStatus = cCmdMessageRead(QueueID, pData, Count, (pInBuf[3]));
            //If cCmdMessageRead encountered an error, there is no real data in the buffer, so clear it out (below)
            if (IS_ERR(RCStatus))
              Count = 0;
            else
              ResponseLen += Count;
          }

          //Pad remaining data bytes with zeroes
          Count = MAX_MESSAGE_SIZE - Count;
          memset(&(pOutBuf[ResponseLen]), 0, Count);
          ResponseLen += Count;
        }
      }
      break;

      default:
      {
        //Unknown remote command -- still inform client to not expect any response bytes
        NXT_BREAK;
        RCStatus = ERR_RC_UNKNOWN_CMD;
      }
      break;
    };
  }
  //Handle reply telegrams
  else
  {
    switch(pInBuf[0])
    {
      case RC_MESSAGE_READ:
      {
        QueueID = pInBuf[2];
        Count   = pInBuf[3];
        pData = &(pInBuf[4]);

        //This is a response to our request to read a message from a remote mailbox.
        //If telegram looks valid, write the resulting message into our local mailbox.
        //(If MsgData is not null-terminated, we can't accept it as a valid string.)
        if (!IS_ERR((SBYTE)(pInBuf[1]))
         && Count > 0
         && Count <= MAX_MESSAGE_SIZE
         && pData[Count - 1] == 0x00)
        {
          RCStatus = cCmdMessageWrite(QueueID, pData, Count);

          //ERR_MEM here means we must compact the dataspace
          if (RCStatus == ERR_MEM)
          {
            cCmdDSCompact();
            RCStatus = cCmdMessageWrite(QueueID, pData, Count);
          }
        }

        //If telegram doesn't check out, do nothing.  No errors are ever returned for reply telegrams.
      }
      break;

      default:
      {
        //Unhandled reply telegram.  Do nothing.
        //!!! Could/should stash unhandled/all replies somewhere so a syscall could read them
      }
      break;
    };
  }

  if (SendResponse == TRUE)
  {
    //Return response length (pointer checked above)
    *pLen = (UBYTE)ResponseLen;
    //Fill in status byte
    pOutBuf[0] = (UBYTE)(RCStatus);
  }
  else
    *pLen = 0;

  return (0);
}


//
// Standard interface functions
//

void      cCmdInit(void* pHeader)
{
  ULONG i;

  pHeaders        = pHeader;

  IOMapCmd.pRCHandler = &cCmdHandleRemoteCommands;

#if defined(ARM_DEBUG)
  //Init run-time assert tracking variables
  VarsCmd.AssertFlag = FALSE;
  VarsCmd.AssertLine = 0;
#endif

  //Initialize IO_PTRS_OUT
  for (i = 0; i < NO_OF_OUTPUTS; i++)
  {
    IO_PTRS_OUT[IO_OUT_FLAGS            + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].Flags);
    IO_PTRS_OUT[IO_OUT_MODE             + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].Mode);
    IO_PTRS_OUT[IO_OUT_SPEED            + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].Speed);
    IO_PTRS_OUT[IO_OUT_ACTUAL_SPEED     + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].ActualSpeed);
    IO_PTRS_OUT[IO_OUT_TACH_COUNT       + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].TachoCnt);
    IO_PTRS_OUT[IO_OUT_TACH_LIMIT       + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].TachoLimit);
    IO_PTRS_OUT[IO_OUT_RUN_STATE        + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].RunState);
    IO_PTRS_OUT[IO_OUT_TURN_RATIO       + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].SyncTurnParameter);
    IO_PTRS_OUT[IO_OUT_REG_MODE         + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].RegMode);
    IO_PTRS_OUT[IO_OUT_OVERLOAD         + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].Overloaded);
    IO_PTRS_OUT[IO_OUT_REG_P_VAL        + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].RegPParameter);
    IO_PTRS_OUT[IO_OUT_REG_I_VAL        + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].RegIParameter);
    IO_PTRS_OUT[IO_OUT_REG_D_VAL        + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].RegDParameter);
    IO_PTRS_OUT[IO_OUT_BLOCK_TACH_COUNT + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].BlockTachoCount);
    IO_PTRS_OUT[IO_OUT_ROTATION_COUNT   + i * IO_OUT_FPP] = (void*)&(pMapOutPut->Outputs[i].RotationCount);
  }

  //Initialize IO_PTRS_IN
  for (i = 0; i < NO_OF_INPUTS; i++)
  {
    IO_PTRS_IN[IO_IN_TYPE         + i * IO_IN_FPP] = (void*)&(pMapInput->Inputs[i].SensorType);
    IO_PTRS_IN[IO_IN_MODE         + i * IO_IN_FPP] = (void*)&(pMapInput->Inputs[i].SensorMode);
    IO_PTRS_IN[IO_IN_ADRAW        + i * IO_IN_FPP] = (void*)&(pMapInput->Inputs[i].ADRaw);
    IO_PTRS_IN[IO_IN_NORMRAW      + i * IO_IN_FPP] = (void*)&(pMapInput->Inputs[i].SensorRaw);
    IO_PTRS_IN[IO_IN_SCALEDVAL    + i * IO_IN_FPP] = (void*)&(pMapInput->Inputs[i].SensorValue);
    IO_PTRS_IN[IO_IN_INVALID_DATA + i * IO_IN_FPP] = (void*)&(pMapInput->Inputs[i].InvalidData);
  }

  //Clear memory pool and initialize VarsCmd (cCmdDeactivateProgram effectively re-inits VarsCmd)
  cCmdInitPool();
  cCmdDeactivateProgram();

  //Global state variables for BlueTooth communication.
  VarsCmd.CommStat      = (SWORD)SUCCESS;
  VarsCmd.CommStatReset = (SWORD)BTBUSY;
  VarsCmd.CommCurrConnection = 1;

  //Global flags for various reset and bookkeeping scenarios
  VarsCmd.DirtyComm = FALSE;
  VarsCmd.DirtyDisplay = FALSE;

  VarsCmd.VMState = VM_IDLE;

#if defined (ARM_NXT)
  //Make sure Pool is long-aligned
  NXT_ASSERT(!((ULONG)(POOL_START) % SIZE_SLONG));
#endif

  IOMapCmd.ProgStatus = PROG_IDLE;
  IOMapCmd.ActivateFlag = FALSE;
  IOMapCmd.Awake  = TRUE;

  //Default offsets explicitly chosen to cause an error if used with IOMAPREAD/IOMAPWRITE
  //Real values will be set when programs run and/or the DS is re-arranged.
  IOMapCmd.OffsetDVA = 0xFFFF;
  IOMapCmd.OffsetDS = 0xFFFF;

  //Initialize format string and clear out FileName string
  strncpy((PSZ)(IOMapCmd.FormatString), VM_FORMAT_STRING, VM_FORMAT_STRING_SIZE);
  memset(IOMapCmd.FileName, 0, sizeof(IOMapCmd.FileName));

  dTimerInit();
  IOMapCmd.Tick = dTimerRead();

  return;
}


void cCmdCtrl(void)
{
  UBYTE Continue = TRUE;
  NXT_STATUS Status = NO_ERR;
  ULONG i;
  CLUMP_ID CurrClumpID;

  switch (VarsCmd.VMState)
  {
    case VM_IDLE:
    {
      //If there's a new program to activate...
      if (IOMapCmd.ActivateFlag == TRUE)
      {
        //Clear flag so we only activate once per new file
        IOMapCmd.ActivateFlag = FALSE;

        Status = cCmdActivateProgram(IOMapCmd.FileName);

        //If we hit an activation error:
        //1. Set PROG_ERROR status
        //2. Proceed to VM_RESET1 (some unneeded work, yes, but preserves contract with UI
        if (IS_ERR(Status))
        {
          IOMapCmd.ProgStatus = PROG_ERROR;
          VarsCmd.VMState = VM_RESET1;
        }
        //Else start running program
        else
        {
          VarsCmd.VMState = VM_RUN_FREE;
          IOMapCmd.ProgStatus = PROG_RUNNING;
          VarsCmd.StartTick = IOMapCmd.Tick;

#if VM_BENCHMARK
          //Re-init benchmark
          VarsCmd.InstrCount         = 0;
          VarsCmd.Average            = 0;
          VarsCmd.OverTimeCount      = 0;
          VarsCmd.MaxOverTimeLength  = 0;
          VarsCmd.CmdCtrlCount       = 0;
          VarsCmd.CompactionCount    = 0;
          VarsCmd.LastCompactionTick = 0;
          VarsCmd.MaxCompactionTime  = 0;
          memset(VarsCmd.OpcodeBenchmarks, 0, sizeof(VarsCmd.OpcodeBenchmarks));
          memset(VarsCmd.SyscallBenchmarks, 0, sizeof(VarsCmd.SyscallBenchmarks));
#endif
          //Reset devices to a known state before we begin running
          cCmdResetDevices();

          pMapUi->Flags |= (UI_DISABLE_LEFT_RIGHT_ENTER | UI_DISABLE_EXIT);
        }
      }

      break;
    }

    //Initialize VM internal state data and devices which must respond immediately to program ending
    case VM_RESET1:
    {
      //If we aborted a program, reset devices (specifically, motors) immediately
      //Otherwise, wait for UI to put us into PROG_RESET (gives motors a chance to brake before setting to coast)
      //!!! This means cCmdResetDevices will get called twice on abort.  Should not be a big deal.
      if (IOMapCmd.ProgStatus == PROG_ABORT)
        cCmdResetDevices();

      //Reenable UI access to buttons
      pMapUi->Flags &= ~(UI_DISABLE_LEFT_RIGHT_ENTER | UI_DISABLE_EXIT);

#if VM_BENCHMARK
      if (IOMapCmd.Tick != VarsCmd.StartTick)
        VarsCmd.Average = VarsCmd.InstrCount / (IOMapCmd.Tick - VarsCmd.StartTick);
      else
        //It appears that we finished in 0 milliseconds.  Very unlikely on ARM, so set a flag value.
        VarsCmd.Average = 0xFFFFFFFF;

      cCmdWriteBenchmarkFile();
#endif

      //Re-initialize program state data (contents of memory pool preserved)
      //!!! Skip this step in simulator builds so helper access methods still work
#ifndef SIM_NXT
      cCmdDeactivateProgram();
#endif //SIM_NXT

      //If this program has taken over the display, reset it for the UI
      cCmdRestoreDefaultScreen();

      //Stop any currently playing sound and re-init volume according to UI prefs
      pMapSound->State = SOUND_STOP;
      pMapSound->Volume = pMapUi->Volume;

      //Artificially set CommStatReset to BTBUSY to force at least one SETCMDMODE call (see VM_RESET2 case)
      VarsCmd.CommStatReset = (SWORD)BTBUSY;

      VarsCmd.VMState = VM_RESET2;
    }
    break;

    case VM_RESET2:
    {
      //Reset BlueCore into "command mode" (close any open streams)
      //Since SETCMDMODE subject to BTBUSY, we may need to make multiple calls
      //Any CommStatReset value other than BTBUSY means our request was accepted
      //Assumptions:
      //Process should never take longer than UI timeout (see below), but if it does,
      // we could be left with the stream open to an NXT peer and block out the PC.
      //Also assuming that once SETCMDMODE request is accepted, it never fails.
      if (VarsCmd.CommStatReset == (SWORD)BTBUSY && VarsCmd.DirtyComm == TRUE)
        pMapComm->pFunc(SETCMDMODE, 0, 0, 0, NULL, (UWORD*)&(VarsCmd.CommStatReset));

      //If UI is done displaying ending program status, move on.
      if (IOMapCmd.ProgStatus == PROG_RESET)
      {
        //Reset devices whenever a program ends for any reason
        cCmdResetDevices();

        VarsCmd.DirtyComm = FALSE;

        //Go to VM_IDLE state
        VarsCmd.VMState = VM_IDLE;
        IOMapCmd.ProgStatus = PROG_IDLE;
      }
      break;
    }

    case VM_RUN_FREE:
    case VM_RUN_SINGLE:
    {

#if VM_BENCHMARK
      //IOMapCmd.Tick currently holds the tick from the end of last cCmdCtrl call.
      //If we don't come back here before dTimerRead() increments, the m_sched loop has taken *at least* 1 ms.
      if (IOMapCmd.Tick != dTimerRead())
      {
        VarsCmd.OverTimeCount++;
        //Record maximum magnitude of schedule loop overage, in millisecs
        if (dTimerRead() - IOMapCmd.Tick > VarsCmd.MaxOverTimeLength)
          VarsCmd.MaxOverTimeLength = dTimerRead() - IOMapCmd.Tick;
      }
      VarsCmd.CmdCtrlCount++;
#endif
      //Abort current program if cancel button is pressed
      if (IOMapCmd.DeactivateFlag == TRUE || pMapButton->State[BTN1] & PRESSED_EV)
      {
        IOMapCmd.DeactivateFlag = FALSE;

        //Clear pressed event so it doesn't get double-counted by UI
        pMapButton->State[BTN1] &= ~PRESSED_EV;

        //Go to VM_RESET1 state and report abort
        VarsCmd.VMState = VM_RESET1;
        IOMapCmd.ProgStatus = PROG_ABORT;
        break;
      }

      //Assert that we have an active program
      NXT_ASSERT(VarsCmd.ActiveProgHandle != NOT_A_HANDLE);

      //Execute from at least one clump
      do
      {
        if (cCmdIsClumpIDSane(VarsCmd.RunQ.Head))
        {
          //Stash and dequeue RunQ's head clump
          CurrClumpID = VarsCmd.RunQ.Head;

          //Execute at least one instruction from current clump
          //Execute up to 'Priority' instructions as long as we are in VM_FREE_RUN mode
          //Finishing/suspending a clump, BREAKOUT_REQ, or any errors will also end this loop
          i = 0;
          do
          {
            //Interpret one instruction per call, advancing PC as needed
            Status = cCmdInterpFromClump(CurrClumpID);

#if VM_BENCHMARK
            VarsCmd.InstrCount++;
#endif

            NXT_ASSERT(!IS_ERR(Status));
            if (IS_ERR(Status) || Status == CLUMP_DONE || Status == CLUMP_SUSPEND || Status == BREAKOUT_REQ || Status == STOP_REQ)
            {
              //We're done with this clump or breaking out prematurely,
              //so break the multi-instruction loop
              break;
            }
            else
            {
              //Count up one more instruction for this pass
              i++;
            }
          } while (VarsCmd.VMState == VM_RUN_FREE && i < VarsCmd.pAllClumps[CurrClumpID].Priority);

          //Only rotate RunQ on a "normal" finish, i.e. no error, clump end, or breakout request
          if (!(IS_ERR(Status) || Status == CLUMP_DONE || Status == CLUMP_SUSPEND || Status == BREAKOUT_REQ))
            cCmdRotateQ(&(VarsCmd.RunQ));
        }

        //Re-evaluate conditions for stopping the dataflow scheduler
        //Halt program on all errors
        if (IS_ERR(Status))
        {
          Continue = FALSE;

          VarsCmd.VMState = VM_RESET1;
          IOMapCmd.ProgStatus = PROG_ERROR;
        }
        else if (Status == BREAKOUT_REQ)
        {
          Continue = FALSE;
        }
        //If RunQ is empty or user requested early termination, program is done
        else if (!cCmdIsClumpIDSane(VarsCmd.RunQ.Head) || Status == STOP_REQ)
        {
          Continue = FALSE;

          VarsCmd.VMState = VM_RESET1;
          IOMapCmd.ProgStatus = PROG_OK;
        }
        //VM_RUN_FREE means continue executing until a new ms tick rolls over
        else if (VarsCmd.VMState == VM_RUN_FREE)
        {
          Continue = (IOMapCmd.Tick == dTimerRead());
        }
        //Otherwise execute only one pass per call
        else //VarsCmd.VMState == VM_RUN_SINGLE
        {
          VarsCmd.VMState = VM_RUN_PAUSE;
          Continue = FALSE;
        }

      } while (Continue == TRUE);

    break;
    }
  }//END state machine switch

  //Busy wait to always maintain 1ms period
  BUSY_WAIT_NEXT_MS;

  //Set tick to new value for next time 'round
  IOMapCmd.Tick = dTimerRead();

  return;
}


void cCmdExit(void)
{
  dTimerExit();

  return;
}


NXT_STATUS cCmdReadFileHeader(UBYTE* pData, ULONG DataSize,
            PROG_FILE_OFFSETS* pFileOffsets)
{
  ULONG i;
  UBYTE * pCursor;
  UWORD CurrOffset = 0;
  UBYTE DepCount;
  UWORD DopeVectorOffset;
  UWORD FileClumpCount;
  UBYTE FileMajor, FileMinor,
        CompatibleMinor, CompatibleMajor,
        CurrentMajor;

  NXT_ASSERT(pData != NULL);

  //Assign pCursor to point to version word inside file header
  pCursor = (pData + VM_FORMAT_STRING_SIZE - 2);

  //Decode version numbers into comparable bytes
  FileMajor = *pCursor;
  FileMinor = *(pCursor + 1);
  CompatibleMajor = (UBYTE)(VM_OLDEST_COMPATIBLE_VERSION >> 8);
  CompatibleMinor = (UBYTE)(VM_OLDEST_COMPATIBLE_VERSION);
  CurrentMajor = (UBYTE)(FIRMWAREVERSION >> 8);
  //CurrentMinor = (UBYTE)(FIRMWAREVERSION);

  //Return ERR_VER if file lacks proper format string or version number
  //!!! Only checking major version recommended for future development
  if (strncmp((PSZ)pData, VM_FORMAT_STRING, VM_FORMAT_STRING_SIZE)
   || FileMajor < CompatibleMajor || FileMinor < CompatibleMinor
   || FileMajor > CurrentMajor)
  {
    NXT_BREAK;
    return (ERR_VER);
  }

  //Advance CurrOffset past header information
  CurrOffset += VM_FORMAT_STRING_SIZE;

  //
  //Initialize bookkeeping variables
  //
  VarsCmd.DataspaceCount = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  VarsCmd.DataspaceSize = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  VarsCmd.DSStaticSize = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  pFileOffsets->DSDefaultsSize = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  pFileOffsets->DynamicDefaults = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  pFileOffsets->DynamicDefaultsSize = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  VarsCmd.MemMgr.Head = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  VarsCmd.MemMgr.Tail = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  DopeVectorOffset = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  //!!! Odd code here to deal with type mismatch between file format and CLUMP_ID typedef.
  //Neither is trivial to change, so it's best to just check the data for consistency here.
  FileClumpCount = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  //Must have at least one clump and count can't exceed the NOT_A_CLUMP sentinel
  if (FileClumpCount == 0 || FileClumpCount >= NOT_A_CLUMP)
    return (ERR_FILE);
  else
    VarsCmd.AllClumpsCount = (CLUMP_ID)FileClumpCount;

  VarsCmd.CodespaceCount = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  //Can't have a valid program with no code
  if (VarsCmd.CodespaceCount == 0)
    return (ERR_FILE);

  //
  // Now, calculate offsets for each data segment in the file
  //

  CurrOffset += CurrOffset % 2;
  pFileOffsets->DSTOC = CurrOffset;
  CurrOffset += VarsCmd.DataspaceCount * sizeof(DS_TOC_ENTRY);

  CurrOffset += CurrOffset % 2;
  pFileOffsets->DSDefaults = CurrOffset;
  CurrOffset += pFileOffsets->DSDefaultsSize;

  //ClumpRecs must be aligned on even boundaries
  CurrOffset += CurrOffset % 2;
  pFileOffsets->Clumps = CurrOffset;

  //Set cursor to start of clump records
  pCursor = pData + CurrOffset;

  //Set CurrOffset to start of dependent lists
  CurrOffset += VarsCmd.AllClumpsCount * VM_FILE_CLUMP_REC_SIZE;

  //Read dependent count from each clump record, advancing CurrOffset accordingly
  for (i = 0; i < VarsCmd.AllClumpsCount; i++)
  {
    DepCount = *(pCursor + 1);
    CurrOffset += DepCount;
    pCursor += VM_FILE_CLUMP_REC_SIZE;
  }

  //Codespace must be aligned on even boundary
  CurrOffset += CurrOffset % 2;
  pFileOffsets->Codespace = CurrOffset;

  //No need to read through codespace, but make sure CurrOffset ended up sane
  //If not, something went wrong reading the header information
  if (CurrOffset != (DataSize - VarsCmd.CodespaceCount * 2))
  {
    NXT_BREAK;
    return (ERR_FILE);
  }

  //
  // Finally, update VarsCmd fields
  //

  VarsCmd.RunQ.Head = NOT_A_CLUMP;
  VarsCmd.RunQ.Tail = NOT_A_CLUMP;

  //Reset codespace pointer
  VarsCmd.pCodespace = (CODE_WORD*)(pData + pFileOffsets->Codespace);

  //...placing clump records first...
  VarsCmd.pAllClumps = (CLUMP_REC*)(VarsCmd.Pool + VarsCmd.PoolSize);
  VarsCmd.PoolSize += VarsCmd.AllClumpsCount * sizeof(CLUMP_REC);

  //...then DSTOC...
  VarsCmd.pDataspaceTOC = (DS_TOC_ENTRY*)(pData + pFileOffsets->DSTOC);

  //...then the dataspace itself
  ALIGN_TO_MOD(VarsCmd.PoolSize, POOL_ALIGN);
  VarsCmd.pDataspace = (VarsCmd.Pool + VarsCmd.PoolSize);
  IOMapCmd.OffsetDS = (UWORD)((ULONG)(VarsCmd.pDataspace) - (ULONG)&(IOMapCmd));
  VarsCmd.PoolSize += VarsCmd.DataspaceSize;

  //init rest of MemMgr
  VarsCmd.MemMgr.pDopeVectorArray = (DOPE_VECTOR *)(VarsCmd.pDataspace + DopeVectorOffset);
  IOMapCmd.OffsetDVA = (UWORD)((ULONG)(VarsCmd.MemMgr.pDopeVectorArray) - (ULONG)&(IOMapCmd));
  VarsCmd.MemMgr.FreeHead = NOT_A_DS_ID;


  if (VarsCmd.PoolSize > POOL_MAX_SIZE)
  {
    NXT_BREAK;
    return (ERR_FILE);
  }

  return (NO_ERR);
}


//!!! Recursive function
NXT_STATUS cCmdInflateDSDefaults(UBYTE* pDSDefaults, UWORD *pDefaultsOffset, DS_ELEMENT_ID DSElementID)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode;
  UWORD i, Count;
  UBYTE *pVal;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));

  TypeCode = cCmdDSType(DSElementID);

  if (TypeCode == TC_CLUSTER)
  {
    Count = cCmdClusterCount(DSElementID);
    //Advance DSElementID to sub-type
    DSElementID = INC_ID(DSElementID);
    //Loop through sub-types, inflate recursively
    for (i = 0; i < Count; i++)
    {
      Status = cCmdInflateDSDefaults(pDSDefaults, pDefaultsOffset, DSElementID);
      if (IS_ERR(Status))
        return Status;
      DSElementID = cCmdNextDSElement(DSElementID);
    }
  }
  else
  {
    if (TypeCode == TC_ARRAY)
    {
      //Resolve pointer to DVIndex
      pVal = VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[DSElementID].DSOffset;
    }
    else
    {
      pVal = cCmdResolveDataArg(DSElementID, 0, NULL);
    }

    //Check if the element has the "default default"
    if (VarsCmd.pDataspaceTOC[DSElementID].Flags & DS_DEFAULT_DEFAULT)
    {
      //Fill element with the "default default" of zero
      memset(pVal, 0, cCmdSizeOf(TypeCode));
    }
    else
    {
      //Get default from stream
      memmove(pVal, pDSDefaults + *pDefaultsOffset, cCmdSizeOf(TypeCode));
      *pDefaultsOffset += cCmdSizeOf(TypeCode);
    }
  }

  //!!! Currently will always return NO_ERR
  return Status;
}


NXT_STATUS cCmdActivateProgram(UBYTE * pFileName)
{
  UWORD i, j;
  UBYTE * pCursor;

  NXT_STATUS Status = NO_ERR;
  PROG_FILE_OFFSETS FileOffsets;

  LOADER_STATUS LStatus;
  ULONG DataSize;
  UBYTE * pData;
  ULONG pDataHolder;
  UWORD DefaultsOffset;

  LStatus = pMapLoader->pFunc(OPENREADLINEAR, pFileName, (UBYTE*)(&pDataHolder), &DataSize);
  pData = (UBYTE*)(pDataHolder);

  //If Loader returned error or bad file pointer, bail out
  if (LOADER_ERR(LStatus) != SUCCESS || pData == NULL || DataSize == 0)
    return (ERR_FILE);

  //Deactivate current program and re-initialize memory pool
  cCmdDeactivateProgram();
  cCmdInitPool();

  //Stash this program's handle since we hold it open while running
  VarsCmd.ActiveProgHandle = LOADER_HANDLE(LStatus);

  //Stash this program's name for easy reference later
  strncpy((PSZ)(VarsCmd.ActiveProgName), (PSZ)(pFileName), FILENAME_LENGTH + 1);

  //Consume activation record data stream.
  //See TargettingVIs/NXT.PackAR.vi for data stream packing details

  //Read header portion of the file, calculating offsets and initializing VarsCmd
  Status = cCmdReadFileHeader(pData, DataSize, &FileOffsets);
  if (IS_ERR(Status))
    return Status;

  //Do some spot checks to make sure bad file contents didn't leave us with obviously insane VarsCmd contents
  //!!! Should add alignment checks on these pointers to avoid data abort exceptions later
  if (((UBYTE*)(VarsCmd.pCodespace) < pData)
   || ((UBYTE*)(VarsCmd.pCodespace) >= (pData + DataSize))
   || ((UBYTE*)(VarsCmd.pAllClumps) < POOL_START)
   || ((UBYTE*)(VarsCmd.pAllClumps) >= POOL_SENTINEL)
   || ((UBYTE*)(VarsCmd.pDataspace) < POOL_START)
   || ((UBYTE*)(VarsCmd.pDataspace) >= POOL_SENTINEL)
   || (VarsCmd.DataspaceSize == 0) )
  {
    NXT_BREAK;
    return ERR_FILE;
  }

  //Initialize CLUMP_RECs as contiguous list in RAM
  pCursor = (pData + FileOffsets.Clumps);
  for (i = 0; i < VarsCmd.AllClumpsCount; i++)
  {
    VarsCmd.pAllClumps[i].InitFireCount  = *(UBYTE*)(pCursor + i * VM_FILE_CLUMP_REC_SIZE);
    VarsCmd.pAllClumps[i].DependentCount = *(UBYTE*)(pCursor + (i * VM_FILE_CLUMP_REC_SIZE) + 1);
    VarsCmd.pAllClumps[i].CodeStart      = *(UWORD*)(pCursor + (i * VM_FILE_CLUMP_REC_SIZE) + 2);

    //Initialize remaining CLUMP_REC fields
    VarsCmd.pAllClumps[i].PC = 0;
    VarsCmd.pAllClumps[i].Priority = 20;
    VarsCmd.pAllClumps[i].Link = NOT_A_CLUMP;

    //Activate any clumps with CurrFireCount of 0
    VarsCmd.pAllClumps[i].CurrFireCount = VarsCmd.pAllClumps[i].InitFireCount;
    if (VarsCmd.pAllClumps[i].CurrFireCount == 0)
      cCmdEnQClump(&(VarsCmd.RunQ), (CLUMP_ID)i);
  }

  //Patch up dependents in separate pass (reuse of pCursor)
  pCursor += VarsCmd.AllClumpsCount * VM_FILE_CLUMP_REC_SIZE;
  for (i = 0; i < VarsCmd.AllClumpsCount; i++)
  {
    if (VarsCmd.pAllClumps[i].DependentCount > 0)
    {
      VarsCmd.pAllClumps[i].pDependents = (CLUMP_ID*)(pCursor);

      pCursor += (VarsCmd.pAllClumps[i].DependentCount * sizeof(CLUMP_ID));
    }
    else
      VarsCmd.pAllClumps[i].pDependents = NULL;

    //Patch up CodeEnd value based on CodeStart of next clump or last overall codeword
    if (i < (VarsCmd.AllClumpsCount - 1))
      VarsCmd.pAllClumps[i].CodeEnd = VarsCmd.pAllClumps[i+1].CodeStart - 1;
    else
      VarsCmd.pAllClumps[i].CodeEnd = VarsCmd.CodespaceCount - 1;

    //Test for empty/insane clump code definitions
    NXT_ASSERT(VarsCmd.pAllClumps[i].CodeStart < VarsCmd.pAllClumps[i].CodeEnd);
  }

  //Programs with no active clumps constitutes an activation error
  if (VarsCmd.RunQ.Head == NOT_A_CLUMP)
    return (ERR_FILE);

  //Initialize dataspace with default values from file
  //!!! This would be a good place to enforce check against potentially
  //    unsafe nested types (deeply nested types mean deep recursive calls)
  DefaultsOffset = 0;
  for (i = 0; i != NOT_A_DS_ID; i = cCmdNextDSElement(i))
  {

    Status = cCmdInflateDSDefaults(pData + FileOffsets.DSDefaults, &DefaultsOffset, i);
    if (IS_ERR(Status))
      return Status;
  }

  if ((DefaultsOffset != FileOffsets.DynamicDefaults)
   || (DefaultsOffset + FileOffsets.DynamicDefaultsSize != FileOffsets.DSDefaultsSize))
  {
    NXT_BREAK;
    return (ERR_FILE);
  }

  //Copy Dynamic defaults from file
  memmove(VarsCmd.pDataspace + VarsCmd.DSStaticSize, pData + FileOffsets.DSDefaults + FileOffsets.DynamicDefaults, FileOffsets.DynamicDefaultsSize);

  //Verify the MemMgr ended up where we said it would
  if ((UBYTE *)VarsCmd.MemMgr.pDopeVectorArray != VarsCmd.pDataspace + DV_ARRAY[0].Offset)
  {
    NXT_BREAK;
    return (ERR_FILE);
  }

  //Initialize message queues
  for (i = 0; i < MESSAGE_QUEUE_COUNT; i++)
  {
    VarsCmd.MessageQueues[i].ReadIndex = 0;
    VarsCmd.MessageQueues[i].WriteIndex = 0;

    for (j = 0; j < MESSAGES_PER_QUEUE; j++)
    {
      VarsCmd.MessageQueues[i].Messages[j] = NOT_A_DS_ID;
    }
  }

  if (cCmdVerifyMemMgr() != TRUE)
    return (ERR_FILE);

  return (Status);
}


void cCmdDeactivateProgram()
{
  UBYTE i, tmp;

  //Wipe away all references into the pool and clear all run-time data
  VarsCmd.pCodespace = NULL;
  VarsCmd.CodespaceCount = 0;

  VarsCmd.pAllClumps = NULL;
  VarsCmd.AllClumpsCount = 0;

  VarsCmd.DataspaceCount = 0;
  VarsCmd.pDataspaceTOC = NULL;
  VarsCmd.pDataspace = NULL;
  VarsCmd.DataspaceSize = 0;
  VarsCmd.DSStaticSize = 0;

  VarsCmd.MemMgr.Head = NOT_A_DS_ID;
  VarsCmd.MemMgr.Tail = NOT_A_DS_ID;
  VarsCmd.MemMgr.FreeHead = NOT_A_DS_ID;
  VarsCmd.MemMgr.pDopeVectorArray = NULL;

  VarsCmd.RunQ.Head = NOT_A_CLUMP;
  VarsCmd.RunQ.Tail = NOT_A_CLUMP;
  VarsCmd.ScratchPC = 0;
  VarsCmd.CallerClump = NOT_A_CLUMP;

  if (VarsCmd.ActiveProgHandle != NOT_A_HANDLE)
  {
    //Close handle that we've kept open for this program
    pMapLoader->pFunc(CLOSE, &(VarsCmd.ActiveProgHandle), NULL, NULL);
    VarsCmd.ActiveProgHandle = NOT_A_HANDLE;

    //Clear internal stashed name
    memset(VarsCmd.ActiveProgName, 0, FILENAME_LENGTH + 1);
  }

  //Close any files we had opened programatically
  for (i = 0; i < MAX_HANDLES; i++)
  {
    //Copy i to tmp, because we pass a pointer to it to pFunc
    tmp = i;
    //Close file
    if (*(VarsCmd.FileHandleTable[i]) != 0)
      pMapLoader->pFunc(CLOSE, &tmp, NULL, NULL);
  }

  //Clear FileHandleTable
  memset(VarsCmd.FileHandleTable, 0, sizeof(VarsCmd.FileHandleTable));

  return;
}


void cCmdResetDevices(void)
{
  UBYTE i;

  //Clear NXT button counts so 'bumped' will work on first run
  for (i = 0; i < NO_OF_BTNS; i++)
  {
    pMapButton->BtnCnt[i].RelCnt = 0;
    //Need to clear short and long counts too, because RelCnt depends on them.  No known side effects.
    pMapButton->BtnCnt[i].ShortRelCnt = 0;
    pMapButton->BtnCnt[i].LongRelCnt = 0;
  }

  for (i = 0; i < NO_OF_INPUTS; i++)
  {
    //Clear type and mode to defaults
    pMapInput->Inputs[i].SensorType = NO_SENSOR;
    pMapInput->Inputs[i].SensorMode = RAWMODE;

    //Reset input values to 0 prior to running (clear things like stale rotation counts)
    pMapInput->Inputs[i].ADRaw       = 0;
    pMapInput->Inputs[i].SensorRaw   = 0;
    pMapInput->Inputs[i].SensorValue = 0;

    //Assert invalid data flag so future code is aware of these changes
    pMapInput->Inputs[i].InvalidData = TRUE;
  }

  for (i = 0; i < NO_OF_OUTPUTS; i++)
  {
    //Coast and reset all motor parameters
    pMapOutPut->Outputs[i].Mode = 0;
    pMapOutPut->Outputs[i].RegMode = REGULATION_MODE_IDLE;
    pMapOutPut->Outputs[i].RunState = MOTOR_RUN_STATE_IDLE;
    pMapOutPut->Outputs[i].Speed = 0;
    pMapOutPut->Outputs[i].TachoLimit = 0;
    pMapOutPut->Outputs[i].SyncTurnParameter = 0;
    pMapOutPut->Outputs[i].Flags = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT | UPDATE_RESET_COUNT | UPDATE_RESET_BLOCK_COUNT | UPDATE_RESET_ROTATION_COUNT;
  }

  //Lowspeed init, INSERT CODE !!!
  for (i = 0; i < NO_OF_LOWSPEED_COM_CHANNEL; i++)
  {
    pMapLowSpeed->InBuf[i].InPtr = 0;
    pMapLowSpeed->InBuf[i].OutPtr = 0;
    pMapLowSpeed->InBuf[i].BytesToRx = 0;
    pMapLowSpeed->OutBuf[i].InPtr = 0;
    pMapLowSpeed->OutBuf[i].OutPtr = 0;
    if (pMapLowSpeed->ChannelState[i] != LOWSPEED_IDLE)
    {
      pMapLowSpeed->ChannelState[i] = LOWSPEED_DONE;
      pMapLowSpeed->State |= (0x01<<i);
    }
  }

}


//Add NewClump to end, updating Queue's head/tail as needed
void cCmdEnQClump(CLUMP_Q * Queue, CLUMP_ID NewClump)
{
  //Make sure NewClump's ID is valid and not already on Q
  NXT_ASSERT(cCmdIsClumpIDSane(NewClump));
  NXT_ASSERT(cCmdIsQSane(Queue) == TRUE);
  NXT_ASSERT(!cCmdIsClumpOnQ(Queue, NewClump));

  VarsCmd.pAllClumps[NewClump].Link = NOT_A_CLUMP;

  //If queue is empty, NewClump becomes both head and tail
  if (Queue->Head == NOT_A_CLUMP)
  {
    NXT_ASSERT(Queue->Tail == NOT_A_CLUMP);

    Queue->Head = NewClump;
    Queue->Tail = NewClump;
  }
  //Otherwise, tack onto the end
  else
  {
    VarsCmd.pAllClumps[Queue->Tail].Link = NewClump;
    Queue->Tail = NewClump;
  }

  return;
}

//Dequeue specified clump
//Normal usage is to dequeue only from the head (i.e. pass Queue.Head as arg)
void cCmdDeQClump(CLUMP_Q * Queue, CLUMP_ID Clump)
{
  CLUMP_ID CurrID, LinkID;

  //Make sure Clump's ID is valid and is already on Queue
  NXT_ASSERT(cCmdIsClumpIDSane(Clump));
  NXT_ASSERT(cCmdIsQSane(Queue) == TRUE);
  NXT_ASSERT(cCmdIsClumpOnQ(Queue, Clump));

  CurrID = Queue->Head;

  //If our clump is the head, move up the next and disconnect
  if (CurrID == Clump)
  {
    Queue->Head = VarsCmd.pAllClumps[Clump].Link;
    VarsCmd.pAllClumps[Clump].Link = NOT_A_CLUMP;

    //If we just removed the last clump, patch up the queue's tail
    if (Queue->Head == NOT_A_CLUMP)
      Queue->Tail = NOT_A_CLUMP;
  }
  //Else, look through rest of list looking for a link to our clump
  else
  {
    do
    {
      LinkID = VarsCmd.pAllClumps[CurrID].Link;

      //If we find a link to our clump, patch up predecessor's link
      if (VarsCmd.pAllClumps[CurrID].Link == Clump)
      {
        VarsCmd.pAllClumps[CurrID].Link = VarsCmd.pAllClumps[Clump].Link;
        VarsCmd.pAllClumps[Clump].Link = NOT_A_CLUMP;

        //If we just removed the tail, patch tail
        if (Clump == Queue->Tail)
          Queue->Tail = CurrID;
      }

      CurrID = LinkID;
    } while (CurrID != NOT_A_CLUMP);
  }

  return;
}


//Rotate head to tail and advance head for given Queue
void cCmdRotateQ(CLUMP_Q * Queue)
{
  CLUMP_ID CurrID;
  CLUMP_REC * pClumpRec;

  //Make sure Queue is sane
  NXT_ASSERT(cCmdIsQSane(Queue) == TRUE);

  //If queue has at least two clumps
  if (Queue->Head != Queue->Tail)
  {
    CurrID = Queue->Head;
    pClumpRec = &(VarsCmd.pAllClumps[CurrID]);

    //Disconnect head
    Queue->Head = pClumpRec->Link;
    pClumpRec->Link = NOT_A_CLUMP;

    //Reconnect head as tail
    pClumpRec = &(VarsCmd.pAllClumps[Queue->Tail]);
    pClumpRec->Link = CurrID;
    Queue->Tail = CurrID;

    //Make sure we didn't make any really stupid mistakes
    NXT_ASSERT(cCmdIsQSane(Queue) == TRUE);
  }

  return;
}


UBYTE cCmdIsClumpOnQ(CLUMP_Q * Queue, CLUMP_ID Clump)
{
  CLUMP_ID CurrID;

  //Make sure Clump's ID is valid and is already on Queue
  NXT_ASSERT(cCmdIsClumpIDSane(Clump));
  NXT_ASSERT(cCmdIsQSane(Queue) == TRUE);

  CurrID = Queue->Head;

  while (CurrID != NOT_A_CLUMP)
  {
    if (CurrID == Clump)
      return TRUE;

    CurrID = VarsCmd.pAllClumps[CurrID].Link;
  }

  return FALSE;
}


UBYTE cCmdIsQSane(CLUMP_Q * Queue)
{
  CLUMP_ID Head, Tail;
  CLUMP_REC * pHead;

  if (Queue == NULL)
  {
    NXT_BREAK;
    return FALSE;
  }

  Head = Queue->Head;
  Tail = Queue->Tail;

  if (Head == NOT_A_CLUMP && cCmdIsClumpIDSane(Tail))
    return FALSE;

  if (cCmdIsClumpIDSane(Head) && Tail == NOT_A_CLUMP)
    return FALSE;

  if (cCmdIsClumpIDSane(Head) && cCmdIsClumpIDSane(Tail))
  {
    pHead = &(VarsCmd.pAllClumps[Head]);

    //!!! More comprehensive queue tests could go here

    //Check for mislinked head if there are at least two queue members
    if (Head != Tail && pHead->Link == NOT_A_CLUMP)
      return FALSE;
  }

    return TRUE;
}

//
// Mutex queuing functions
//

NXT_STATUS cCmdAcquireMutex(MUTEX_Q * Mutex, CLUMP_ID Clump)
{
  NXT_STATUS Status = NO_ERR;

  NXT_ASSERT(Mutex != NULL && cCmdIsClumpIDSane(Clump));

  if (Mutex->Owner == NOT_A_CLUMP)
  {
    //Mutex is open, so just take it
    Mutex->Owner = Clump;

    NXT_ASSERT(Mutex->WaitQ.Head == NOT_A_CLUMP && Mutex->WaitQ.Tail == NOT_A_CLUMP);
  }
  else
  {
    //Mutex is reserved by someone else, take self off RunQ and add to WaitQ
    cCmdDeQClump(&(VarsCmd.RunQ), Clump);
    cCmdEnQClump(&(Mutex->WaitQ), Clump);
    Status = CLUMP_SUSPEND;
  }

  NXT_ASSERT(cCmdIsQSane(&(Mutex->WaitQ)));

  return (Status);
}


NXT_STATUS cCmdReleaseMutex(MUTEX_Q * Mutex, CLUMP_ID Clump)
{
  NXT_ASSERT(Mutex != NULL);
  //!!! don't actually need to pass in Owner clump, but provides nice error checking for now
  // Might want to return an error/warning if we see a Release on an free mutex, though...
  NXT_ASSERT(Clump != NOT_A_CLUMP && Mutex->Owner == Clump);

  //Always set new Owner to WaitQ's Head, since NOT_A_CLUMP means mutex is free
  Mutex->Owner = Mutex->WaitQ.Head;

  if (Mutex->Owner != NOT_A_CLUMP)
  {
    cCmdDeQClump(&(Mutex->WaitQ), Mutex->Owner);
    cCmdEnQClump(&(VarsCmd.RunQ), Mutex->Owner);
  }

  NXT_ASSERT(cCmdIsQSane(&(Mutex->WaitQ)));
  NXT_ASSERT(cCmdIsQSane(&(VarsCmd.RunQ)));

  return (NO_ERR);
}


NXT_STATUS cCmdSchedDependents(CLUMP_ID Clump, SWORD Begin, SWORD End)
{
  CLUMP_ID CurrDepClumpID;
  SWORD i;

  //Begin and End specify range of CLUMP_IDs in dependent list to schedule
  //If either equals -1, both should equal -1, and no dependents will be scheduled
  //Else schedule specified subset offset from pDependents

  //Check for valid args
  NXT_ASSERT(cCmdIsClumpIDSane(Clump));
  NXT_ASSERT((Begin >= 0 && End >= 0 && End < VarsCmd.pAllClumps[Clump].DependentCount)
          || (Begin == -1 && End == -1));

  //If non-empty range
  if (Begin != -1 || End != -1)
  {
    //update dependents, scheduling if their CurrFireCount reaches 0
    for (i = Begin; i <= End; i++)
    {
      CurrDepClumpID = VarsCmd.pAllClumps[Clump].pDependents[i];

      NXT_ASSERT(cCmdIsClumpIDSane(CurrDepClumpID));

      VarsCmd.pAllClumps[CurrDepClumpID].CurrFireCount--;

      if (VarsCmd.pAllClumps[CurrDepClumpID].CurrFireCount == 0)
        cCmdEnQClump(&(VarsCmd.RunQ), CurrDepClumpID);
    }
  }

  return (NO_ERR);
}


NXT_STATUS cCmdSchedDependent(CLUMP_ID Clump, CLUMP_ID TargetClump)
{
  //TargetClump specifies the clump number of the target to schedule explicitly.

  //Check for valid args
  NXT_ASSERT(cCmdIsClumpIDSane(Clump));
  NXT_ASSERT(cCmdIsClumpIDSane(TargetClump));

  VarsCmd.pAllClumps[TargetClump].CurrFireCount--;
  if (VarsCmd.pAllClumps[TargetClump].CurrFireCount == 0)
    cCmdEnQClump(&(VarsCmd.RunQ), TargetClump);

  return (NO_ERR);
}


UBYTE cCmdIsClumpIDSane(CLUMP_ID Clump)
{
  if (Clump < VarsCmd.AllClumpsCount)
    return TRUE;
  else
    return FALSE;
}


//
// Memory pool management functions
//
void cCmdInitPool(void)
{
  ULONG i;

  //VarsCmd.Pool is a UBYTE pointer to ULONG array
  //This was done to enforce portable alignment.
  VarsCmd.Pool = (UBYTE*)(IOMapCmd.MemoryPool);

  for (i = 0; i < (POOL_MAX_SIZE / 4); i++)
    ((SLONG*)(POOL_START))[i] = 0xDEADBEEF;

  VarsCmd.PoolSize = 0;
}


NXT_STATUS cCmdDSArrayAlloc(DS_ELEMENT_ID DSElementID, UWORD Offset, UWORD NewCount)
{
  NXT_STATUS Status = NO_ERR;
  UWORD DVIndex;
  UWORD OldCount;
  UWORD i;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));

  //Only arrays are valid here
  //!!! Recommended to upgrade NXT_ASSERT to ERR_INSTR return
  NXT_ASSERT(cCmdDSType(DSElementID) == TC_ARRAY);

  DVIndex = cCmdGetDVIndex(DSElementID, Offset);
  OldCount = DV_ARRAY[DVIndex].Count;

  Status = cCmdDVArrayAlloc(DVIndex, NewCount);
  if (Status < NO_ERR)
    return Status;

  if (OldCount > NewCount)
  {
    //Free dope vectors for sub-arrays.
    for (i = NewCount; i < OldCount; i++)
    {
      Status = cCmdFreeSubArrayDopeVectors(INC_ID(DSElementID), ARRAY_ELEM_OFFSET(DVIndex, i));
      if (IS_ERR(Status))
        return Status;
    }
  }
  else if (OldCount < NewCount)
  {
    //Alloc dope vectors for sub-arrays. Set up DVIndexes
    for (i = OldCount; i < NewCount; i++)
    {
      Status = cCmdAllocSubArrayDopeVectors(INC_ID(DSElementID), ARRAY_ELEM_OFFSET(DVIndex, i));
      if (IS_ERR(Status))
        return Status;
    }
  }

  NXT_ASSERT(cCmdVerifyMemMgr());

  return Status;
}

NXT_STATUS cCmdDVArrayAlloc(DV_INDEX DVIndex, UWORD NewCount)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE *pData;
  UWORD ArraySize, InplaceSize;
  UWORD NextDVIndex;
  UWORD OldCount;

  OldCount = DV_ARRAY[DVIndex].Count;

  if (OldCount == NewCount)
  {
    //Nothing to alloc. Return.
    return Status;
  }
  else if (OldCount > NewCount)
  {
    //Already have the space. Shrink inplace.
    DV_ARRAY[DVIndex].Count = NewCount;
        return Status;
  }
  else // need to grow array
  {
    //Calculate new array size
    ArraySize = NewCount * DV_ARRAY[DVIndex].ElemSize;

    //Try growing inplace
    // If the Offset == NOT_AN_OFFSET then the array has never been allocated and can't grow inplace.
    if (DV_ARRAY[DVIndex].Offset != NOT_AN_OFFSET)
    {
      //Get pointer to next dope vector in dataspace
      if (DV_ARRAY[DVIndex].Link != NOT_A_DS_ID)
      {
        NextDVIndex = DV_ARRAY[DVIndex].Link;
        InplaceSize = DV_ARRAY[NextDVIndex].Offset - DV_ARRAY[DVIndex].Offset;
      }
      else
      {
        //Last element in dataspace.
        NXT_ASSERT(DVIndex == VarsCmd.MemMgr.Tail);
        InplaceSize = VarsCmd.DataspaceSize - DV_ARRAY[DVIndex].Offset;
      }

      if (ArraySize <= InplaceSize)
      {
        DV_ARRAY[DVIndex].Count = NewCount;
            return Status;
      }
    }

    //Can't grow inplace, have to allocate new space

    //Make sure we properly align for type
    //!!! This could also overflow memory (make PoolSize > POOL_MAX_SIZE) if we're within 3 bytes of the end.
    //     I don't think it matters because if it does happend, we'll trigger the ERR_MEM below and compact.
    //     During compaction, we'll reclaim these unused bytes.
    //!!! Aligning beginning of ALL arrays to 4 byte address
    ALIGN_TO_MOD(VarsCmd.PoolSize, SIZE_ULONG);
    ALIGN_TO_MOD(VarsCmd.DataspaceSize, SIZE_ULONG);

    if (VarsCmd.PoolSize + ArraySize >= POOL_MAX_SIZE)
    {
      //Not enough memory available
      return ERR_MEM;
    }

    //Get data from end of pool
    pData = VarsCmd.Pool + VarsCmd.PoolSize;
    //Grow pool and dataspace
    VarsCmd.PoolSize += ArraySize;
    VarsCmd.DataspaceSize += ArraySize;

    //Move Array Data
    memmove(pData, VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset, (UWORD)(DV_ARRAY[DVIndex].ElemSize * DV_ARRAY[DVIndex].Count));
    //!!! Clear mem so we make sure we don't reference stale data. Not strictly needed.
    memset(VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset, 0xFF, (UWORD)(DV_ARRAY[DVIndex].ElemSize * DV_ARRAY[DVIndex].Count));

    //Update dope vector
    DV_ARRAY[DVIndex].Offset = pData - VarsCmd.pDataspace;
    DV_ARRAY[DVIndex].Count = NewCount;

    //Move dope vector to end of MemMgr list
    Status = cCmdMemMgrMoveToTail(DVIndex);
    if (IS_ERR(Status))
      return Status;

    NXT_ASSERT(cCmdVerifyMemMgr());
  }

  return Status;
}


//!!! Recursive function
NXT_STATUS cCmdAllocSubArrayDopeVectors(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  // Walks a single array element to see if it contains arrays
  // For any array it finds, a dope vector is allocated and the DVIndex is placed in the dataspace for the parent array.
  // This is a non-recursive function. It only walks the immediate array element.
  // DSElementID - ID of array sub-entry
  // Offset - offset to array element in dataspace
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode;
  DV_INDEX DVIndex;
  UWORD i;
  UWORD DVIndexOffset; //Offset to DVIndex field that points to the DopeVector from pDataspace
  UWORD LoopCount = 1;
  UWORD ElemSize;

  for (i = 0; i < LoopCount; i++)
  {
    TypeCode = cCmdDSType((DS_ELEMENT_ID)(DSElementID + i));
    if (TypeCode == TC_CLUSTER)
    {
      LoopCount += cCmdClusterCount(DSElementID);
    }
    else if (TypeCode == TC_ARRAY)
    {
      //!!! ElemSize is a static value, but we don't have anywhere we put it (another TOC sub-entry?)
      //     It'd be nice to not have to recalculate it.
      ElemSize = cCmdCalcArrayElemSize((DS_ELEMENT_ID)(DSElementID + i));
      DVIndexOffset = VarsCmd.pDataspaceTOC[DSElementID + i].DSOffset + Offset;
      Status = cCmdAllocDopeVector(&DVIndex, ElemSize, DVIndexOffset);
      if (IS_ERR(Status))
        return Status;

      *((UWORD *)(VarsCmd.pDataspace + DVIndexOffset)) = DVIndex;
    }
  }

  return Status;
}


//!!! Recursive function
NXT_STATUS cCmdFreeSubArrayDopeVectors(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  // Walks a single array element to see if it contains arrays
  // Frees all dope vectors associated with the array element.
  // Recursively deletes sub-arrays.
  // DSElementID - ID of array sub-entry
  // Offset - offset to array element in dataspace
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode;
  DV_INDEX DVIndex;
  UWORD i, Count;

  TypeCode = cCmdDSType(DSElementID);

  if (TypeCode == TC_ARRAY)
  {
    DVIndex = cCmdGetDVIndex(DSElementID, Offset);

    NXT_ASSERT(DVIndex < DV_ARRAY[0].Count);

    Count = DV_ARRAY[DVIndex].Count;
    //Recur on sub-elements
    for (i = 0; i < Count; i++)
    {
      Status = cCmdFreeSubArrayDopeVectors(INC_ID(DSElementID), ARRAY_ELEM_OFFSET(DVIndex, i));
      if (IS_ERR(Status))
        return Status;
    }

    //Free Dope Vector
    Status = cCmdFreeDopeVector(DVIndex);
  }
  else if (TypeCode == TC_CLUSTER)
  {
    Count = cCmdClusterCount(DSElementID);
    DSElementID = INC_ID(DSElementID);
    //Recur on sub-elements
    for (i = 0; i < Count; i++)
    {
      Status = cCmdFreeSubArrayDopeVectors((DS_ELEMENT_ID)(DSElementID + i), Offset);
      if (IS_ERR(Status))
        return Status;
    }
  }

  return Status;
}


NXT_STATUS cCmdAllocDopeVector(DV_INDEX *pIndex, UWORD ElemSize, UWORD BackPtr)
{
  NXT_STATUS Status = NO_ERR;

  if (VarsCmd.MemMgr.FreeHead == NOT_A_DS_ID)
  {
    //No free DVs. Need to grow DopeVector array.
    Status = cCmdGrowDopeVectorArray(DV_ARRAY_GROWTH_COUNT);
    if (IS_ERR(Status))
      return Status;
  }

  NXT_ASSERT(VarsCmd.MemMgr.FreeHead != NOT_A_DS_ID);

  //Remove DV from free list
  *pIndex = VarsCmd.MemMgr.FreeHead;
  VarsCmd.MemMgr.FreeHead = DV_ARRAY[VarsCmd.MemMgr.FreeHead].Link;
  //Add DV to tail of MemMgr list
  Status = cCmdMemMgrInsertAtTail(*pIndex);

  //Initialize values
  DV_ARRAY[*pIndex].Offset = NOT_AN_OFFSET;
  DV_ARRAY[*pIndex].ElemSize = ElemSize;
  DV_ARRAY[*pIndex].Count = 0;
  DV_ARRAY[*pIndex].BackPtr = BackPtr;

  NXT_ASSERT(cCmdVerifyMemMgr());

  return Status;
}

//
//cCmdFreeDopeVector() - Open up a spot in the DopeVectorArray for future use
// The DopeVectorArray doesn't shrink when arrays (and their dope vectors) are deleted.
// Instead they're pushed on the free list and the array stays the same size.
// Future allocations check the free list before resorting to cCmdGrowDopeVectorArray()
//
NXT_STATUS cCmdFreeDopeVector(DV_INDEX DVIndex)
{
  NXT_STATUS Status = NO_ERR;
  DV_INDEX i;

  //Bounds check
  NXT_ASSERT(DVIndex < DV_ARRAY[0].Count);

  //Reset dope vector fields
  DV_ARRAY[DVIndex].Count = 0;
  DV_ARRAY[DVIndex].ElemSize = 0;
  DV_ARRAY[DVIndex].Offset = NOT_AN_OFFSET;
  DV_ARRAY[DVIndex].BackPtr = NOT_AN_OFFSET;

  //Remove from MemMgr list
  if (DVIndex == VarsCmd.MemMgr.Head)
  {
    VarsCmd.MemMgr.Head = DV_ARRAY[DVIndex].Link;
  }
  else
  {
    //Walk MemMgr list to find previous.
    //!!! Could speed this up if MemMgr list were doubly linked
    for (i = VarsCmd.MemMgr.Head; i != NOT_A_DS_ID; i = DV_ARRAY[i].Link)
    {
      if (DV_ARRAY[i].Link == DVIndex)
      {
        DV_ARRAY[i].Link = DV_ARRAY[DVIndex].Link;
        if (DVIndex == VarsCmd.MemMgr.Tail)
          VarsCmd.MemMgr.Tail = i;
        break;
      }
    }
    //Make sure we found the previous DV, otherwise this DV was not in the the list (already freed?)
    NXT_ASSERT(i != NOT_A_DS_ID);
  }

  //Push on to free list
  DV_ARRAY[DVIndex].Link = VarsCmd.MemMgr.FreeHead;
  VarsCmd.MemMgr.FreeHead = DVIndex;

  NXT_ASSERT(cCmdVerifyMemMgr());

  return Status;
}

//
//cCmdGrowDopeVectorArray() - expand DopeVectorArray to be able to track more dataspace arrays
//
NXT_STATUS cCmdGrowDopeVectorArray(UWORD NewNodesCount)
{
  NXT_STATUS Status = NO_ERR;
  UWORD ArraySize;
  UWORD OldCount, NewCount, i;
  UBYTE * pData;

  NXT_ASSERT(cCmdVerifyMemMgr());

  OldCount = DV_ARRAY[0].Count;
  NewCount = OldCount + NewNodesCount;
  NXT_ASSERT(NewCount > OldCount);

  ArraySize = DV_ARRAY[0].ElemSize * NewCount;

  //!!! Aligning beginning of ALL arrays to 4 byte address
  ALIGN_TO_MOD(VarsCmd.PoolSize, SIZE_ULONG);
  ALIGN_TO_MOD(VarsCmd.DataspaceSize, SIZE_ULONG);

  if (VarsCmd.PoolSize + ArraySize >= POOL_MAX_SIZE)
  {
    //Not enough memory available
    return ERR_MEM;
  }

  //Get data from end of pool
  pData = VarsCmd.Pool + VarsCmd.PoolSize;
  //Grow pool and dataspace
  VarsCmd.PoolSize += ArraySize;
  VarsCmd.DataspaceSize += ArraySize;

  //Move DopeVector Array
  memmove(pData, (UBYTE *)VarsCmd.MemMgr.pDopeVectorArray, (UWORD)(DV_ARRAY[0].ElemSize * DV_ARRAY[0].Count));

  //Update MemMgr pointer
  VarsCmd.MemMgr.pDopeVectorArray = (DOPE_VECTOR *)pData;
  IOMapCmd.OffsetDVA = (UWORD)((ULONG)(VarsCmd.MemMgr.pDopeVectorArray) - (ULONG)&(IOMapCmd));

  //Update dope vector
  DV_ARRAY[0].Offset = pData - VarsCmd.pDataspace;
  DV_ARRAY[0].Count = NewCount;

  //Add new DopeVectors to free list
  //Push in reverse order so they get popped in order (mostly for ease of debugging)
  for (i = NewCount - 1; i >= OldCount; i--)
  {
    DV_ARRAY[i].Offset = 0xFFFF;
    DV_ARRAY[i].ElemSize = 0;
    DV_ARRAY[i].Count = 0;
    DV_ARRAY[i].BackPtr = 0xFFFF;
    DV_ARRAY[i].Link = VarsCmd.MemMgr.FreeHead;
    VarsCmd.MemMgr.FreeHead = i;
  }

  //Move dope vector to end of MemMgr list
  Status = cCmdMemMgrMoveToTail(0);

  NXT_ASSERT(cCmdVerifyMemMgr());

  return Status;
}

NXT_STATUS cCmdCompactDopeVectorArray(void)
{
  //!!! Not implemented. Needs BackPtr support.
  NXT_BREAK;
  return ERR_ARG;
}


UWORD cCmdCalcArrayElemSize(DS_ELEMENT_ID DSElementID)
{
  TYPE_CODE TypeCode;
  UWORD SizeOfType;
  UWORD i;
  UWORD LoopCount = 1;
  UWORD Size = 0;
  UWORD Alignment = 0;

  NXT_ASSERT(cCmdDSType(DSElementID) == TC_ARRAY);

  DSElementID = INC_ID(DSElementID);
  for (i = 0; i < LoopCount; i++)
  {
    TypeCode = cCmdDSType((DS_ELEMENT_ID)(DSElementID + i));
    if (TypeCode == TC_CLUSTER)
    {
      LoopCount += cCmdClusterCount((DS_ELEMENT_ID)(DSElementID + i));
    }
    else
    {
      SizeOfType = cCmdSizeOf(TypeCode);
      ALIGN_TO_MOD(Size, SizeOfType);
      Size += SizeOfType;
      if (SizeOfType > Alignment)
        Alignment = SizeOfType;
    }
  }
  ALIGN_TO_MOD(Size, Alignment);

  return Size;
}


NXT_STATUS cCmdMemMgrMoveToTail(DV_INDEX DVIndex)
{
  DV_INDEX i;

  //Bounds check
  NXT_ASSERT(DVIndex < DV_ARRAY[0].Count);

  //Short circut if its already at the tail
  if (DVIndex == VarsCmd.MemMgr.Tail)
    return NO_ERR;

  if (DVIndex == VarsCmd.MemMgr.Head)
    VarsCmd.MemMgr.Head = DV_ARRAY[DVIndex].Link;
  else
  {
    //Walk MemMgr list to find previous.
    //!!! Could speed this up if MemMgr list were doubly linked
    for (i = VarsCmd.MemMgr.Head; i != NOT_A_DS_ID; i = DV_ARRAY[i].Link)
    {
      if (DV_ARRAY[i].Link == DVIndex)
      {
        DV_ARRAY[i].Link = DV_ARRAY[DVIndex].Link;
        break;
      }
    }
    //Make sure we found the previous DV, otherwise this DV was not in the the list
    NXT_ASSERT(i != NOT_A_DS_ID);
  }

  DV_ARRAY[DVIndex].Link = NOT_A_DS_ID;
  DV_ARRAY[VarsCmd.MemMgr.Tail].Link = DVIndex;
  VarsCmd.MemMgr.Tail = DVIndex;

  NXT_ASSERT(cCmdVerifyMemMgr());

  return NO_ERR;
}


NXT_STATUS cCmdMemMgrInsertAtTail(DV_INDEX DVIndex)
{
  //Bounds check
  NXT_ASSERT(DVIndex < DV_ARRAY[0].Count);

  DV_ARRAY[VarsCmd.MemMgr.Tail].Link = DVIndex;
  VarsCmd.MemMgr.Tail = DVIndex;
  DV_ARRAY[DVIndex].Link = NOT_A_DS_ID;

  NXT_ASSERT(cCmdVerifyMemMgr());

  return NO_ERR;
}


UBYTE cCmdVerifyMemMgr()
{
  DV_INDEX i;
  UWORD CurrOffset = 0;
  UWORD PrevOffset = 0;
  UWORD DVCount = 0;

  //Make sure the MemMgr list is properly sorted in ascending offset order
  for (i = VarsCmd.MemMgr.Head; i != NOT_A_DS_ID; i = DV_ARRAY[i].Link)
  {
    CurrOffset = DV_ARRAY[i].Offset;

    if (CurrOffset != 0xFFFF)
    {
      if (PrevOffset > CurrOffset)
        return FALSE;

      PrevOffset = CurrOffset;
    }

    if (DV_ARRAY[i].Link == NOT_A_DS_ID && i != VarsCmd.MemMgr.Tail)
      return FALSE;

    DVCount++;
  }

  for (i = VarsCmd.MemMgr.FreeHead; i != NOT_A_DS_ID; i = DV_ARRAY[i].Link)
  {
    DVCount++;
  }

  //Make sure the # of dope vectors = # used + # free
  if (DVCount != DV_ARRAY[0].Count)
    return FALSE;

  return TRUE;
}


NXT_STATUS cCmdDSCompact(void)
{
  NXT_STATUS Status = NO_ERR;

  DV_INDEX CurrIndex;
  UWORD NewOffset;
  UWORD CurrOffset;
  UWORD Size;
  UWORD DeltaDSSize;
  UWORD TempOffset, TempSize;

#if VM_BENCHMARK
  ULONG StartTime, TotalTime;

  VarsCmd.CompactionCount++;
  VarsCmd.LastCompactionTick = IOMapCmd.Tick - VarsCmd.StartTick;

  StartTime = dTimerRead();
#endif

  NXT_ASSERT(cCmdVerifyMemMgr());

  NewOffset = VarsCmd.DSStaticSize;
  for (CurrIndex = VarsCmd.MemMgr.Head; CurrIndex != NOT_A_DS_ID; CurrIndex = DV_ARRAY[CurrIndex].Link)
  {
    //Align NewOffset for array to 4 byte address.
    ALIGN_TO_MOD(NewOffset, SIZE_ULONG);

    CurrOffset = DV_ARRAY[CurrIndex].Offset;
    if (CurrOffset != NOT_AN_OFFSET)
    {
      Size = DV_ARRAY[CurrIndex].ElemSize * DV_ARRAY[CurrIndex].Count;
      if (CurrOffset != NewOffset)
      {
        NXT_ASSERT(NewOffset < CurrOffset);
        memmove(VarsCmd.pDataspace + NewOffset, VarsCmd.pDataspace + CurrOffset, Size);

        //  Clear mem to make stale data references more obvious while debugging.
        //  Correct for overlapping memory regions (make sure we don't clear what we just moved).
        //!!! Clearing step not strictly necessary, so it could be optimized out
        if (NewOffset + Size > CurrOffset)
        {
          TempOffset = NewOffset + Size;
          TempSize = Size - (TempOffset - CurrOffset);
        }
        else
        {
          TempOffset = CurrOffset;
          TempSize = Size;
        }
        memset(VarsCmd.pDataspace + TempOffset, 0xFF, TempSize);

        //Update pDopeVectorArray if we move the dope vector array
        if (CurrIndex == 0)
        {
          VarsCmd.MemMgr.pDopeVectorArray = (DOPE_VECTOR *)(VarsCmd.pDataspace + NewOffset);
          IOMapCmd.OffsetDVA = (UWORD)((ULONG)(VarsCmd.MemMgr.pDopeVectorArray) - (ULONG)&(IOMapCmd));
        }

        //Update offset in DV Array
        DV_ARRAY[CurrIndex].Offset = NewOffset;
      }

      NewOffset += Size;
    }
  }

  DeltaDSSize = VarsCmd.DataspaceSize - NewOffset;

  VarsCmd.PoolSize -= DeltaDSSize;
  VarsCmd.DataspaceSize -= DeltaDSSize;

  NXT_ASSERT(cCmdVerifyMemMgr());

#if VM_BENCHMARK
  TotalTime = dTimerRead() - StartTime;

  if (TotalTime > VarsCmd.MaxCompactionTime)
    VarsCmd.MaxCompactionTime = TotalTime;
#endif

  return Status;
}


//
// Message Queue functions
//

NXT_STATUS cCmdMessageWrite(UWORD QueueID, UBYTE * pData, UWORD Length)
{
  NXT_STATUS Status = NO_ERR;

  if (pData == NULL)
    return ERR_ARG;

  if (QueueID >= MESSAGE_QUEUE_COUNT)
    return ERR_INVALID_QUEUE;

  if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
    return ERR_NO_PROG;

  //Can't accept oversize messages because we treat them as strings (truncation would remove null termination)
  if (Length > MAX_MESSAGE_SIZE)
    return ERR_INVALID_SIZE;

  if (IS_DV_INDEX_SANE(GET_WRITE_MSG(QueueID)))
  {
    //A message is already there, the queue is full
    NXT_ASSERT(VarsCmd.MessageQueues[QueueID].WriteIndex == VarsCmd.MessageQueues[QueueID].ReadIndex);

    //Bump read index, drop existing message to make room for our new incoming message
    VarsCmd.MessageQueues[QueueID].ReadIndex = (VarsCmd.MessageQueues[QueueID].ReadIndex + 1) % MESSAGES_PER_QUEUE;
  }
  else
  {
    //Allocate dope vector for message
    Status = cCmdAllocDopeVector(&GET_WRITE_MSG(QueueID), 1, NOT_AN_OFFSET);
    if (IS_ERR(Status))
      return Status;
  }

  //Allocate storage for message
  Status = cCmdDVArrayAlloc(GET_WRITE_MSG(QueueID), Length);
  if (IS_ERR(Status))
  {
    //Clear the dope vector for the message, since we're unable to put a message there.
    cCmdFreeDopeVector(GET_WRITE_MSG(QueueID));
    SET_WRITE_MSG(QueueID, NOT_A_DS_ID);
    return Status;
  }

  //Copy message
  memmove(cCmdDVPtr(GET_WRITE_MSG(QueueID)), pData, Length);

  //Advance write index
  VarsCmd.MessageQueues[QueueID].WriteIndex = (VarsCmd.MessageQueues[QueueID].WriteIndex + 1) % MESSAGES_PER_QUEUE;

  return Status;
}


NXT_STATUS cCmdMessageGetSize(UWORD QueueID, UWORD * Size)
{
  DV_INDEX ReadDVIndex;

  if (Size == NULL)
    return (ERR_ARG);

  if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
  {
    *Size = 0;
    return (ERR_NO_PROG);
  }

  if (QueueID >= MESSAGE_QUEUE_COUNT)
  {
    *Size = 0;
    return (ERR_INVALID_QUEUE);
  }

  ReadDVIndex = GET_READ_MSG(QueueID);

  if (IS_DV_INDEX_SANE(ReadDVIndex))
  {
    *Size = (DV_ARRAY[ReadDVIndex].Count);
    return (NO_ERR);
  }
  else
  {
    *Size = 0;
    return (STAT_MSG_EMPTY_MAILBOX);
  }
}


NXT_STATUS cCmdMessageRead(UWORD QueueID, UBYTE * pBuffer, UWORD Length, UBYTE Remove)
{
  NXT_STATUS Status = NO_ERR;
  DV_INDEX ReadDVIndex;

  if (pBuffer == NULL)
    return (ERR_ARG);

  if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
    return (ERR_NO_PROG);

  if (QueueID >= MESSAGE_QUEUE_COUNT)
    return (ERR_INVALID_QUEUE);

  ReadDVIndex = GET_READ_MSG(QueueID);

  if (IS_DV_INDEX_SANE(ReadDVIndex))
  {
    //If Buffer doesn't have room for the entire message,
    //don't risk incomplete string floating around
    if (Length < DV_ARRAY[ReadDVIndex].Count)
      return (ERR_INVALID_SIZE);

    //Copy message
    memmove(pBuffer, cCmdDVPtr(ReadDVIndex), DV_ARRAY[ReadDVIndex].Count);

    if (Remove)
    {
      //Free memory used by message
      Status = cCmdFreeDopeVector(ReadDVIndex);
      if (IS_ERR(Status))
        return Status;

      SET_READ_MSG(QueueID, NOT_A_DS_ID);

      //Advance read index
      VarsCmd.MessageQueues[QueueID].ReadIndex = (VarsCmd.MessageQueues[QueueID].ReadIndex + 1) % MESSAGES_PER_QUEUE;
    }
  }
  else
  {
    //No message to read, message Queue is empty
    NXT_ASSERT(VarsCmd.MessageQueues[QueueID].ReadIndex == VarsCmd.MessageQueues[QueueID].WriteIndex);

    return (STAT_MSG_EMPTY_MAILBOX);
  }

  return Status;
}


//
// Dataspace Support functions
//

UBYTE cCmdIsDSElementIDSane(DS_ELEMENT_ID Index)
{
  if (Index < VarsCmd.DataspaceCount)
    return TRUE;
  else
    return FALSE;
}

TYPE_CODE cCmdDSType(DS_ELEMENT_ID DSElementID)
{
  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));

  return (VarsCmd.pDataspaceTOC[DSElementID].TypeCode);
}


void * cCmdResolveDataArg(DATA_ARG DataArg, UWORD Offset, TYPE_CODE * TypeCode)
{
  UBYTE ModuleID;
  UWORD FieldID;
  void * ret_val = NULL;

  //
  //!!! DATA_ARG masking system only for internal c_cmd use!
  //    All normal bytecode arguments should go through top if() block.
  //

  if (DataArg <= (DATA_ARG)(DATA_ARG_ADDR_MASK) )
  {
    NXT_ASSERT(cCmdIsDSElementIDSane(DataArg));
    ret_val = cCmdDSPtr(DataArg, Offset);
    if (TypeCode)
      *TypeCode = VarsCmd.pDataspaceTOC[DataArg].TypeCode;
  }
  else if (DataArg & ~((DATA_ARG)(DATA_ARG_ADDR_MASK)))
  {
    //DataArg refers to a field in the IO map
    ModuleID = (UBYTE)((DataArg >> 9) & 0x001F);
    FieldID = (UWORD)(DataArg & 0x01FF);

    //!!! Preliminary bounds check -- still could allow invalid combos through
    if (ModuleID > MOD_OUTPUT || FieldID >= IO_OUT_FIELD_COUNT)
    {
      NXT_BREAK;
      return NULL;
    }

    ret_val = IO_PTRS[ModuleID][FieldID];
    if (TypeCode)
      *TypeCode = IO_TYPES[ModuleID][FieldID];
  }

  //!!! Caller beware! If DataArg isn't sane, ret_val may be out of range or NULL!
  return ret_val;
}

void cCmdSetVal(void * pVal, TYPE_CODE TypeCode, ULONG NewVal)
{

  if (pVal)
  {
    switch (TypeCode)
    {
      case TC_ULONG:
      case TC_SLONG:
      {
        *(ULONG*)pVal = NewVal;
      }
      break;

      case TC_UWORD:
      case TC_SWORD:
      {
        *(UWORD*)pVal = (UWORD)NewVal;
      }
      break;

      case TC_UBYTE:
      case TC_SBYTE:
      {
        *(UBYTE*)pVal = (UBYTE)NewVal;
      }
      break;
    }
  }

  return;
}


ULONG cCmdGetVal(void * pVal, TYPE_CODE TypeCode)
{
  if (pVal)
  {
    switch (TypeCode)
    {
      case TC_ULONG:
      {
        return (ULONG)(*(ULONG*)pVal);
      }

      case TC_SLONG:
      {
        return (SLONG)(*(SLONG*)pVal);
      }

      case TC_UWORD:
      {
        return (UWORD)(*(UWORD*)pVal);
      }

      case TC_SWORD:
      {
        return (SWORD)(*(SWORD*)pVal);
      }

      case TC_UBYTE:
      {
        return (UBYTE)(*(UBYTE*)pVal);
      }

      case TC_SBYTE:
      {
        return (SBYTE)(*(SBYTE*)pVal);
      }

      default:
      break;
    }
  }

  //!!! Default return value places responsibility on caller to use this function wisely
  return 0;
}


UWORD cCmdSizeOf(TYPE_CODE TypeCode)
{
  //!!! Why not use a lookup table? No particular reason...
  switch(TypeCode)
  {
  case TC_ULONG:
    return SIZE_ULONG;
  case TC_SLONG:
    return SIZE_SLONG;
  case TC_UWORD:
    return SIZE_UWORD;
  case TC_SWORD:
    return SIZE_SWORD;
  case TC_UBYTE:
    return SIZE_UBYTE;
  case TC_SBYTE:
    return SIZE_SBYTE;
  case TC_MUTEX:
    return SIZE_MUTEX;
  case TC_ARRAY:
    //Arrays have a 2-byte structure in the dataspace for the DVIndex
    return SIZE_UWORD;
  case TC_CLUSTER:
      default:
  return 0;
  }
}


void* cCmdDSPtr(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  void * pDSItem;
  DV_INDEX DVIndex;
  TYPE_CODE TypeCode;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));

  TypeCode = cCmdDSType(DSElementID);
  if (TypeCode == TC_ARRAY)
  {
    //!!! Empty arrays return NULL.
    if (cCmdArrayCount(DSElementID, Offset) == 0)
      pDSItem = NULL;
    else
    {
      DVIndex = cCmdGetDVIndex(DSElementID, Offset);
      pDSItem = (VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset);
    }
  }
  else if (TypeCode == TC_CLUSTER)
  {
    NXT_ASSERT(cCmdClusterCount(DSElementID) != 0)

    //Returning pointer to the first element in the cluster
    pDSItem = cCmdDSPtr(INC_ID(DSElementID), Offset);
  }
  else
    pDSItem = (VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[DSElementID].DSOffset + Offset);

  NXT_ASSERT((UBYTE*)pDSItem < POOL_SENTINEL);

  return pDSItem;
}

void* cCmdDVPtr(DV_INDEX DVIndex)
{
  NXT_ASSERT(IS_DV_INDEX_SANE(DVIndex));
  return (VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset);
}


//!!! Recursive function
DS_ELEMENT_ID cCmdNextDSElement(DS_ELEMENT_ID CurrID)
{
  DS_ELEMENT_ID NextID;
  TYPE_CODE CurrType;
  UWORD ClusterCount, i;

  NXT_ASSERT(cCmdIsDSElementIDSane(CurrID));

  NextID = CurrID + 1;

  if (!cCmdIsDSElementIDSane(NextID))
    return NOT_A_DS_ID;

  CurrType = cCmdDSType(CurrID);

  if (CurrType == TC_ARRAY)
  {
    //Arrays contain two elements. Advance past the second one.
    NextID = cCmdNextDSElement(NextID);
  }
  else if (CurrType == TC_CLUSTER)
  {
    ClusterCount = cCmdClusterCount(CurrID);
    for (i = 0; i < ClusterCount; i++)
    {
      NextID = cCmdNextDSElement(NextID);
    }
  }

  return NextID;
}


//!!! Recursive function
UBYTE cCmdCompareDSType(DS_ELEMENT_ID DSElementID1, DS_ELEMENT_ID DSElementID2)
{
  TYPE_CODE Type1, Type2;
  UWORD i, Count1, Count2;

  Type1 = cCmdDSType(DSElementID1);
  Type2 = cCmdDSType(DSElementID2);

  if (Type1 != Type2)
    return FALSE;

  if (Type1 == TC_CLUSTER)
  {
    Count1 = cCmdClusterCount(DSElementID1);
    Count2 = cCmdClusterCount(DSElementID2);

    if(Count1 != Count2)
      return FALSE;

    DSElementID1 = INC_ID(DSElementID1);
    DSElementID2 = INC_ID(DSElementID2);

    for (i = 0; i < Count1; i++)
    {
      if (!cCmdCompareDSType(DSElementID1, DSElementID2))
        return FALSE;

      DSElementID1 = cCmdNextDSElement(DSElementID1);
      DSElementID2 = cCmdNextDSElement(DSElementID2);
    }
  }
  else if (Type1 == TC_ARRAY)
  {
    if (!cCmdCompareDSType(INC_ID(DSElementID1), INC_ID(DSElementID2)))
      return FALSE;
  }

  return TRUE;
}


//!!! Recursive function
UWORD cCmdCalcFlattenedSize(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  UWORD Size = 0;
  TYPE_CODE TypeCode;
  DV_INDEX DVIndex;
  UWORD i;
  UWORD Count;

  TypeCode = cCmdDSType(DSElementID);

  if (TypeCode == TC_ARRAY)
  {
    DVIndex = cCmdGetDVIndex(DSElementID, Offset);

    DSElementID = INC_ID(DSElementID);
    TypeCode = cCmdDSType(DSElementID);

    if (!IS_AGGREGATE_TYPE(TypeCode))
    {
      //Short circuit recursive calculation if our array sub-type is a scalar
      Size += DV_ARRAY[DVIndex].ElemSize * DV_ARRAY[DVIndex].Count;
    }
    else
    {
      //If the sub type is an aggregate type, then it can contain arrays, so we have to recur
      for (i = 0; i < DV_ARRAY[DVIndex].Count; i++)
      {
        Size += cCmdCalcFlattenedSize(DSElementID, ARRAY_ELEM_OFFSET(DVIndex, i));
      }
    }
  }
  else if (TypeCode == TC_CLUSTER)
  {
    Count = cCmdClusterCount(DSElementID);

    DSElementID = INC_ID(DSElementID);
    for (i = 0; i < Count; i++)
    {
      Size += cCmdCalcFlattenedSize(DSElementID, Offset);
      DSElementID = cCmdNextDSElement(DSElementID);
    }
  }
  else //Scalar
  {
    Size += cCmdSizeOf(TypeCode);
  }
  return Size;
}


//!!! Recursive function
NXT_STATUS cCmdFlattenToByteArray(UBYTE * pByteArray, UWORD * pByteOffset, DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode;
  DV_INDEX DVIndex;
  UWORD i;
  UWORD Count;
  UBYTE *pVal;

  TypeCode = cCmdDSType(DSElementID);

  if (TypeCode == TC_ARRAY)
  {
    DVIndex = cCmdGetDVIndex(DSElementID, Offset);
    Count = DV_ARRAY[DVIndex].Count;

    DSElementID = INC_ID(DSElementID);
    TypeCode = cCmdDSType(DSElementID);
    if (!IS_AGGREGATE_TYPE(TypeCode))
    {
      //Short circuit recursive calculation if our array sub-type is a scalar
      Count = DV_ARRAY[DVIndex].ElemSize * DV_ARRAY[DVIndex].Count;
      memmove((pByteArray + *pByteOffset), (VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset), Count);
      *pByteOffset += Count;
    }
    else
    {
      //If the sub type is an aggregate type, then it can contain arrays, so we have to recur
      for (i = 0; i < Count; i++)
      {
        cCmdFlattenToByteArray(pByteArray, pByteOffset, DSElementID, ARRAY_ELEM_OFFSET(DVIndex, i));
      }
    }
  }
  else if (TypeCode == TC_CLUSTER)
  {
    Count = cCmdClusterCount(DSElementID);

    DSElementID = INC_ID(DSElementID);
    for (i = 0; i < Count; i++)
    {
      cCmdFlattenToByteArray(pByteArray, pByteOffset, DSElementID, Offset);
      DSElementID = cCmdNextDSElement(DSElementID);
    }
  }
  else //Scalar
  {
    pVal = cCmdResolveDataArg(DSElementID, Offset, NULL);
    Count = cCmdSizeOf(TypeCode);

    memmove((pByteArray + *pByteOffset), pVal, Count);
    *pByteOffset += Count;
  }

  return Status;
}

NXT_STATUS cCmdUnflattenFromByteArray(UBYTE * pByteArray, UWORD * pByteOffset, DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode;
  DV_INDEX DVIndex;
  UWORD i;
  UWORD Count;
  UBYTE *pVal;

  TypeCode = cCmdDSType(DSElementID);

  if (TypeCode == TC_ARRAY)
  {
    DVIndex = cCmdGetDVIndex(DSElementID, Offset);
    Count = DV_ARRAY[DVIndex].Count;

    DSElementID = INC_ID(DSElementID);
    TypeCode = cCmdDSType(DSElementID);
    if (!IS_AGGREGATE_TYPE(TypeCode))
    {
      //Short circuit recursive calculation if our array sub-type is a scalar
      Count = DV_ARRAY[DVIndex].ElemSize * DV_ARRAY[DVIndex].Count;
      memmove((VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset), (pByteArray + *pByteOffset), Count);
      *pByteOffset += Count;
    }
    else
    {
      //If the sub type is an aggregate type, then it can contain arrays, so we have to recur
      for (i = 0; i < Count; i++)
      {
        cCmdUnflattenFromByteArray(pByteArray, pByteOffset, DSElementID, ARRAY_ELEM_OFFSET(DVIndex, i));
      }
    }
  }
  else if (TypeCode == TC_CLUSTER)
  {
    Count = cCmdClusterCount(DSElementID);

    DSElementID = INC_ID(DSElementID);
    for (i = 0; i < Count; i++)
    {
      cCmdUnflattenFromByteArray(pByteArray, pByteOffset, DSElementID, Offset);
      DSElementID = cCmdNextDSElement(DSElementID);
    }
  }
  else //Scalar
  {
    pVal = cCmdResolveDataArg(DSElementID, Offset, NULL);
    Count = cCmdSizeOf(TypeCode);

    memmove(pVal, (pByteArray + *pByteOffset), Count);
    *pByteOffset += Count;
  }

  return Status;
}


UWORD cCmdClusterCount(DS_ELEMENT_ID DSElementID)
{
  UWORD ClusterCount;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));
  NXT_ASSERT(cCmdDSType(DSElementID) == TC_CLUSTER);

  ClusterCount = VarsCmd.pDataspaceTOC[DSElementID].DSOffset;

  return ClusterCount;
}


UWORD cCmdGetDVIndex(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  UWORD DVIndex;

  NXT_ASSERT(cCmdDSType(DSElementID) == TC_ARRAY);

  DVIndex = *(UWORD *)(VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[DSElementID].DSOffset + Offset);

  //Make sure we're returning a valid DVIndex
  NXT_ASSERT(DVIndex != 0 && DVIndex < DV_ARRAY[0].Count);

  return DVIndex;
}


UWORD cCmdArrayCount(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  DV_INDEX DVIndex;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));
  NXT_ASSERT(cCmdDSType(DSElementID) == TC_ARRAY);

  DVIndex = cCmdGetDVIndex(DSElementID, Offset);
  return DV_ARRAY[DVIndex].Count;
}

TYPE_CODE cCmdArrayType(DS_ELEMENT_ID DSElementID)
{
  TYPE_CODE TypeCode;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));
  NXT_ASSERT(cCmdIsDSElementIDSane(INC_ID(DSElementID)));
  NXT_ASSERT(cCmdDSType(DSElementID) == TC_ARRAY);

  TypeCode = VarsCmd.pDataspaceTOC[DSElementID + 1].TypeCode;

  return TypeCode;
}


DS_ELEMENT_ID cCmdGetDataspaceCount(void)
{
  return (VarsCmd.DataspaceCount);
}


CODE_INDEX cCmdGetCodespaceCount(CLUMP_ID Clump)
{
  if (Clump == NOT_A_CLUMP)
    return (VarsCmd.CodespaceCount);
  else
  {
    NXT_ASSERT(cCmdIsClumpIDSane(Clump));
    return (VarsCmd.pAllClumps[Clump].CodeEnd - VarsCmd.pAllClumps[Clump].CodeStart + 1);
  }
}


UBYTE cCmdCompare(UBYTE CompCode, ULONG Val1, ULONG Val2, TYPE_CODE TypeCode1, TYPE_CODE TypeCode2)
{
  SLONG SVal1, SVal2;

  if (IS_SIGNED_TYPE(TypeCode1) && IS_SIGNED_TYPE(TypeCode2))
  {
    SVal1 = (SLONG)Val1;
    SVal2 = (SLONG)Val2;
    return ((CompCode == OPCC1_LT   && SVal1 <  SVal2)
         || (CompCode == OPCC1_GT   && SVal1 >  SVal2)
         || (CompCode == OPCC1_LTEQ && SVal1 <= SVal2)
         || (CompCode == OPCC1_GTEQ && SVal1 >= SVal2)
         || (CompCode == OPCC1_EQ   && SVal1 == SVal2)
         || (CompCode == OPCC1_NEQ  && SVal1 != SVal2));
  }
  else
  {
    return ((CompCode == OPCC1_LT   && Val1 <  Val2)
         || (CompCode == OPCC1_GT   && Val1 >  Val2)
         || (CompCode == OPCC1_LTEQ && Val1 <= Val2)
         || (CompCode == OPCC1_GTEQ && Val1 >= Val2)
         || (CompCode == OPCC1_EQ   && Val1 == Val2)
         || (CompCode == OPCC1_NEQ  && Val1 != Val2));
  }

}


NXT_STATUS cCmdCompareAggregates(UBYTE CompCode, UBYTE *ReturnBool, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE Finished;

  Finished = FALSE;
  Status = cCmdRecursiveCompareAggregates(CompCode, ReturnBool, &Finished, Arg2, Offset2, Arg3, Offset3);
  if (Finished == FALSE)
  {
    //If Finished has not been set to TRUE, it means that it was unable to find an inequality, thereby ending the comparison.
    //Both elements are equal. Assign the proper value to ReturnBool
    *ReturnBool = (CompCode == OPCC1_EQ || CompCode == OPCC1_GTEQ || CompCode == OPCC1_LTEQ);
  }

  return Status;
}


//!!! Recursive function
NXT_STATUS cCmdRecursiveCompareAggregates(UBYTE CompCode, UBYTE *ReturnBool, UBYTE *Finished, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3)
{
  //The value of Finished must be set to FALSE before calling this function.
  //We are able to determine the result of the comparison once we find an inequality.
  //Once an inequality is found, Finished is set to TRUE and ReturnBool is set based on the CompCode.
  //A call to this function will return with Finished still equal to FALSE if both elements are equal in value and count.
  //It is the caller of this function's job to set ReturnBool if this function returns with Finished == FALSE.

  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode2, TypeCode3;
  DV_INDEX DVIndex2, DVIndex3;
  ULONG ArgVal2, ArgVal3;
  UWORD Count2, Count3, MinCount;
  UWORD i;

  void *pArg2 = NULL,
       *pArg3 = NULL;

  TypeCode2 = cCmdDSType(Arg2);
  TypeCode3 = cCmdDSType(Arg3);

  //Make sure the two things we're comparing are the same type
  if (IS_AGGREGATE_TYPE(TypeCode2) && (TypeCode2 != TypeCode3))
  {
    NXT_BREAK;
    return ERR_ARG;
  }

  //Simple case, both args are scalars. Solve and return.
  if (!IS_AGGREGATE_TYPE(TypeCode2))
  {
    pArg2 = cCmdResolveDataArg(Arg2, Offset2, &TypeCode2);
    pArg3 = cCmdResolveDataArg(Arg3, Offset3, &TypeCode3);

    ArgVal2 = cCmdGetVal(pArg2, TypeCode2);
    ArgVal3 = cCmdGetVal(pArg3, TypeCode3);

    //Once we find an inequality, we can determine the result of the comparison
    *Finished = cCmdCompare(OPCC1_NEQ, ArgVal2, ArgVal3, TypeCode2, TypeCode3);

    if (*Finished)
      *ReturnBool = cCmdCompare(CompCode, ArgVal2, ArgVal3, TypeCode2, TypeCode3);

    return Status;
  }

  // Initialize local variables for each argument

  if (TypeCode2 == TC_ARRAY)
  {
    Count2 = cCmdArrayCount(Arg2, Offset2);
    DVIndex2 = cCmdGetDVIndex(Arg2, Offset2);
    Offset2 = DV_ARRAY[DVIndex2].Offset;

    Count3 = cCmdArrayCount(Arg3, Offset3);
    DVIndex3 = cCmdGetDVIndex(Arg3, Offset3);
    Offset3 = DV_ARRAY[DVIndex3].Offset;
  }
  else if (TypeCode2 == TC_CLUSTER)
  {
    Count2 = cCmdClusterCount(Arg2);
    Count3 = cCmdClusterCount(Arg3);
  }

  //Short circuit evaluation of EQ and NEQ if counts are different
  if (Count2 != Count3)
  {
    if ((CompCode == OPCC1_EQ) || (CompCode == OPCC1_NEQ))
    {
      *Finished = TRUE;
      *ReturnBool = (CompCode == OPCC1_NEQ);
      return Status;
    }
  }

  MinCount = (Count2 < Count3) ? Count2 : Count3;

  //Advance aggregate args to first sub-element for next call
  Arg2 = INC_ID(Arg2);
  Arg3 = INC_ID(Arg3);

  //
  // Loop through the sub-elements of aggregate arguments.
  // Call cCmdRecursiveCompareAggregates recursively with simpler type.
  //

  for (i = 0; i < MinCount; i++)
  {
    Status = cCmdRecursiveCompareAggregates(CompCode, ReturnBool, Finished, Arg2, Offset2, Arg3, Offset3);
    if (*Finished || IS_ERR(Status))
      return Status;

    //Advance aggregate args to next sub-element
    if (TypeCode2 == TC_ARRAY)
    {
      Offset2 += DV_ARRAY[DVIndex2].ElemSize;
      Offset3 += DV_ARRAY[DVIndex3].ElemSize;
    }
    else if (TypeCode2 == TC_CLUSTER)
    {
      Arg2 = cCmdNextDSElement(Arg2);
      Arg3 = cCmdNextDSElement(Arg3);
    }
  }

  //All elements in aggregates type up to MinCount are equal. Count discrepancy determines comparison outcome.
  if (Count2 != Count3)
  {
    *Finished = TRUE;
    *ReturnBool = cCmdCompare(CompCode, Count2, Count3, TC_UWORD, TC_UWORD);
  }
  //Else, no size discrepancy. Elements are equal. Comparison still not resolved,
  //so return !Finished and status back up the call chain for further comparison

  return Status;
}


//
// Interpreter Functions
//
#define VAR_INSTR_SIZE 0xE

NXT_STATUS cCmdInterpFromClump(CLUMP_ID Clump)
{
  NXT_STATUS Status = NO_ERR;
  CLUMP_REC * pClumpRec;
  CODE_WORD * pInstr;
  UBYTE InterpFuncIndex, InstrSize;
#if VM_BENCHMARK
  ULONG InstrTime = dTimerRead();
#endif

  if (!cCmdIsClumpIDSane(Clump))
  {
    //Caller gave us a bad clump ID -- something is very wrong!  Force interpretter to halt.
    NXT_BREAK;
    return (ERR_ARG);
  }

  //Resolve clump record structure and current instruction pointer
  pClumpRec = &(VarsCmd.pAllClumps[Clump]);
  pInstr = (VarsCmd.pCodespace + pClumpRec->CodeStart + pClumpRec->PC);

  //Get instruction size in bytes.
  InstrSize = INSTR_SIZE(pInstr);

  //If instruction is odd-sized or if "real" op code is out of range, give up and return ERR_INSTR
  if ((InstrSize & 0x01) || OP_CODE(pInstr) >= OPCODE_COUNT)
    return (ERR_INSTR);

  InterpFuncIndex = (InstrSize / 2) - 1;

#ifdef USE_SHORT_OPS
  //If instruction has shortened encoding, add 1 for true interpretter
  if (IS_SHORT_OP(pInstr))
  {
    InterpFuncIndex++;
  }
#endif

  //Peg InterpFuncIndex to 'Other'.  Illegal instructions will be caught in cCmdInterpOther().
  if (InterpFuncIndex > 4)
    InterpFuncIndex = 4;

  //If instruction is variably-sized; true size is held in the first argument
  //!!! This InstrSize wrangling MUST occur after computing InterpFuncIndex
  //because variable sized instructions may confuse the code otherwise
  if (InstrSize == VAR_INSTR_SIZE)
    InstrSize = (UBYTE)(pInstr[1]);

  //Set ScratchPC to clump's current PC so sub-interpreters can apply relative offsets
  VarsCmd.ScratchPC = pClumpRec->PC;

  //Set CallerClump to Clump, for use by instructions such as OP_ACQUIRE
  VarsCmd.CallerClump = Clump;

  Status = (*InterpFuncs[InterpFuncIndex])(pInstr);

  if (Status == ERR_MEM)
  {
    //Memory is full. Compact dataspace and try the instruction again.
    //!!! Could compact DopeVectorArray here
    cCmdDSCompact();
    Status = (*InterpFuncs[InterpFuncIndex])(pInstr);
  }

  if (!IS_ERR(Status))
  {
    //If clump is finished, reset PC and firecount
    if (Status == CLUMP_DONE)
    {
      pClumpRec->PC = 0;
      pClumpRec->CurrFireCount = pClumpRec->InitFireCount;
    }
    //Else, if instruction has provided override program counter, use it
    else if (Status == PC_OVERRIDE)
    {
      pClumpRec->PC = VarsCmd.ScratchPC;
    }
    //Else, auto-advance from encoded instruction size (PC is word-based)
    else
    {
      pClumpRec->PC += InstrSize / 2;
    }

    //Throw error if we ever advance beyond the clump's codespace
    if (pClumpRec->PC > cCmdGetCodespaceCount(Clump))
    {
      NXT_BREAK;
      Status = ERR_INSTR;
    }
  }

#if VM_BENCHMARK
  //Increment opcode count
  VarsCmd.OpcodeBenchmarks[OP_CODE(pInstr)][0]++;

  InstrTime = dTimerRead() - InstrTime;
  if (InstrTime > 1)
  {
    VarsCmd.OpcodeBenchmarks[OP_CODE(pInstr)][1]++;
    if (InstrTime > VarsCmd.OpcodeBenchmarks[OP_CODE(pInstr)][2])
      VarsCmd.OpcodeBenchmarks[OP_CODE(pInstr)][2] = InstrTime;
  }
#endif

  return (Status);
}


NXT_STATUS cCmdInterpUnop1(CODE_WORD * const pCode)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1;
  void *pArg1 = NULL;
  TYPE_CODE TypeCode1;

  NXT_ASSERT(pCode != NULL);

#ifdef USE_SHORT_OPS
  if (IS_SHORT_OP(pCode))
  {
    //add mapping from quick op to real op
    opCode = ShortOpMap[SHORT_OP_CODE(pCode)];
    Arg1 = SHORT_ARG(pCode);
  }
  else
  {
  opCode = OP_CODE(pCode);
  Arg1   = pCode[1];
  }
#else
  opCode = OP_CODE(pCode);
  Arg1   = pCode[1];
#endif //USE_SHORT_OPS


  switch (opCode)
  {
    case OP_JMP:
    {
      VarsCmd.ScratchPC = VarsCmd.ScratchPC + (SWORD)Arg1;
      Status = PC_OVERRIDE;
    }
    break;

    case OP_ACQUIRE:
    {
      NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
      NXT_ASSERT(VarsCmd.pDataspaceTOC[Arg1].TypeCode == TC_MUTEX);

      Status = cCmdAcquireMutex((MUTEX_Q *)cCmdDSPtr(Arg1, 0), VarsCmd.CallerClump);
    }
    break;

    case OP_RELEASE:
    {
      NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
      NXT_ASSERT(VarsCmd.pDataspaceTOC[Arg1].TypeCode == TC_MUTEX);

      Status = cCmdReleaseMutex((MUTEX_Q *)cCmdDSPtr(Arg1, 0), VarsCmd.CallerClump);
    }
    break;

    case OP_SUBRET:
    {
      NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));

      //Take Subroutine off RunQ
      //Add Subroutine's caller to RunQ
      cCmdDeQClump(&(VarsCmd.RunQ), VarsCmd.CallerClump);
      cCmdEnQClump(&(VarsCmd.RunQ), *((CLUMP_ID *)cCmdDSPtr(Arg1, 0)));

      Status = CLUMP_DONE;
    }
    break;

    case OP_FINCLUMPIMMED:
    {
        cCmdDeQClump(&(VarsCmd.RunQ), VarsCmd.CallerClump); //Dequeue finalized clump
        cCmdSchedDependent(VarsCmd.CallerClump, (CLUMP_ID)Arg1);  // Use immediate form

        Status = CLUMP_DONE;
    }
    break;

    case OP_GETTICK:
    {
      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);


      cCmdSetVal(pArg1, TypeCode1, dTimerRead());
    }
    break;

    case OP_STOP:
    {
      //Unwired Arg1 means always stop
      if (Arg1 == NOT_A_DS_ID)
      {
        Status = STOP_REQ;
      }
      else
      {
        pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);

        if (cCmdGetVal(pArg1, TypeCode1) > 0)
          Status = STOP_REQ;
      }
    }
    break;

    default:
    {
      //Fatal error: Unrecognized instruction
      NXT_BREAK;
      Status = ERR_INSTR;
    }
    break;
  }

  return (Status);
}


NXT_STATUS cCmdInterpUnop2(CODE_WORD * const pCode)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1;
  DATA_ARG Arg2;
  void *pArg1 = NULL, *pArg2 = NULL;
  TYPE_CODE TypeCode1, TypeCode2;

  ULONG i;
  UWORD ArgC;
  static UBYTE * ArgV[MAX_CALL_ARGS + 1];

  UWORD Count;
  UWORD Offset;
  SLONG TmpSLong;
  ULONG TmpULong;
  ULONG ArgVal2;

  NXT_ASSERT(pCode != NULL);

#ifdef USE_SHORT_OPS
  if (IS_SHORT_OP(pCode))
  {
    //add mapping from quick op to real op
    opCode = ShortOpMap[SHORT_OP_CODE(pCode)];
    Arg1 = SHORT_ARG(pCode) + pCode[1];
    Arg2 = pCode[1];
  }
  else
  {
  opCode = OP_CODE(pCode);
  Arg1   = pCode[1];
  Arg2   = pCode[2];
  }
#else
  opCode = OP_CODE(pCode);
  Arg1   = pCode[1];
  Arg2   = pCode[2];
#endif //USE_SHORT_OPS

  if (opCode == OP_NEG || opCode == OP_NOT || opCode == OP_TST)
  {
    return cCmdInterpPolyUnop2(*pCode, Arg1, 0, Arg2, 0);
  }

  switch (opCode)
  {
    case OP_MOV:
    {
      //!!! Optimized move for byte arrays (makes File I/O involving CStrs tolerable). Optimize for other cases?
      if ((cCmdDSType(Arg1) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg1)) == TC_UBYTE) &&
          (cCmdDSType(Arg2) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg2)) == TC_UBYTE))
      {
        Count = cCmdArrayCount(Arg2, 0);
        Status = cCmdDSArrayAlloc(Arg1, 0, Count);
        if (IS_ERR(Status))
          return Status;

        pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
        pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);

        memmove(pArg1, pArg2, Count);
      }
      else
      {
        Status = cCmdInterpPolyUnop2(OP_MOV, Arg1, 0, Arg2, 0);
      }
    }
    break;

    case OP_SET:
    {
      //!!! Should throw error if TypeCode1 is non-scalar
      //    Accepting non-scalar destinations could have unpredictable results!
      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
      cCmdSetVal(pArg1, TypeCode1, Arg2);
    }
    break;

    case OP_BRTST:
    {
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
      if (cCmdCompare(COMP_CODE(pCode), (SLONG)cCmdGetVal(pArg2, TypeCode2), 0, TC_SLONG, TC_SLONG))
      {
        VarsCmd.ScratchPC = VarsCmd.ScratchPC + (SWORD)Arg1;
        Status = PC_OVERRIDE;
      }
    }
    break;

    case OP_FINCLUMP:
    {
      cCmdDeQClump(&(VarsCmd.RunQ), VarsCmd.CallerClump); //Dequeue finalized clump
      cCmdSchedDependents(VarsCmd.CallerClump, (SWORD)Arg1, (SWORD)Arg2);

      Status = CLUMP_DONE;
    }
    break;

    case OP_SUBCALL:
    {
      NXT_ASSERT(cCmdIsClumpIDSane((CLUMP_ID)Arg1));
      NXT_ASSERT(!cCmdIsClumpOnQ(&(VarsCmd.RunQ), (CLUMP_ID)Arg1));

      NXT_ASSERT(cCmdIsDSElementIDSane(Arg2));

      *((CLUMP_ID *)(cCmdDSPtr(Arg2, 0))) = VarsCmd.CallerClump;

      cCmdDeQClump(&(VarsCmd.RunQ), VarsCmd.CallerClump); //Take caller off RunQ
      cCmdEnQClump(&(VarsCmd.RunQ), (CLUMP_ID)Arg1);  //Add callee to RunQ

      Status = CLUMP_SUSPEND;
    }
    break;

    case OP_ARRSIZE:
    {
      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
      cCmdSetVal(pArg1, TypeCode1, cCmdArrayCount(Arg2, 0));
    }
    break;

    case OP_SYSCALL:
    {
      if (Arg1 >= SYSCALL_COUNT)
      {
        NXT_BREAK;
        Status = ERR_INSTR;
        break;
      }

      ArgC = cCmdClusterCount(Arg2);

      if (ArgC > MAX_CALL_ARGS)
      {
        NXT_BREAK;
        Status = ERR_INSTR;
        break;
      }

      if (ArgC > 0)
      {
        Arg2 = INC_ID(Arg2);

        for (i = 0; i < ArgC; i++)
        {
          if (cCmdDSType(Arg2) == TC_ARRAY)
          {
            //Storing pointer to array's DV_INDEX
            //!!! This resolve is different than cCmdDSPtr
            // since SysCalls may need the DVIndex to re-alloc arrays
            ArgV[i] = VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[Arg2].DSOffset;
          }
          else
          {
            ArgV[i] = cCmdDSPtr(Arg2, 0);
          }

          //If any argument fails to resolve, return a fatal error.
          if (ArgV[i] == NULL)
          {
            Status = ERR_BAD_PTR;
            break;
          }

          Arg2 = cCmdNextDSElement(Arg2);
        }
      }
      else
      {
        i = 0;
      }

      //ArgV list is null terminated
      ArgV[i] = NULL;

      Status = (*SysCallFuncs[Arg1])(ArgV);
    }
    break;

    case OP_FLATTEN:
    {
      //Flatten Arg2 to a NULL terminated string

      //Assert that the destination is a string (array of bytes)
      NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg1)) == TC_UBYTE);

      Count = cCmdCalcFlattenedSize(Arg2, 0);
      //Add room for NULL terminator
      Count++;
      Status = cCmdDSArrayAlloc(Arg1, 0, Count);
      if (IS_ERR(Status))
        return Status;

      pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
      Offset = 0;

      Status = cCmdFlattenToByteArray(pArg1, &Offset, Arg2, 0);
      //Append NULL terminator
      *((UBYTE *)pArg1 + Offset) = 0;
      Offset++;
      NXT_ASSERT(Offset == Count);
    }
    break;

    case OP_NUMTOSTRING:
    {
      //Assert that the destination is a string (array of bytes)
      NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg1)) == TC_UBYTE);

      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
      //Make sure we're trying to convert a scalar to a string
      NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode2));

      ArgVal2 = cCmdGetVal(pArg2, TypeCode2);

      //Calculate size of array
      if (ArgVal2 == 0)
        Count = 1;
      else
        Count = 0;

      if (TypeCode2 == TC_SLONG || TypeCode2 == TC_SWORD || TypeCode2 == TC_SBYTE)
      {
        TmpSLong = (SLONG)ArgVal2;
        //Add room for negative sign
        if (TmpSLong < 0)
          Count++;

        while (TmpSLong)
        {
          TmpSLong /= 10;
          Count++;
        }
      }
      else
      {
        TmpULong = ArgVal2;
        while (TmpULong)
        {
          TmpULong /= 10;
          Count++;
        }
      }

      //add room for NULL terminator
      Count++;

      //Allocate array
      Status = cCmdDSArrayAlloc(Arg1, 0, Count);
      if (IS_ERR(Status))
        return Status;

      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);

      //Populate array
      if (TypeCode2 == TC_SLONG || TypeCode2 == TC_SWORD || TypeCode2 == TC_SBYTE)
      {
        sprintf(pArg1, "%d", (SLONG)ArgVal2);
      }
      else
      {
        sprintf(pArg1, "%u", ArgVal2);
      }
    }
    break;

    case OP_STRTOBYTEARR:
    {
      NXT_ASSERT((cCmdDSType(Arg1) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg1)) == TC_UBYTE));
      NXT_ASSERT((cCmdDSType(Arg2) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg2)) == TC_UBYTE));

      Count = cCmdArrayCount(Arg2, 0);

      if (Count > 0)
      {
        Status = cCmdDSArrayAlloc(Arg1, 0, (UWORD)(Count - 1));
        if (IS_ERR(Status))
          return Status;

        pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
        pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);

        memmove(pArg1, pArg2, Count - 1);
      }
    }
    break;

    case OP_BYTEARRTOSTR:
    {
      NXT_ASSERT((cCmdDSType(Arg1) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg1)) == TC_UBYTE));
      NXT_ASSERT((cCmdDSType(Arg2) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg2)) == TC_UBYTE));

      Count = cCmdArrayCount(Arg2, 0);

      Status = cCmdDSArrayAlloc(Arg1, 0, (UWORD)(Count + 1));
      if (IS_ERR(Status))
        return Status;

      pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
      pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);

      memmove(pArg1, pArg2, Count);
      *((UBYTE *)pArg1 + Count) = '\0';
    }
    break;

    default:
    {
      //Fatal error: Unrecognized instruction
      NXT_BREAK;
      Status = ERR_INSTR;
    }
    break;
  }

  return (Status);
}


NXT_STATUS cCmdInterpPolyUnop2(CODE_WORD const Code, DATA_ARG Arg1, UWORD Offset1, DATA_ARG Arg2, UWORD Offset2)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode1, TypeCode2;
  DV_INDEX DVIndex1, DVIndex2;
  ULONG ArgVal1, ArgVal2;
  UWORD Count1, Count2;
  UWORD MinArrayCount;
  UWORD i;
  //!!! AdvCluster is intended to catch the case where sources are Cluster and an Array of Clusters.
  // In practice, the logic it uses is broken, leading to some source cluster elements being ignored.
  UBYTE AdvCluster;

  void * pArg1 = NULL,
        *pArg2 = NULL;

  TypeCode1 = cCmdDSType(Arg1);
  TypeCode2 = cCmdDSType(Arg2);

  //Simple case, scalar. Solve and return.
  if (!IS_AGGREGATE_TYPE(TypeCode2))
  {
    NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode1));

    pArg1 = cCmdResolveDataArg(Arg1, Offset1, &TypeCode1);
    pArg2 = cCmdResolveDataArg(Arg2, Offset2, &TypeCode2);

    ArgVal2 = cCmdGetVal(pArg2, TypeCode2);
    ArgVal1 = cCmdUnop2(Code, ArgVal2, TypeCode2);
    cCmdSetVal(pArg1, TypeCode1, ArgVal1);
    return Status;
  }

  //At least one of the args is an aggregate type

  //
  // Initialize Count and ArrayType local variables for each argument
  //

  if (TypeCode2 == TC_ARRAY)
  {
    Count2 = cCmdArrayCount(Arg2, Offset2);
    DVIndex2 = cCmdGetDVIndex(Arg2, Offset2);
    Offset2 = DV_ARRAY[DVIndex2].Offset;
  }
  else if (TypeCode2 == TC_CLUSTER)
  {
    Count2 = cCmdClusterCount(Arg2);
  }

  if (TypeCode1 == TC_ARRAY)
  {
    if (TypeCode2 != TC_ARRAY)
    {
      //If output is an array, but source is not an array, that's a fatal error!
      NXT_BREAK;
      return (ERR_ARG);
    }

    MinArrayCount = Count2;

    //Make sure the destination array is the proper size to hold the result
    Status = cCmdDSArrayAlloc(Arg1, Offset1, MinArrayCount);
    if (IS_ERR(Status))
      return Status;

    Count1 = MinArrayCount;
    DVIndex1 = cCmdGetDVIndex(Arg1, Offset1);
    Offset1 = DV_ARRAY[DVIndex1].Offset;
    AdvCluster = FALSE;
  }
  else if (TypeCode1 == TC_CLUSTER)
  {
    Count1 = cCmdClusterCount(Arg1);
    AdvCluster = TRUE;
  }

  //Advance aggregate args to first sub-element for next call
  if (IS_AGGREGATE_TYPE(TypeCode1))
    Arg1 = INC_ID(Arg1);
  if (IS_AGGREGATE_TYPE(TypeCode2))
    Arg2 = INC_ID(Arg2);

  //
  // Loop through the sub-elements of aggregate arguments.
  // Call cCmdInterpPolyUnop2 recursively with simpler type.
  //

  for (i = 0; i < Count1; i++)
  {
    Status = cCmdInterpPolyUnop2(Code, Arg1, Offset1, Arg2, Offset2);
    if (IS_ERR(Status))
      return Status;

    //Advance aggregate args to next sub-element
    if (TypeCode1 == TC_ARRAY)
      Offset1 += DV_ARRAY[DVIndex1].ElemSize;
    else if ((TypeCode1 == TC_CLUSTER) && AdvCluster)
      Arg1 = cCmdNextDSElement(Arg1);

    if (TypeCode2 == TC_ARRAY)
      Offset2 += DV_ARRAY[DVIndex2].ElemSize;
    else if ((TypeCode2 == TC_CLUSTER) && AdvCluster)
      Arg2 = cCmdNextDSElement(Arg2);
  }

  return Status;
}


ULONG cCmdUnop2(CODE_WORD const Code, ULONG Operand, TYPE_CODE TypeCode)
{
  UBYTE opCode;

  opCode = OP_CODE((&Code));

  switch (opCode)
  {
    case OP_MOV:
    {
      return Operand;
    }

    case OP_NEG:
    {
      return (-((SLONG)Operand));
    }

    case OP_NOT:
    {
      //!!! OP_NOT is logical, *not* bit-wise.
      //This differs from the other logical ops because we don't distinguish booleans from UBYTEs.
      return (!Operand);
    }

    case OP_TST:
    {
      return cCmdCompare(COMP_CODE((&Code)), Operand, 0, TypeCode, TypeCode);
    }

    default:
    {
      //Unrecognized instruction, NXT_BREAK for easy debugging (ERR_INSTR handled in caller)
      NXT_BREAK;
      return 0;
    }
  }
}


NXT_STATUS cCmdInterpBinop(CODE_WORD * const pCode)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1, Arg2, Arg3;
  TYPE_CODE TypeCode1, TypeCode2, TypeCode3;
  ULONG ArgVal2, ArgVal3;
  UBYTE CmpBool;
  DV_INDEX DVIndex1, DVIndex2;
  UWORD i;

  void * pArg1 = NULL,
        *pArg2 = NULL,
        *pArg3 = NULL;

  NXT_ASSERT(pCode != NULL);
#ifdef USE_SHORT_OPS
  if (IS_SHORT_OP(pCode))
  {
    //add mapping from quick op to real op
    opCode = ShortOpMap[SHORT_OP_CODE(pCode)];
    Arg1 = SHORT_ARG(pCode) + pCode[1];
    Arg2 = pCode[1];
    Arg3 = pCode[2];
  }
  else
  {
    opCode = OP_CODE(pCode);
    Arg1 = pCode[1];
    Arg2 = pCode[2];
    Arg3 = pCode[3];
  }
#else
  opCode = OP_CODE(pCode);
  Arg1 = pCode[1];
  Arg2 = pCode[2];
  Arg3 = pCode[3];
#endif //USE_SHORT_OPS

  if (opCode == OP_ADD || opCode == OP_SUB || opCode == OP_MUL || opCode == OP_DIV || opCode == OP_MOD ||
      opCode == OP_AND || opCode == OP_OR || opCode == OP_XOR)
  {
    return cCmdInterpPolyBinop(opCode, Arg1, 0, Arg2, 0, Arg3, 0);
  }

  //Resolve data arguments, except for opcodes which the arguments are not DataArgs
  if (opCode != OP_BRCMP)
  {
    pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
  }

  if (opCode != OP_INDEX)
  {
    pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
    ArgVal2 = cCmdGetVal(pArg2, TypeCode2);
  }

  if ((opCode != OP_GETOUT) && (opCode != OP_SETIN)  && (opCode != OP_GETIN) && (opCode != OP_INDEX) && (opCode != OP_ARRINIT))
  {
    pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);
    ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
  }

  switch (opCode)
  {

    case OP_CMP:
    {
      if (!IS_AGGREGATE_TYPE(cCmdDSType(Arg1)) && IS_AGGREGATE_TYPE(cCmdDSType(Arg2)) && IS_AGGREGATE_TYPE(cCmdDSType(Arg3)))
      {
        //Compare Aggregates
        Status = cCmdCompareAggregates(COMP_CODE(pCode), &CmpBool, Arg2, 0, Arg3, 0);
        cCmdSetVal(pArg1, TypeCode1, CmpBool);
      }
      else
      {
        //Compare Elements
        Status = cCmdInterpPolyBinop(*pCode, Arg1, 0, Arg2, 0, Arg3, 0);
      }
    }
    break;

    case OP_BRCMP:
    {
      //Compare Aggregates
      Status = cCmdCompareAggregates(COMP_CODE(pCode), &CmpBool, Arg2, 0, Arg3, 0);

      if (CmpBool)
      {
        VarsCmd.ScratchPC = VarsCmd.ScratchPC + (SWORD)Arg1;
        Status = PC_OVERRIDE;
      }
    }
    break;

    case OP_GETOUT:
    {
      Arg2 = (UWORD)(0xC200 | (Arg3 + ArgVal2 * IO_OUT_FPP));
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);

      cCmdSetVal(pArg1, TypeCode1, cCmdGetVal(pArg2, TypeCode2));
    }
    break;

    //!!! All IO map access commands should screen illegal port values!
    //    Right now, cCmdResolveDataArg's implementation allows SETIN/GETIN to access arbitrary RAM!
    case OP_SETIN:
    {
      Arg2 = (UWORD)(0xC000 | (Arg3 + ArgVal2 * IO_IN_FPP));
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);

      cCmdSetVal(pArg2, TypeCode2, cCmdGetVal(pArg1, TypeCode1));
    }
    break;

    case OP_GETIN:
    {
      Arg2 = (UWORD)(0xC000 | (Arg3 + ArgVal2 * IO_IN_FPP));
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);

      cCmdSetVal(pArg1, TypeCode1, cCmdGetVal(pArg2, TypeCode2));
    }
    break;

    case OP_INDEX:
    {
      if (Arg3 != NOT_A_DS_ID)
      {
        pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);
        ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
      }
      else //Index input unwired
      {
        ArgVal3 = 0;
      }

      DVIndex2 = cCmdGetDVIndex(Arg2, 0);
      if (ArgVal3 >= DV_ARRAY[DVIndex2].Count)
        return (ERR_ARG);

      Status = cCmdInterpPolyUnop2(OP_MOV, Arg1, 0, INC_ID(Arg2), ARRAY_ELEM_OFFSET(DVIndex2, ArgVal3));
    }
    break;

    case OP_ARRINIT:
    {
      //Arg1 - Dst, Arg2 - element type/default val, Arg3 - length

      NXT_ASSERT(TypeCode1 == TC_ARRAY);

      if (Arg3 != NOT_A_DS_ID)
      {
        pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);
        ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
      }
      else //Length input unwired
      {
        ArgVal3 = 0;
      }

      Status = cCmdDSArrayAlloc(Arg1, 0, (UWORD)ArgVal3);
      if (IS_ERR(Status))
        return Status;

      DVIndex1 = cCmdGetDVIndex(Arg1, 0);
      for (i = 0; i < ArgVal3; i++)
      {
        //copy Arg2 into each element of Arg1
        Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg1), ARRAY_ELEM_OFFSET(DVIndex1, i), Arg2, 0);
      }
    }
    break;

    default:
    {
      //Fatal error: Unrecognized instruction
      NXT_BREAK;
      Status = ERR_INSTR;
    }
    break;
  }

  return (Status);
}


NXT_STATUS cCmdInterpPolyBinop(CODE_WORD const Code, DATA_ARG Arg1, UWORD Offset1, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode1, TypeCode2, TypeCode3;
  DV_INDEX DVIndex1, DVIndex2, DVIndex3;
  ULONG ArgVal1, ArgVal2, ArgVal3;
  UWORD Count1, Count2, Count3;
  UWORD MinArrayCount;
  UWORD i;
  //!!! AdvCluster is intended to catch the case where sources are Cluster and an Array of Clusters.
  // In practice, the logic it uses is broken, leading to some source cluster elements being ignored.
  UBYTE AdvCluster;

  void * pArg1 = NULL,
        *pArg2 = NULL,
        *pArg3 = NULL;

  TypeCode1 = cCmdDSType(Arg1);
  TypeCode2 = cCmdDSType(Arg2);
  TypeCode3 = cCmdDSType(Arg3);

  //Simple case, both args are scalars. Solve and return.
  if ((!IS_AGGREGATE_TYPE(TypeCode2)) && (!IS_AGGREGATE_TYPE(TypeCode3)))
  {
    NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode1));

    pArg1 = cCmdResolveDataArg(Arg1, Offset1, &TypeCode1);
    pArg2 = cCmdResolveDataArg(Arg2, Offset2, &TypeCode2);
    pArg3 = cCmdResolveDataArg(Arg3, Offset3, &TypeCode3);

    ArgVal2 = cCmdGetVal(pArg2, TypeCode2);
    ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
    ArgVal1 = cCmdBinop(Code, ArgVal2, ArgVal3, TypeCode2, TypeCode3);
    cCmdSetVal(pArg1, TypeCode1, ArgVal1);
    return Status;
  }

  //At least one of the args is an aggregate type

  //
  // Initialize Count and ArrayType local variables for each argument
  //

  if (TypeCode2 == TC_ARRAY)
  {
    Count2 = cCmdArrayCount(Arg2, Offset2);
    DVIndex2 = cCmdGetDVIndex(Arg2, Offset2);
    Offset2 = DV_ARRAY[DVIndex2].Offset;
  }
  else if (TypeCode2 == TC_CLUSTER)
  {
    Count2 = cCmdClusterCount(Arg2);
  }

  if (TypeCode3 == TC_ARRAY)
  {
    Count3 = cCmdArrayCount(Arg3, Offset3);
    DVIndex3 = cCmdGetDVIndex(Arg3, Offset3);
    Offset3 = DV_ARRAY[DVIndex3].Offset;
  }
  else if (TypeCode3 == TC_CLUSTER)
  {
    Count3 = cCmdClusterCount(Arg3);
  }


  if (TypeCode1 == TC_ARRAY)
  {
    //If the destination is an array, make sure it has enough memory to hold the result
    if ((TypeCode2 == TC_ARRAY) && (TypeCode3 == TC_ARRAY))
    {
      if (Count2 < Count3)
        MinArrayCount = Count2;
      else
        MinArrayCount = Count3;
    }
    else if (TypeCode2 == TC_ARRAY)
      MinArrayCount = Count2;
    else if (TypeCode3 == TC_ARRAY)
      MinArrayCount = Count3;
    else
    {
      //If output is an array, but no sources are arrays, that's a fatal error!
      NXT_BREAK;
      return (ERR_ARG);
    }

    //Make sure the destination array is the proper size to hold the result
    Status = cCmdDSArrayAlloc(Arg1, Offset1, MinArrayCount);
    if (IS_ERR(Status))
      return Status;

    Count1 = MinArrayCount;
    DVIndex1 = cCmdGetDVIndex(Arg1, Offset1);
    Offset1 = DV_ARRAY[DVIndex1].Offset;
    AdvCluster = FALSE;
  }
  else if (TypeCode1 == TC_CLUSTER)
  {
    Count1 = cCmdClusterCount(Arg1);
    AdvCluster = TRUE;
  }

  //Advance aggregate args to first sub-element for next call
  if (IS_AGGREGATE_TYPE(TypeCode1))
    Arg1 = INC_ID(Arg1);
  if (IS_AGGREGATE_TYPE(TypeCode2))
    Arg2 = INC_ID(Arg2);
  if (IS_AGGREGATE_TYPE(TypeCode3))
    Arg3 = INC_ID(Arg3);

  //
  // Loop through the sub-elements of aggregate arguments.
  // Call cCmdInterpPolyBinop recursively with simpler type.
  //

  for (i = 0; i < Count1; i++)
  {
    Status = cCmdInterpPolyBinop(Code, Arg1, Offset1, Arg2, Offset2, Arg3, Offset3);
    if (IS_ERR(Status))
      return Status;

    //Advance aggregate args to next sub-element
    if (TypeCode1 == TC_ARRAY)
      Offset1 += DV_ARRAY[DVIndex1].ElemSize;
    else if ((TypeCode1 == TC_CLUSTER) && AdvCluster)
      Arg1 = cCmdNextDSElement(Arg1);

    if (TypeCode2 == TC_ARRAY)
      Offset2 += DV_ARRAY[DVIndex2].ElemSize;
    else if ((TypeCode2 == TC_CLUSTER) && AdvCluster)
      Arg2 = cCmdNextDSElement(Arg2);

    if (TypeCode3 == TC_ARRAY)
      Offset3 += DV_ARRAY[DVIndex3].ElemSize;
    else if ((TypeCode3 == TC_CLUSTER) && AdvCluster)
      Arg3 = cCmdNextDSElement(Arg3);
  }

  return Status;
}


ULONG cCmdBinop(CODE_WORD const Code, ULONG LeftOp, ULONG RightOp, TYPE_CODE LeftType, TYPE_CODE RightType)
{
  UBYTE opCode;

  opCode = OP_CODE((&Code));

  switch (opCode)
  {
    case OP_ADD:
    {
      return LeftOp + RightOp;
    }

    case OP_SUB:
    {
      return LeftOp - RightOp;
    }

    case OP_MUL:
    {
      return LeftOp * RightOp;
    }

    case OP_DIV:
    {
      //Catch divide-by-zero for a portable, well-defined result.
      //(x / 0) = 0. Thus Spake LOTHAR!! (It's technical.)
      if (RightOp == 0)
        return 0;

      if (IS_SIGNED_TYPE(LeftType) && IS_SIGNED_TYPE(RightType))
        return ((SLONG)LeftOp) / ((SLONG)RightOp);
      else if (IS_SIGNED_TYPE(LeftType))
        return ((SLONG)LeftOp) / RightOp;
      else if (IS_SIGNED_TYPE(RightType))
        return LeftOp / ((SLONG)RightOp);
      else
        return LeftOp / RightOp;
    }

    case OP_MOD:
    {
      //As with OP_DIV, make sure (x % 0) = x is well-defined
      if (RightOp == 0)
        return (LeftOp);

      if (IS_SIGNED_TYPE(LeftType) && IS_SIGNED_TYPE(RightType))
        return ((SLONG)LeftOp) % ((SLONG)RightOp);
      else if (IS_SIGNED_TYPE(LeftType))
        return ((SLONG)LeftOp) % RightOp;
      else if (IS_SIGNED_TYPE(RightType))
        return LeftOp % ((SLONG)RightOp);
      else
        return LeftOp % RightOp;
    }

    case OP_AND:
    {
      return (LeftOp & RightOp);
    }

    case OP_OR:
    {
      return (LeftOp | RightOp);
    }

    case OP_XOR:
    {
      return ((LeftOp | RightOp) & (~(LeftOp & RightOp)));
    }

    case OP_CMP:
    {
      return cCmdCompare(COMP_CODE((&Code)), LeftOp, RightOp, LeftType, RightType);
    }

    default:
    {
      //Unrecognized instruction, NXT_BREAK for easy debugging (ERR_INSTR handled in caller)
      NXT_BREAK;
      return 0;
    }
  }
}

NXT_STATUS cCmdInterpNoArg(CODE_WORD * const pCode)
{
  //Fatal error: Unrecognized instruction (no current opcodes have zero instructions)
  NXT_BREAK;
  return (ERR_INSTR);
}


//OP_SETOUT gets it's own interpreter function because it is relatively complex
// (called from cCmdInterpOther())
//This also serves as a convenient breakpoint stop for investigating output module behavior
NXT_STATUS cCmdExecuteSetOut(CODE_WORD * const pCode)
{
  TYPE_CODE TypeCodeField, TypeCodeSrc, TypeCodePortArg;
  void *pField = NULL,
       *pSrc   = NULL,
       *pPort  = NULL;
  DS_ELEMENT_ID PortArg;
  UWORD PortCount, InstrSize;
  ULONG Port, FieldTableIndex, i, j;
  DV_INDEX DVIndex;

  //Arg1 = InstrSize
  //Arg2 = port number or list of ports
  //Arg3 and beyond = FieldID, src DSID tuples

  //Calculate number of tuples
  //!!! Might want to throw ERR_INSTR if instrSize and tuples don't check out
  InstrSize = (pCode[1] / 2);

  //Second argument may specify a single port or an array list.
  //Figure out which and resolve accordingly.
  PortArg = pCode[2];
  TypeCodePortArg = cCmdDSType(PortArg);

  if (TypeCodePortArg == TC_ARRAY)
  {
    DVIndex = cCmdGetDVIndex(PortArg, 0);
    PortCount = cCmdArrayCount(PortArg, 0);
  }
  else
    PortCount = 1;

  //For each port, process all the tuples
  for (i = 0; i < PortCount; i++)
  {
    if (TypeCodePortArg == TC_ARRAY)
    {
      pPort = (UBYTE*)cCmdResolveDataArg(INC_ID(PortArg), ARRAY_ELEM_OFFSET(DVIndex, i), NULL);
      Port = cCmdGetVal(pPort, cCmdDSType(INC_ID(PortArg)));
    }
    else
    {
      pPort = (UBYTE*)cCmdResolveDataArg(PortArg, 0, NULL);
      Port = cCmdGetVal(pPort, TypeCodePortArg);
    }

    //If user specified a valid port, process the tuples.  Else, this port is a no-op
    if (Port < NO_OF_OUTPUTS)
    {
      for (j = 3; j < InstrSize; j += 2)
      {
        FieldTableIndex = (Port * IO_OUT_FPP) + pCode[j];
        pSrc = cCmdResolveDataArg(pCode[j + 1], 0, &TypeCodeSrc);

        //If FieldTableIndex is valid, go ahead and set the value
        if (FieldTableIndex < IO_OUT_FIELD_COUNT)
        {
          pField = IO_PTRS[MOD_OUTPUT][FieldTableIndex];
          TypeCodeField = IO_TYPES[MOD_OUTPUT][FieldTableIndex];
          cCmdSetVal(pField, TypeCodeField, cCmdGetVal(pSrc, TypeCodeSrc));
        }
        //Else, compiler is nutso! Return fatal error.
        else
          return (ERR_INSTR);
      }
    }
  }

  return (NO_ERR);
}


NXT_STATUS cCmdInterpOther(CODE_WORD * const pCode)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1, Arg2, Arg3, Arg4, Arg5;
  TYPE_CODE TypeCode1, TypeCode2, TypeCode3, TypeCode4, TypeCode5;
  ULONG ArgVal1, ArgVal2, ArgVal3, ArgVal4, ArgVal5;
  UWORD ArrayCount1, ArrayCount2, ArrayCount3, ArrayCount4;
  UWORD MinCount;
  UWORD i,j;
  DV_INDEX DVIndex1, DVIndex2, DVIndex4,TmpDVIndex;
  UWORD SrcCount;
  DS_ELEMENT_ID TmpDSID;
  UWORD DstIndex;
  UWORD Size;
  UWORD Offset;

  void  *pArg1 = NULL;
  void  *pArg2 = NULL;
  void  *pArg3 = NULL;
  void  *pArg4 = NULL;
  void  *pArg5 = NULL;

  NXT_ASSERT(pCode != NULL);

  opCode = OP_CODE(pCode);

  switch (opCode)
  {

    case OP_REPLACE:
    {
      //Arg1 - Dst
      //Arg2 - Src
      //Arg3 - Index
      //Arg4 - New val / array of vals

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];

      NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);

      //Copy Src to Dst
      //!!! Could avoid full data copy if we knew which portion to overwrite
      if (Arg1 != Arg2)
      {
        Status = cCmdInterpPolyUnop2(OP_MOV, Arg1, 0, Arg2, 0);
        if (IS_ERR(Status))
          return Status;
      }

      DVIndex1 = cCmdGetDVIndex(Arg1, 0);
      //Copy new val to Dst
      if (Arg3 != NOT_A_DS_ID)
      {
        pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);
        ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
      }
      else
      {
        //Index input unwired
        ArgVal3 = 0;
      }

      ArrayCount1 = cCmdArrayCount(Arg1, 0);
      //Bounds check
      //If array index (ArgVal3) is out of range, just pass out the copy of Src (effectively no-op)
      if (ArgVal3 >= ArrayCount1)
        return (NO_ERR);

      if (cCmdDSType(Arg4) != TC_ARRAY)
      {
        Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg1), ARRAY_ELEM_OFFSET(DVIndex1, ArgVal3), Arg4, 0);
        if (IS_ERR(Status))
          return Status;
      }
      else
      {
        DVIndex4 = cCmdGetDVIndex(Arg4, 0);

        ArrayCount4 = cCmdArrayCount(Arg4, 0);
        if (ArrayCount1 - ArgVal3 < ArrayCount4)
          MinCount = (UWORD)(ArrayCount1 - ArgVal3);
        else
          MinCount = ArrayCount4;

        for (i = 0; i < MinCount; i++)
        {
          Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg1), ARRAY_ELEM_OFFSET(DVIndex1, ArgVal3 + i), INC_ID(Arg4), ARRAY_ELEM_OFFSET(DVIndex4, i));
          if (IS_ERR(Status))
            return Status;
        }
      }
    }
    break;

    case OP_ARRSUBSET:
    {
      //Arg1 - Dst
      //Arg2 - Src
      //Arg3 - Index
      //Arg4 - Length

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];

      NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);

      ArrayCount2 = cCmdArrayCount(Arg2, 0);

      if (Arg3 != NOT_A_DS_ID)
      {
        pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);
        ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
      }
      else //Index input unwired
      {
        ArgVal3 = 0;
      }

      if (Arg4 != NOT_A_DS_ID)
      {
        pArg4 = cCmdResolveDataArg(Arg4, 0, &TypeCode4);
        ArgVal4 = cCmdGetVal(pArg4, TypeCode4);
      }
      else //Length input unwired, set to "rest"
      {
        ArgVal4 = (UWORD)(ArrayCount2 - ArgVal3);
      }

      //Bounds check
      if (ArgVal3 > ArrayCount2)
      {
        //Illegal range - return empty subset
        Status = cCmdDSArrayAlloc(Arg1, 0, 0);
        return Status;
      }

      //Set MinCount to "rest"
      MinCount = (UWORD)(ArrayCount2 - ArgVal3);

      // Copy "Length" if it is less than "rest"
      if (ArgVal4 < (ULONG)MinCount)
        MinCount = (UWORD)ArgVal4;

      //Allocate Dst array
      Status = cCmdDSArrayAlloc(Arg1, 0, MinCount);
      if (IS_ERR(Status))
        return Status;

      DVIndex1 = cCmdGetDVIndex(Arg1, 0);
      DVIndex2 = cCmdGetDVIndex(Arg2, 0);

      //Move src subset to dst
      for (i = 0; i < MinCount; i++)
      {
        Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg1), ARRAY_ELEM_OFFSET(DVIndex1, i), INC_ID(Arg2), ARRAY_ELEM_OFFSET(DVIndex2, ArgVal3 + i));
        if (IS_ERR(Status))
          return Status;
      }
    }
    break;

    case OP_STRSUBSET:
    {
      //Arg1 - Dst
      //Arg2 - Src
      //Arg3 - Index
      //Arg4 - Length

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];

      NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg1)) == TC_UBYTE);
      NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg2)) == TC_UBYTE);

      ArrayCount2 = cCmdArrayCount(Arg2, 0);

      //Remove NULL from Count
      ArrayCount2--;

      if (Arg3 != NOT_A_DS_ID)
      {
        pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);
        ArgVal3 = cCmdGetVal(pArg3, TypeCode3);
      }
      else //Index input unwired
      {
        ArgVal3 = 0;
      }

      if (Arg4 != NOT_A_DS_ID)
      {
        pArg4 = cCmdResolveDataArg(Arg4, 0, &TypeCode4);
        ArgVal4 = cCmdGetVal(pArg4, TypeCode4);
      }
      else //Length input unwired, set to "rest"
      {
        ArgVal4 = (UWORD)(ArrayCount2 - ArgVal3);
      }

      //Bounds check
      if (ArgVal3 > ArrayCount2)
      {
        //Illegal range - return empty string
        Status = cCmdDSArrayAlloc(Arg1, 0, 1);
        if (!IS_ERR(Status))
        {
          pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
          *((UBYTE *)pArg1) = '\0';
        }
        return Status;
      }

      //Set MinCount to "rest"
      MinCount = (UWORD)(ArrayCount2 - ArgVal3);

      // Copy "Length" if it is less than "rest"
      if (ArgVal4 < (ArrayCount2 - ArgVal3))
        MinCount = (UWORD)ArgVal4;

      //Allocate Dst array
      Status = cCmdDSArrayAlloc(Arg1, 0, (UWORD)(MinCount + 1));
      if (IS_ERR(Status))
        return Status;

      pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
      pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);

      //Move src subset to dst
      memmove((UBYTE *)pArg1, (UBYTE *)pArg2 + ArgVal3, MinCount);

      //Append NULL terminator to Dst
      *((UBYTE *)pArg1 + MinCount) = '\0';

    }
    break;

    case OP_SETOUT:
    {
      Status = cCmdExecuteSetOut(pCode);
    }
    break;

    case OP_ARRBUILD:
    {
      // Arg1 - Instruction Size in bytes
      // Arg2 - Dst
      // Arg3-N - Srcs

      Arg2 = pCode[2];

      NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);

      //Number of Srcs = total code words - 3 (account for opcode word, size, and Dst)
      //!!! Argument access like this is potentially unsafe.
      //A function/macro which checks proper encoding would be better
      SrcCount = (pCode[1] / 2) - 3;

      //Calculate Dst array count
      ArrayCount2 = 0;
      for (i = 0; i < SrcCount; i++)
      {
        TmpDSID = pCode[3 + i];
        NXT_ASSERT(cCmdIsDSElementIDSane(TmpDSID));

        //If the type descriptors are the same, then the input is an array, not a single element
        if (cCmdCompareDSType(Arg2, TmpDSID))
        {
          NXT_ASSERT(cCmdDSType(TmpDSID) == TC_ARRAY);
          ArrayCount2 += cCmdArrayCount(TmpDSID, 0);
        }
        else
        {
          //Assert that the output is an array of this input type
          NXT_ASSERT(cCmdCompareDSType(INC_ID(Arg2), TmpDSID));
          ArrayCount2++;
        }
      }

      //Allocate Dst array
      Status = cCmdDSArrayAlloc(Arg2, 0, ArrayCount2);
      if (IS_ERR(Status))
        return Status;

      DVIndex2 = cCmdGetDVIndex(Arg2, 0);

      //Move Src(s) to Dst
      DstIndex = 0;
      for (i = 0; i < SrcCount; i++)
      {
        TmpDSID = pCode[3 + i];

        //If the type descriptors are the same, then the input is an array, not a single element
        if (cCmdCompareDSType(Arg2, TmpDSID))
        {
          NXT_ASSERT(cCmdDSType(TmpDSID) == TC_ARRAY);
          TmpDVIndex = cCmdGetDVIndex(TmpDSID, 0);
          for (j = 0; j < DV_ARRAY[TmpDVIndex].Count; j++)
          {
            Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg2), ARRAY_ELEM_OFFSET(DVIndex2, DstIndex), INC_ID(TmpDSID), ARRAY_ELEM_OFFSET(TmpDVIndex, j));
            if (IS_ERR(Status))
              return Status;
            DstIndex++;
          }
        }
        else
        {
          //Assert that the output is an array of this input type
          NXT_ASSERT(cCmdCompareDSType(INC_ID(Arg2), TmpDSID));
          Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg2), ARRAY_ELEM_OFFSET(DVIndex2, DstIndex), TmpDSID, 0);
          if (IS_ERR(Status))
            return Status;
          DstIndex++;
        }
      }

      NXT_ASSERT(DstIndex == ArrayCount2);
    }
    break;

    case OP_STRCAT:
    {
      // Arg1 - Instruction Size in bytes
      // Arg2 - Dst
      // Arg3-N - Srcs

      Arg2 = pCode[2];

      //Make sure Dst arg is a string
      NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg2)) == TC_UBYTE);

      //Number of Srcs = total code words - 3 (account for opcode word, size, and Dst)
      //!!! Argument access like this is potentially unsafe.
      //A function/macro which checks proper encoding would be better
      SrcCount = (pCode[1] / 2) - 3;

      //Calculate Dst array count
      ArrayCount2 = 0;
      for (i = 0; i < SrcCount; i++)
      {
        TmpDSID = pCode[3 + i];
        NXT_ASSERT(cCmdIsDSElementIDSane(TmpDSID));

        //Make sure Src arg is a string
        //!!! Type checks here should be richer to allow array of strings as input (match LabVIEW behavior)
        NXT_ASSERT(cCmdDSType(TmpDSID) == TC_ARRAY);

        if (cCmdDSType(INC_ID(TmpDSID)) != TC_UBYTE)
        {
          NXT_BREAK;
          return ERR_ARG;
        }

        ArrayCount3 = cCmdArrayCount(TmpDSID, 0);
        NXT_ASSERT(ArrayCount3 > 0);
        //Subtract NULL terminator from Src array count
        ArrayCount3--;

        //Increase Dst array count by Src array count
        ArrayCount2 += ArrayCount3;
      }

      //Add room for NULL terminator
      ArrayCount2++;

      //Allocate Dst array
      Status = cCmdDSArrayAlloc(Arg2, 0, ArrayCount2);
      if (IS_ERR(Status))
        return Status;

      //Move Src(s) to Dst
      DstIndex = 0;
      pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);
      for (i = 0; i < SrcCount; i++)
      {
        TmpDSID = pCode[3 + i];

        pArg3 = cCmdResolveDataArg(TmpDSID, 0, NULL);

        ArrayCount3 = cCmdArrayCount(TmpDSID, 0);
        NXT_ASSERT(ArrayCount3 > 0);
        //Subtract NULL terminator from Src array count
        ArrayCount3--;

        memmove((UBYTE *)pArg2 + DstIndex, pArg3, ArrayCount3);
        DstIndex += ArrayCount3;
      }

      //Append NULL terminator to Dst
      *((UBYTE *)pArg2 + DstIndex) = '\0';
      DstIndex++;

      NXT_ASSERT(DstIndex == ArrayCount2);
    }
    break;

    case OP_UNFLATTEN:
    {
      //Arg1 - Dst
      //Arg2 - Err (output)
      //Arg3 - Src (byte stream)
      //Arg4 - Type

      //The Type arg is a preallocated structure of the exact size you
      //want to unflatten into. This allows us to support unflattening arbitrary types.

      //!!! Currently, both outputs must have valid destinations.
      //    It would be trivial to handle NOT_A_DS_ID to avoid dummy
      //     allocations when outputs are unused.

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];

      //Move Type template to Dst
      //This provides a default value for Dst and makes sure Dst is properly sized		
      Status = cCmdInterpPolyUnop2(OP_MOV, Arg1, 0, Arg4, 0);
      if (IS_ERR(Status))
        return Status;

      //Resolve error data pointer
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);

      //Make sure Arg3 is a String
      NXT_ASSERT(cCmdDSType(Arg3) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg3)) == TC_UBYTE);

      ArrayCount3 = cCmdArrayCount(Arg3, 0);
      //Take NULL terminator out of count
      ArrayCount3--;

      Size = cCmdCalcFlattenedSize(Arg4, 0);

      //Check that we have a proper type template to unflatten into
      if (ArrayCount3 == Size)
      {
        pArg3 = cCmdResolveDataArg(Arg3, 0, NULL);
        Offset = 0;
        Status = cCmdUnflattenFromByteArray(pArg3, &Offset, Arg1, 0);

        //!!! Status ignored from cCmdUnflattenFromByteArray
        //    If future revisions of this function provide better error checking,
        //    Err arg should be conditionally set based on the result.
        //Unflatten succeeded; set Err arg to FALSE
        cCmdSetVal(pArg2, TypeCode2, FALSE);

        NXT_ASSERT(Offset == Size);
      }
      else
      {
        //Unflatten failed; set Err arg to TRUE
        cCmdSetVal(pArg2, TypeCode2, TRUE);
      }
    }
    break;

    case OP_STRINGTONUM:
    {

      // Arg1 - Dst number (output)
      // Arg2 - Offset past match (output)
      // Arg3 - Src string
      // Arg4 - Offset
      // Arg5 - Default (type/value)

      //!!! Currently, both outputs must have valid destinations.
      //    It would be trivial to handle NOT_A_DS_ID to avoid dummy
      //     allocations when outputs are unused.

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];
      Arg5 = pCode[5];

      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
      pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);

      if (Arg4 != NOT_A_DS_ID)
      {
        pArg4 = cCmdResolveDataArg(Arg4, 0, &TypeCode4);
        ArgVal4 = cCmdGetVal(pArg4, TypeCode4);
      }
      else //Offset input unwired
      {
        ArgVal4 = 0;
      }

      if (Arg5 != NOT_A_DS_ID)
      {
        pArg5 = cCmdResolveDataArg(Arg5, 0, &TypeCode5);
        ArgVal5 = cCmdGetVal(pArg5, TypeCode5);
      }
      else //Default input unwired
      {
        ArgVal5 = 0;
      }

      //Read number from string
      if (sscanf(((PSZ)pArg3 + ArgVal4), "%d", &ArgVal1) == 1)
      {
        i = (UWORD)ArgVal4;
        //Scan until we see the number
        while ((((UBYTE *)pArg3)[i] < '0') || (((UBYTE *)pArg3)[i] > '9'))
          i++;

        //Scan until we get past the number
        while ((((UBYTE *)pArg3)[i] >= '0') && (((UBYTE *)pArg3)[i] <= '9'))
          i++;

        ArgVal2 = i;
      }
      else
      {
        //Number wasn't found in string, use defaults
        ArgVal1 = ArgVal5;
        ArgVal2 = 0;
      }

      //Set outputs
      cCmdSetVal(pArg1, TypeCode1, ArgVal1);
      cCmdSetVal(pArg2, TypeCode2, ArgVal2);
    }
    break;

    default:
    {
      //Fatal error: Unrecognized instruction
      NXT_BREAK;
      Status = ERR_INSTR;
    }
    break;
  }

  return (Status);
}


//
//Support functions for lowspeed (I2C devices, i.e. ultrasonic sensor) communications
//

//Simple lookup table for pMapLowSpeed->ChannelState[Port] values
//This is used to keep VM status code handling consistent
//...and ChannelState gives us too much information, anyway...
static const NXT_STATUS MapLStoVMStat[6] =
{
  NO_ERR,             //LOWSPEED_IDLE,
  STAT_COMM_PENDING,  //LOWSPEED_INIT,
  STAT_COMM_PENDING,  //LOWSPEED_LOAD_BUFFER,
  STAT_COMM_PENDING,  //LOWSPEED_COMMUNICATING,
  ERR_COMM_BUS_ERR,   //LOWSPEED_ERROR,
  STAT_COMM_PENDING,  //LOWSPEED_DONE (really means c_lowspeed state machine is resetting)
};


//cCmdLSCheckStatus
//Check lowspeed port status, optionally returning bytes available in the buffer for reading
NXT_STATUS cCmdLSCheckStatus(UBYTE Port)
{
  if (Port >= NO_OF_LOWSPEED_COM_CHANNEL)
  {
    return (ERR_COMM_CHAN_INVALID);
  }

  //If port is not configured properly ahead of time, report that error
  //!!! This seems like the right policy, but may restrict otherwise valid read operations...
  if (!(pMapInput->Inputs[Port].SensorType == LOWSPEED_9V || pMapInput->Inputs[Port].SensorType == LOWSPEED)
   || !(pMapInput->Inputs[Port].InvalidData == FALSE))
  {
    return (ERR_COMM_CHAN_NOT_READY);
  }

  return (MapLStoVMStat[pMapLowSpeed->ChannelState[Port]]);
}

//cCmdLSCalcBytesReady
//Calculate true number of bytes available in the inbound LS buffer
UBYTE cCmdLSCalcBytesReady(UBYTE Port)
{
  SLONG Tmp;

  //Expect callers to validate Port, but short circuit here to be safe.
  if (Port >= NO_OF_LOWSPEED_COM_CHANNEL)
    return 0;

  //Normally, bytes available is a simple difference.
  Tmp = pMapLowSpeed->InBuf[Port].InPtr - pMapLowSpeed->InBuf[Port].OutPtr;

  //If InPtr is actually behind OutPtr, circular buffer has wrapped.  Account for wrappage...
  if (Tmp < 0)
    Tmp = (pMapLowSpeed->InBuf[Port].InPtr + (SIZE_OF_LSBUF - pMapLowSpeed->InBuf[Port].OutPtr));

  return (UBYTE)(Tmp);
}

//cCmdLSWrite
//Write BufLength bytes into specified port's lowspeed buffer and kick off comm process to device
NXT_STATUS cCmdLSWrite(UBYTE Port, UBYTE BufLength, UBYTE *pBuf, UBYTE ResponseLength)
{
  if (Port >= NO_OF_LOWSPEED_COM_CHANNEL)
  {
    return (ERR_COMM_CHAN_INVALID);
  }

  if (BufLength > SIZE_OF_LSBUF || ResponseLength > SIZE_OF_LSBUF)
  {
    return (ERR_INVALID_SIZE);
  }

  //Only start writing process if port is properly configured and c_lowspeed module is ready
  if ((pMapInput->Inputs[Port].SensorType == LOWSPEED_9V || pMapInput->Inputs[Port].SensorType == LOWSPEED)
   && (pMapInput->Inputs[Port].InvalidData == FALSE)
   && (pMapLowSpeed->ChannelState[Port] == LOWSPEED_IDLE) || (pMapLowSpeed->ChannelState[Port] == LOWSPEED_ERROR))
  {
    pMapLowSpeed->OutBuf[Port].InPtr = 0;
    pMapLowSpeed->OutBuf[Port].OutPtr = 0;

    memcpy(pMapLowSpeed->OutBuf[Port].Buf, pBuf, BufLength);
    pMapLowSpeed->OutBuf[Port].InPtr = (UBYTE)BufLength;

    pMapLowSpeed->InBuf[Port].BytesToRx = ResponseLength;

    pMapLowSpeed->ChannelState[Port] = LOWSPEED_INIT;
    pMapLowSpeed->State |= (COM_CHANNEL_ONE_ACTIVE << Port);

    return (NO_ERR);
  }
  else
  {
    //!!! Would be more consistent to return STAT_COMM_PENDING if c_lowspeed is busy
    return (ERR_COMM_CHAN_NOT_READY);
  }
}


//cCmdLSRead
//Read BufLength bytes from specified port's lowspeed buffer
NXT_STATUS cCmdLSRead(UBYTE Port, UBYTE BufLength, UBYTE * pBuf)
{
  UBYTE BytesReady, BytesToRead;

  if (Port >= NO_OF_LOWSPEED_COM_CHANNEL)
  {
    return (ERR_COMM_CHAN_INVALID);
  }

  if (BufLength > SIZE_OF_LSBUF)
  {
    return (ERR_INVALID_SIZE);
  }

  BytesReady = cCmdLSCalcBytesReady(Port);

  if (BufLength > BytesReady)
  {
    return (ERR_COMM_CHAN_NOT_READY);
  }

  BytesToRead = BufLength;

  //If the bytes we want to read wrap around the end, we must first read the end, then reset back to the beginning
  if (pMapLowSpeed->InBuf[Port].OutPtr + BytesToRead >= SIZE_OF_LSBUF)
  {
    BytesToRead = SIZE_OF_LSBUF - pMapLowSpeed->InBuf[Port].OutPtr;
    memcpy(pBuf, pMapLowSpeed->InBuf[Port].Buf + pMapLowSpeed->InBuf[Port].OutPtr, BytesToRead);
    pMapLowSpeed->InBuf[Port].OutPtr = 0;
    pBuf += BytesToRead;
    BytesToRead = BufLength - BytesToRead;
  }

  memcpy(pBuf, pMapLowSpeed->InBuf[Port].Buf + pMapLowSpeed->InBuf[Port].OutPtr, BytesToRead);
  pMapLowSpeed->InBuf[Port].OutPtr += BytesToRead;

  return (NO_ERR);
}


//
//Wrappers for OP_SYSCALL
//

//
//cCmdWrapFileOpenRead
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenRead(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(OPENREAD, ArgV[2], NULL, (ULONG *)ArgV[3]);

  //Add entry into FileHandleTable
  if (LOADER_ERR(LStatus) == SUCCESS)
  {
    VarsCmd.FileHandleTable[LOADER_HANDLE(LStatus)][0] = 'r';
    strcpy((PSZ)(VarsCmd.FileHandleTable[LOADER_HANDLE(LStatus)] + 1), (PSZ)(ArgV[2]));
  }

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return NO_ERR;
}

//cCmdWrapFileOpenWrite
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenWrite(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(OPENWRITEDATA, ArgV[2], NULL, (ULONG *)ArgV[3]);

  //Add entry into FileHandleTable
  if (LOADER_ERR(LStatus) == SUCCESS)
  {
    VarsCmd.FileHandleTable[LOADER_HANDLE(LStatus)][0] = 'w';
    strcpy((PSZ)(VarsCmd.FileHandleTable[LOADER_HANDLE(LStatus)] + 1), (PSZ)(ArgV[2]));
  }

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return NO_ERR;
}

//cCmdWrapFileOpenAppend
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length Remaining, U32 return
NXT_STATUS cCmdWrapFileOpenAppend(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(OPENAPPENDDATA, ArgV[2], NULL, (ULONG *)ArgV[3]);

  //Add entry into FileHandleTable
  if (LOADER_ERR(LStatus) == SUCCESS)
  {
    VarsCmd.FileHandleTable[LOADER_HANDLE(LStatus)][0] = 'w';
    strcpy((PSZ)(VarsCmd.FileHandleTable[LOADER_HANDLE(LStatus)] + 1), (PSZ)(ArgV[2]));
  }

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return NO_ERR;
}

//cCmdWrapFileRead
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 in/out
//ArgV[2]: Buffer, CStr out
//ArgV[3]: Length, U32 in/out
NXT_STATUS cCmdWrapFileRead(UBYTE * ArgV[])
{
  NXT_STATUS Status = NO_ERR;
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  //Size Buffer to Length
  //Add room for null terminator to length
  Status = cCmdDVArrayAlloc(DVIndex, (UWORD)(*(ULONG *)ArgV[3] + 1));
  if (IS_ERR(Status))
    return Status;

  ArgV[2] = cCmdDVPtr(DVIndex);
  LStatus = pMapLoader->pFunc(READ, ArgV[1], ArgV[2], (ULONG *)ArgV[3]);

  //Tack on NULL terminator
  //Note that loader code may have adjusted length (*ArgV[3]) if all requested data was not available
  //!!! Better solution would be to resize buffer to new length + 1,
  //     but then you must also be wary of side effects if resize allocation fails!
  *(ArgV[2] + *(ULONG *)ArgV[3]) = '\0';

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return Status;
}

//cCmdWrapFileWrite
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 in/out
//ArgV[2]: Buffer, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileWrite(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(WRITE, ArgV[1], ArgV[2], (ULONG *)ArgV[3]);

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return NO_ERR;
}

//cCmdWrapFileClose
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8
NXT_STATUS cCmdWrapFileClose(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;

  //!!! This bounds check also exists in dLoaderCloseHandle(), but we provide an explicit error code
  if (*(ArgV[1]) >= MAX_HANDLES)
  {
    *((UWORD *)ArgV[0]) = ILLEGALHANDLE;
    return NO_ERR;
  }

  LStatus = pMapLoader->pFunc(CLOSE, ArgV[1], NULL, NULL);

  //Clear entry in FileHandleTable
  memset(VarsCmd.FileHandleTable[*(ArgV[1])], 0, FILENAME_LENGTH + 2);

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);

  return NO_ERR;
}

//cCmdWrapFileResolveHandle
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Write Handle?, Bool return
//ArgV[3]: Filename, CStr
NXT_STATUS cCmdWrapFileResolveHandle (UBYTE * ArgV[])
{
  UBYTE i;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[3]);
  ArgV[3] = cCmdDVPtr(DVIndex);

  for (i = 0; i < MAX_HANDLES; i++)
  {
    if (strcmp((PSZ)(ArgV[3]), (PSZ)(VarsCmd.FileHandleTable[i] + 1)) == 0)
    {
      *(ArgV[2]) = (VarsCmd.FileHandleTable[i][0] == 'w');
      break;
    }
  }

  if (i == MAX_HANDLES)
  {
    i = NOT_A_HANDLE;
    *((UWORD *)ArgV[0]) = HANDLEALREADYCLOSED;
  }
  else
  {
    *((UWORD *)ArgV[0]) = SUCCESS;
  }

  *(ArgV[1]) = i;

  return NO_ERR;
}


//cCmdWrapFileRename
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: Old Filename, CStr
//ArgV[2]: New Filename, CStr
NXT_STATUS cCmdWrapFileRename (UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  ULONG Tmp;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  ArgV[1] = cCmdDVPtr(DVIndex);
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  //!!! Tmp placeholder passed into loader code to avoid illegal dereferencing.
  LStatus = pMapLoader->pFunc(RENAMEFILE, ArgV[1], ArgV[2], &Tmp);

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);

  return NO_ERR;
}


//cCmdWrapFileDelete
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: Filename, CStr
NXT_STATUS cCmdWrapFileDelete (UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  ArgV[1] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(DELETE, ArgV[1], NULL, NULL);

  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);

  return NO_ERR;
}

//
//cCmdWrapSoundPlayFile
//ArgV[0]: (Return value) Status code, SBYTE
//ArgV[1]: Filename, CStr
//ArgV[2]: Loop?, UBYTE (bool)
//ArgV[3]: Volume, UBYTE
//
NXT_STATUS cCmdWrapSoundPlayFile(UBYTE * ArgV[])
{
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  ArgV[1] = cCmdDVPtr(DVIndex);

  //!!! Should check filename and/or existence and return error before proceeding
  strncpy((PSZ)(pMapSound->SoundFilename), (PSZ)(ArgV[1]), FILENAME_LENGTH);

  if (*(ArgV[2]) == TRUE)
    pMapSound->Mode = SOUND_LOOP;
  else
    pMapSound->Mode = SOUND_ONCE;

  pMapSound->Volume = *(ArgV[3]);
  //SampleRate of '0' means "let file specify SampleRate"
  pMapSound->SampleRate = 0;
  pMapSound->Flags |= SOUND_UPDATE;

  *((SBYTE*)(ArgV[0])) = (NO_ERR);

  return (NO_ERR);
}

//
//cCmdWrapSoundPlayTone
//ArgV[0]: (Return value) Status code, SBYTE
//ArgV[1]: Frequency, UWORD
//ArgV[2]: Duration, UWORD
//ArgV[3]: Loop?, UBYTE (Boolean)
//ArgV[4]: Volume, UBYTE
//
NXT_STATUS cCmdWrapSoundPlayTone(UBYTE * ArgV[])
{
  pMapSound->Freq = *(UWORD*)(ArgV[1]);
  pMapSound->Duration = *(UWORD*)(ArgV[2]);
  pMapSound->Volume = *(ArgV[4]);
  pMapSound->Flags |= SOUND_UPDATE;

  if (*(ArgV[3]) == TRUE)
    pMapSound->Mode = SOUND_TONE | SOUND_LOOP;
  else
    pMapSound->Mode = SOUND_TONE;

  *((SBYTE*)(ArgV[0])) = (NO_ERR);

  return (NO_ERR);
}

//
//cCmdWrapSoundGetState
//ArgV[0]: (Return value) sound module state, UBYTE
//ArgV[1]: Flags, UBYTE
//
NXT_STATUS cCmdWrapSoundGetState(UBYTE * ArgV[])
{
  *(ArgV[0]) = pMapSound->State;
  *(ArgV[1]) = pMapSound->Flags;
  return (NO_ERR);
}

//
//cCmdWrapSoundSetState
//ArgV[0]: (Return value) sound module state, UBYTE
//ArgV[1]: State, UBYTE
//ArgV[2]: Flags, UBYTE
//
NXT_STATUS cCmdWrapSoundSetState(UBYTE * ArgV[])
{
  pMapSound->State = *(ArgV[1]);
  //Return same state we just set, mostly for interface consistency
  *(ArgV[0]) = pMapSound->State;

  //OR in provided flags (usually 0)
  pMapSound->Flags |= *(ArgV[2]);

  return (NO_ERR);
}

//
//cCmdWrapReadButton
//ArgV[0]: (Function return) Status code, SBYTE
//ArgV[1]: Index (U8)
//ArgV[2]: Pressed (bool)
//ArgV[3]: Count (U8) (count of press-then-release cycles)
//ArgV[4]: ResetCount? (bool in)
//
NXT_STATUS cCmdWrapReadButton(UBYTE * ArgV[])
{
  UBYTE btnIndex;

  btnIndex = *((UBYTE*)(ArgV[1]));

  if (btnIndex < NO_OF_BTNS)
  {
    //Set pressed boolean output
    if (pMapButton->State[btnIndex] & PRESSED_STATE)
      *(ArgV[2]) = TRUE;
    else
      *(ArgV[2]) = FALSE;

    //Set count output
    *(ArgV[3]) = (UBYTE)(pMapButton->BtnCnt[btnIndex].RelCnt);

    //Optionally reset internal count
    if (*(ArgV[4]) != 0)
    {
      pMapButton->BtnCnt[btnIndex].RelCnt = 0;
      //Need to clear short and long counts too, because RelCnt depends on them.  No known side effects.
      pMapButton->BtnCnt[btnIndex].ShortRelCnt = 0;
      pMapButton->BtnCnt[btnIndex].LongRelCnt = 0;
    }

    // Set status code 'OK'
    *((SBYTE*)(ArgV[0])) = NO_ERR;
  }
  else
  {
    //Bad button index specified, return error and default outputs
    *((SBYTE*)(ArgV[0])) = ERR_INVALID_PORT;
    *(ArgV[2]) = FALSE;
    *(ArgV[3]) = 0;
  }

  return (NO_ERR);
}

//
//cCmdWrapCommLSWrite
//ArgV[0]: (return) Status code, SBYTE
//ArgV[1]: Port specifier, UBYTE
//ArgV[2]: Buffer to send, UBYTE array, only SIZE_OF_LSBUF bytes will be used
//ArgV[3]: ResponseLength, UBYTE, specifies expected bytes back from slave device
//
NXT_STATUS cCmdWrapCommLSWrite(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UBYTE Port = *(ArgV[1]);
  UBYTE * pBuf;
  UWORD BufLength;
  UBYTE ResponseLength = *(ArgV[3]);
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  pBuf = cCmdDVPtr(DVIndex);
  BufLength = DV_ARRAY[DVIndex].Count;

  *pReturnVal = cCmdLSWrite(Port, (UBYTE)BufLength, pBuf, ResponseLength);

  return (NO_ERR);
}

//
//cCmdWrapCommLSCheckStatus
//ArgV[0]: (return) Status code, SBYTE
//ArgV[1]: Port specifier, UBYTE
//ArgV[2]: BytesReady, UBYTE
//
NXT_STATUS cCmdWrapCommLSCheckStatus(UBYTE * ArgV[])
{
  UBYTE Port = *(ArgV[1]);

  *((SBYTE*)(ArgV[0])) = cCmdLSCheckStatus(Port);
  *((UBYTE*)(ArgV[2])) = cCmdLSCalcBytesReady(Port);

  return (NO_ERR);
}

//
//cCmdWrapCommLSRead
//ArgV[0]: (return) Status code, SBYTE
//ArgV[1]: Port specifier, UBYTE
//ArgV[2]: Buffer for data, UBYTE array, max SIZE_OF_LSBUF bytes will be written
//ArgV[3]: BufferLength, UBYTE, specifies size of buffer requested
//
NXT_STATUS cCmdWrapCommLSRead(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UBYTE Port = *(ArgV[1]);
  UBYTE * pBuf;
  UBYTE BufLength = *(ArgV[3]);
  UBYTE BytesToRead;
  DV_INDEX DVIndex = *(DV_INDEX *)(ArgV[2]);
  NXT_STATUS AllocStatus;

  *pReturnVal = cCmdLSCheckStatus(Port);
  BytesToRead = cCmdLSCalcBytesReady(Port);

  //If channel is OK and has data ready for us, put the data into outgoing buffer
  if (!IS_ERR(*pReturnVal) && BytesToRead > 0)
  {
    //Limit buffer to available data
    if (BufLength > BytesToRead)
      BufLength = BytesToRead;

    AllocStatus = cCmdDVArrayAlloc(DVIndex, BufLength);
    if (IS_ERR(AllocStatus))
      return (AllocStatus);

    pBuf = cCmdDVPtr(DVIndex);
    *pReturnVal = cCmdLSRead(Port, BufLength, pBuf);
  }
  //Else, the channel has an error and/or there's no data to read; clear the output array
  else
  {
    AllocStatus = cCmdDVArrayAlloc(DVIndex, 0);
    if (IS_ERR(AllocStatus))
      return (AllocStatus);
  }

  return (NO_ERR);
}

//
//cCmdWrapRandomNumber
//ArgV[0]: (return) Random number, SWORD
//
NXT_STATUS cCmdWrapRandomNumber(UBYTE * ArgV[])
{
  static UBYTE count = 0;
  SWORD random;

  if (count == 0)
    srand(dTimerRead());

  if (count > 20)
    count = 0;
  else
    count++;

  //!!! IAR's implementation of the rand() library function returns signed values, and we want it that way.
  //Some stdlib implementations may return only positive numbers, so be wary if this code is ported.
  random = rand();

  *((SWORD *)ArgV[0]) = random;

  return NO_ERR;
}

//
//cCmdWrapGetStartTick
//ArgV[0]: (return) Start Tick, ULONG
//
NXT_STATUS cCmdWrapGetStartTick(UBYTE * ArgV[])
{
  *((ULONG *)ArgV[0]) = VarsCmd.StartTick;
  return NO_ERR;
}

//
//cCmdWrapMessageWrite
//ArgV[0]: (return) Error Code, SBYTE (NXT_STATUS)
//ArgV[1]: QueueID, UBYTE
//ArgV[2]: Message, CStr
//
NXT_STATUS cCmdWrapMessageWrite(UBYTE * ArgV[])
{
  NXT_STATUS Status = NO_ERR;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  Status = cCmdMessageWrite(*(UBYTE *)(ArgV[1]), ArgV[2], DV_ARRAY[DVIndex].Count);

  *(SBYTE *)(ArgV[0]) = Status;

  if (IS_FATAL(Status))
    return Status;
  else
    return (NO_ERR);
}

#define UNPACK_STATUS(StatusWord) ((SBYTE)(StatusWord))

NXT_STATUS cCmdBTCheckStatus(UBYTE Connection)
{
  //If specified connection is invalid, return error code to the user.
  if (Connection >= SIZE_OF_BT_CONNECT_TABLE)
  {
    return (ERR_INVALID_PORT);
  }

  //INPROGRESS means a request is currently pending completion by the comm module
  if (VarsCmd.CommStat == INPROGRESS)
  {
    return (STAT_COMM_PENDING);
  }
  //Translate BTBUSY to ERR_COMM_CHAN_NOT_READY
  //And check if specified connection is indeed configured
  else if (VarsCmd.CommStat == (SWORD)BTBUSY
       || (pMapComm->BtConnectTable[Connection].Name[0]) == '\0')
  {
    return (ERR_COMM_CHAN_NOT_READY);
  }
  else
  {
    return (UNPACK_STATUS(VarsCmd.CommStat));
  }
}

//Default packet to send for a remote MESSAGE_READ command.
//3rd byte must be replaced with remote mailbox (QueueID)
//4th byte must be replaced with local mailbox
static UBYTE RemoteMsgReadPacket[5] = {0x00, 0x13, 0xFF, 0xFF, 0x01};

//
//cCmdWrapMessageRead
//ArgV[0]: (return) Error Code, SBYTE (NXT_STATUS)
//ArgV[1]: QueueID, UBYTE
//ArgV[2]: Remove, UBYTE
//ArgV[3]: (return) Message, CStr
//
NXT_STATUS cCmdWrapMessageRead(UBYTE * ArgV[])
{
  NXT_STATUS Status = NO_ERR;
  NXT_STATUS AllocStatus = NO_ERR;
  UBYTE QueueID = *(UBYTE *)(ArgV[1]);
  DV_INDEX DestDVIndex = *(DV_INDEX *)(ArgV[3]);
  UWORD MessageSize;
  UBYTE i;

  NXT_ASSERT(IS_DV_INDEX_SANE(DestDVIndex));

  //Check Next Message's size
  Status = cCmdMessageGetSize(QueueID, &MessageSize);

  //If there is a valid message in local mailbox, read it
  if (!IS_ERR(Status) && MessageSize > 0 )
  {
    //!!! Also check for EMPTY_MAILBOX status?
    //Size destination string
    AllocStatus = cCmdDVArrayAlloc(DestDVIndex, MessageSize);
    if (IS_ERR(AllocStatus))
      return AllocStatus;

    //Get Message
    //!!! Should more aggressively enforce null termination before blindly copying to dataspace
    Status = cCmdMessageRead(QueueID, cCmdDVPtr(DestDVIndex), MessageSize, *(ArgV[2]));
  }
  else
  {
    //Clear destination string
    AllocStatus = cCmdDVArrayAlloc(DestDVIndex, 1);
    if (IS_ERR(AllocStatus))
      return AllocStatus;

    //Successful allocation, make sure first byte is null terminator
    *(UBYTE*)(cCmdDVPtr(DestDVIndex)) = '\0';
  }

  //If there were no local messages, see if there are any waiting in our slaves' outboxes
  if (Status == STAT_MSG_EMPTY_MAILBOX && QueueID < INCOMING_QUEUE_COUNT)
  {
    //If there's an old error code hanging around, clear it before proceeding.
    //!!! Clearing error here means bytecode status checking loops could get false SUCCESS results?
    if (VarsCmd.CommStat < 0)
      VarsCmd.CommStat = SUCCESS;

    //Search through possible slaves, looking for valid connection
    for (i = 0; i < SIZE_OF_BT_CONNECT_TABLE - 1; i++)
    {
      //Advance CommCurrConnection and limit to 1, 2, or 3 (only slave connection slots are checked)
      VarsCmd.CommCurrConnection++;
      if (VarsCmd.CommCurrConnection == SIZE_OF_BT_CONNECT_TABLE)
        VarsCmd.CommCurrConnection = 1;

      if (cCmdBTCheckStatus(VarsCmd.CommCurrConnection) == NO_ERR)
        break;
    }

    //If there is at least one configured slave connection, make a remote read request
    if (i < SIZE_OF_BT_CONNECT_TABLE - 1)
    {
      //Outgoing QueueID on slave device is the local QueueID + INCOMING_QUEUE_COUNT
      RemoteMsgReadPacket[2] = QueueID + INCOMING_QUEUE_COUNT;
      RemoteMsgReadPacket[3] = QueueID;

      //Request comm module to send assembled packet and not go idle until response comes back (or error)
      pMapComm->pFunc(SENDDATA, sizeof(RemoteMsgReadPacket), VarsCmd.CommCurrConnection, TRUE, RemoteMsgReadPacket, (UWORD*)&(VarsCmd.CommStat));

      //Read status back after SENDDATA call so bytecode gets STAT_COMM_PENDING or error
      Status = cCmdBTCheckStatus(VarsCmd.CommCurrConnection);

      //If our request was accepted, set the DirtyComm flag so stream will get cleaned up later
      if (Status == STAT_COMM_PENDING)
        VarsCmd.DirtyComm = TRUE;
    }
  }

  *(SBYTE *)(ArgV[0]) = Status;
  if (IS_FATAL(Status))
    return Status;
  else
    return (NO_ERR);
}


//
//cCmdWrapCommBTCheckStatus
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Connection index, 0-3
//
NXT_STATUS cCmdWrapCommBTCheckStatus(UBYTE * ArgV[])
{
  *((SBYTE*)(ArgV[0])) = cCmdBTCheckStatus(*(ArgV[1]));

  return (NO_ERR);
}

//
//cCmdWrapCommBTWrite
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Connection index, 0-3
//ArgV[2]: Buffer
//
NXT_STATUS cCmdWrapCommBTWrite(UBYTE * ArgV[])
{
  SBYTE * pReturnVal =  (SBYTE*)(ArgV[0]);
  UBYTE Connection   = *(ArgV[1]);
  UBYTE * pBuf;
  UWORD BufLength;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  pBuf = cCmdDVPtr(DVIndex);

  BufLength = DV_ARRAY[DVIndex].Count;

  //If there's an old error code hanging around, clear it before proceeding.
  if (VarsCmd.CommStat < 0)
    VarsCmd.CommStat = SUCCESS;

  //!!! Only first 256 bytes could possibly make it through! Should return error on longer input?
  //!!! Not requesting a wait-for-response because only known use doesn't read responses.
  pMapComm->pFunc(SENDDATA, (UBYTE)BufLength, Connection, FALSE, pBuf, (UWORD*)&(VarsCmd.CommStat));

  //!!! Reasonable to wrap below code in cCmdCommBTCheckStatus?
  //INPROGRESS means our request was accepted by His Funkiness of pFunc
  if (VarsCmd.CommStat == (SWORD)INPROGRESS)
  {
    *pReturnVal = STAT_COMM_PENDING;

    //Set DirtyComm flag so stream is reset after program ends
    VarsCmd.DirtyComm = TRUE;
  }
  //Translate BTBUSY to ERR_COMM_CHAN_NOT_READY
  else if (VarsCmd.CommStat == (SWORD)BTBUSY)
  {
    *pReturnVal = ERR_COMM_CHAN_NOT_READY;
  }
  else
  {
    *pReturnVal = UNPACK_STATUS(VarsCmd.CommStat);
  }

  return (NO_ERR);
}

//
//cCmdWrapCommBTRead
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Count to read
//ArgV[2]: Buffer
//
NXT_STATUS cCmdWrapCommBTRead(UBYTE * ArgV[])
{
  //SBYTE * pReturnVal =  (SBYTE*)(ArgV[0]);
  //UBYTE * pBuf       =  (ArgV[2]);
  //!!! should provide length and/or connection to read?

  //!!! This syscall is not implemented; return fatal error.
  return (ERR_INSTR);
}

//
//cCmdWrapKeepAlive
//ArgV[0]: (return) Current timer limit in ms, ULONG
//
NXT_STATUS cCmdWrapKeepAlive(UBYTE * ArgV[])
{
  pMapUi->Flags |= UI_RESET_SLEEP_TIMER;

  //Convert UI's minute-based timeout value to millisecs
  //Milliseconds are the "natural" time unit in user-land.
  *(ULONG*)(ArgV[0]) = (pMapUi->SleepTimeout * 60 * 1000);

  return (NO_ERR);
}

#define MAX_IOM_BUFFER_SIZE 64
//
//cCmdWrapIOMapRead
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Module name, CStr
//ArgV[2]: Offset, UWORD
//ArgV[3]: Count, UWORD
//ArgV[4]: Buffer, UBYTE array
//
NXT_STATUS cCmdWrapIOMapRead(UBYTE * ArgV[])
{
  UWORD LStatus;
  NXT_STATUS Status;

  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UWORD Offset = *(UWORD*)(ArgV[2]);
  //Our copy of 'Count' must be a ULONG to match the loader interface
  ULONG Count = *(UWORD*)(ArgV[3]);

  DV_INDEX DVIndex;

  //Buffer for return of FINDFIRSTMODULE call, structure defined in protocol doc
  //We need it to transfer the ModuleID to the IOMAPREAD call
  UBYTE FindBuffer[FILENAME_LENGTH + 10];
  //Buffer to store data and offset in for IOMAPREAD call
  //!!! Constant size means only limited reads and writes
  UBYTE DataBuffer[MAX_IOM_BUFFER_SIZE + 2];

  if (Count > MAX_IOM_BUFFER_SIZE)
  {
    //Request to read too much data at once; clear buffer, return error.
    DVIndex = *(DV_INDEX *)(ArgV[4]);
    *pReturnVal = cCmdDVArrayAlloc(DVIndex, 0);
    if (IS_ERR(*pReturnVal))
      return (*pReturnVal);

    *pReturnVal = ERR_INVALID_SIZE;
    return (NO_ERR);
  }

  //Resolve module name
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  ArgV[1] = cCmdDVPtr(DVIndex);

  //Find module by name.  Note that wildcards are accepted, but only first match matters.
  LStatus = pMapLoader->pFunc(FINDFIRSTMODULE, ArgV[1], FindBuffer, NULL);

  if (LOADER_ERR(LStatus) == SUCCESS)
  {
    //Module was found, transfer Offset into first two bytes of DataBuffer and attempt to read
    *(UWORD*)(DataBuffer) = Offset;
    LStatus = pMapLoader->pFunc(IOMAPREAD, &(FindBuffer[FILENAME_LENGTH + 1]), DataBuffer, &Count);

    if (LOADER_ERR(LStatus) == SUCCESS)
    {
      //No error from IOMAPREAD, so copy the data into VM's dataspace
      //Size destination array
      DVIndex = *(DV_INDEX *)(ArgV[4]);
      Status = cCmdDVArrayAlloc(DVIndex, (UWORD)Count);
      if (IS_ERR(Status))
      {
        //Alloc failed, so close handle and return
        pMapLoader->pFunc(CLOSEMODHANDLE, NULL, NULL, NULL);
        return (Status);
      }

      //Alloc succeeded, so resolve and copy away
      ArgV[4] = cCmdDVPtr(DVIndex);
      memcpy(ArgV[4], &(DataBuffer[2]), Count);
    }
  }

  *pReturnVal = LOADER_ERR_BYTE(LStatus);

  pMapLoader->pFunc(CLOSEMODHANDLE, NULL, NULL, NULL);
  return (NO_ERR);
}

//
//cCmdWrapIOMapWrite
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Module name, CStr
//ArgV[2]: Offset, UWORD
//ArgV[3]: Buffer, UBYTE array
//
NXT_STATUS cCmdWrapIOMapWrite(UBYTE * ArgV[])
{
  UWORD LStatus;

  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UWORD Offset = *(UWORD*)(ArgV[2]);

  //Our copy of 'Count' must be a ULONG to match the loader interface
  ULONG Count;
  DV_INDEX DVIndex;

  //Buffer for return of FINDFIRSTMODULE call, structure defined in protocol doc
  //We need it to transfer the ModuleID to the IOMAPREAD call
  UBYTE FindBuffer[FILENAME_LENGTH + 10];
  //Buffer to store data and offset in for IOMAPREAD call
  //!!! Constant size means only limited reads and writes
  UBYTE DataBuffer[MAX_IOM_BUFFER_SIZE + 2];

  //Resolve module name and buffer
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  ArgV[1] = cCmdDVPtr(DVIndex);

  DVIndex = *(DV_INDEX *)(ArgV[3]);
  ArgV[3] = cCmdDVPtr(DVIndex);
  Count = DV_ARRAY[DVIndex].Count;

  if (Count > MAX_IOM_BUFFER_SIZE)
  {
    //Request to read too much data at once; return error and give up
    *pReturnVal = ERR_INVALID_SIZE;
    return (NO_ERR);
  }

  LStatus = pMapLoader->pFunc(FINDFIRSTMODULE, ArgV[1], FindBuffer, NULL);

  if (LOADER_ERR(LStatus) == SUCCESS)
  {
    //Module was found, transfer Offset into first two bytes of DataBuffer, copy data into rest of buffer, then write
    *(UWORD*)(DataBuffer) = Offset;
    memcpy(&(DataBuffer[2]), ArgV[3], Count);
    LStatus = pMapLoader->pFunc(IOMAPWRITE, &(FindBuffer[FILENAME_LENGTH + 1]), DataBuffer, &Count);
  }

  *pReturnVal = LOADER_ERR_BYTE(LStatus);

  pMapLoader->pFunc(CLOSEMODHANDLE, NULL, NULL, NULL);
  return (NO_ERR);
}

#if VM_BENCHMARK
void cCmdWriteBenchmarkFile()
{
  LOADER_STATUS LStatus;
  UBYTE Handle;
  ULONG BenchFileSize;
  ULONG i, Length;
  UBYTE Buffer[256];

  //Remove old benchmark file, create a new one
  strcpy((char *)Buffer, "benchmark.txt");
  pMapLoader->pFunc(DELETE, Buffer, NULL, NULL);
  BenchFileSize = 2048;
  LStatus = pMapLoader->pFunc(OPENWRITEDATA, Buffer, NULL, &BenchFileSize);

  if (!LOADER_ERR(LStatus))
  {
    //Write Benchmark file
    Handle = LOADER_HANDLE(LStatus);

    //Header
    sprintf((char *)Buffer, "Program Name: %s\r\n", VarsCmd.ActiveProgName);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "InstrCount: %d\r\n", VarsCmd.InstrCount);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "Time: %d\r\n", IOMapCmd.Tick - VarsCmd.StartTick);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "Instr/Tick: %d\r\n", VarsCmd.Average);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "CmdCtrl Calls: %d\r\n", VarsCmd.CmdCtrlCount);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "OverTime Rounds: %d\r\n", VarsCmd.OverTimeCount);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "Max OverTime Length: %d\r\n", VarsCmd.MaxOverTimeLength);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "CompactionCount: %d\r\n", VarsCmd.CompactionCount);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "LastCompactionTick: %d\r\n", VarsCmd.LastCompactionTick);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    sprintf((char *)Buffer, "MaxCompactionTime: %d\r\n", VarsCmd.MaxCompactionTime);
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);

    //opcode benchmarks
    sprintf((char *)Buffer, "Op\tCnt\tOver\tMax\r\n");
    Length = strlen((char *)Buffer);
    LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);
    for (i = 0; i < OPCODE_COUNT; i++)
    {
      sprintf((char *)Buffer, "%x\t%d\t%d\t%d\t%d\r\n", i, VarsCmd.OpcodeBenchmarks[i][0], VarsCmd.OpcodeBenchmarks[i][1], VarsCmd.OpcodeBenchmarks[i][2], VarsCmd.OpcodeBenchmarks[i][3]);
      Length = strlen((char *)Buffer);
      LStatus = pMapLoader->pFunc(WRITE, &Handle, Buffer, &Length);
    }
    //close file
    LStatus = pMapLoader->pFunc(CLOSE, &Handle, NULL, NULL);
  }
}
#endif

#ifdef SIM_NXT
// Accessors for simulator library code
SWORD cCmdGetCodeWord(CLUMP_ID Clump, CODE_INDEX Index)
{
  if (Clump == NOT_A_CLUMP)
  {
    NXT_ASSERT(Index < VarsCmd.CodespaceCount);
    return (VarsCmd.pCodespace[Index]);
  }
  else
  {
    NXT_ASSERT(cCmdIsClumpIDSane(Clump));
    return (((SWORD)VarsCmd.pCodespace[VarsCmd.pAllClumps[Clump].CodeStart + Index]));
  }
}


UBYTE * cCmdGetDataspace(UWORD *DataspaceSize)
{
  if (DataspaceSize)
    *DataspaceSize = VarsCmd.DataspaceSize;
  return (VarsCmd.pDataspace);
}


DOPE_VECTOR * cCmdGetDopeVectorPtr()
{
  return VarsCmd.MemMgr.pDopeVectorArray;
}


MEM_MGR cCmdGetMemMgr(void)
{
  return VarsCmd.MemMgr;
}


ULONG cCmdGetPoolSize()
{
  return VarsCmd.PoolSize;
}
#endif

#else //!ENABLE_VM
//
//Implementations of standard interface if VM is disabled.
//Place low-level test code here if VM is causing issues.
//Test code must implement cCmdInit(), cCmdCtrl(), and cCmdExit() at a minimum.
//Recommend using a pattern like #include "c_cmd_alternate.c"
//

//!!! !ENABLE_VM implementations really should provide a placeholder function for this pointer
//IOMapCmd.pRCHandler = &cCmdHandleRemoteCommands;
//#include "c_cmd_alternate.c"
//#include "c_cmd_FB_LowSpeed_Test.c"
//#include "c_cmd_FB_LowSpeed_JB_Compass.c"
//#include "c_cmd_FB_LowSpeed_Continius.c"
//#include "c_cmd_FB_LowSpeed_JB_Color.c"
#include "c_cmd_FB_LowSpeed_NorthStar_Demo2.c"
//#include "c_cmd_FB_LowSpeed_LEGO_TEST.c"

#endif //ENABLE_VM
