//
// Date init       14.12.2004
//
// Revision date   $Date: 26-02-10 11:38 $
//
// Filename        $Workfile:: c_cmd.c                                       $
//
// Version         $Revision: 15 $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_cmd. $
//
// Platform        C
//

//
// File Description:
// This file contains the virtual machine implementation to run bytecode
// programs compatible with LEGO MINDSTORMS NXT Software 2.0.
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
#include  "m_sched.h"

#include "c_cmd.h"
#include "c_cmd_bytecodes.h"
#include "d_timer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h> // for sqrt, abs, and trig stuff
#include <limits.h>
#include <float.h>

#define VMProfilingCode 0

static  IOMAPCMD  IOMapCmd;
static  VARSCMD   VarsCmd;
static  HEADER    **pHeaders;
static  SLONG gPCDelta;
#define NUM_INTERP_FUNCS 16
#define NUM_SHORT_INTERP_FUNCS 8
#define VAR_INSTR_SIZE 0xE
// important to cast since most args are assigned from signed value, and locals may be ULONG
#define GetDataArg(arg) ((UWORD)(arg))
#if VMProfilingCode
static  ULONG     ExecutedInstrs= 0, CmdCtrlTime= 0, OverheadTime= 0, CmdCtrlCalls= 0, LeaveTime= 0, NotFirstCall= 0, LastAvgCount= 0;
static ULONG CmdCtrlClumpTime[256];
typedef struct  {
  ULONG Time;
  ULONG Count;
  ULONG Avg;
  ULONG Max;
} VMInstrProfileInfo;
static VMInstrProfileInfo InstrProfile[OPCODE_COUNT];
static VMInstrProfileInfo SysCallProfile[SYSCALL_COUNT];
static VMInstrProfileInfo InterpFuncProfile[NUM_INTERP_FUNCS];
static VMInstrProfileInfo ShortInstrProfile[NUM_SHORT_OPCODE_COUNT];
#endif

#define cCmdDSType(Arg) (VarsCmd.pDataspaceTOC[(Arg)].TypeCode)
#define cCmdDSScalarPtr(DSElementID, Offset) (VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[DSElementID].DSOffset + Offset)
#define cCmdSizeOf(TC) (TC_Size_Table[(TC)])

#define scalarBinopDispatchMask 0x1
#define scalarUnop2DispatchMask 0x2

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
//This table is indexed by instr size
//Unary operations can have arity of 1 or 2 (some need a destination)
//All instructions taking 4 or more operands are handled as "Other"
// Table uses NoArg for illegal instr sizes such as zero and odd sizes
//
static pInterp InterpFuncs[NUM_INTERP_FUNCS] =
{
  cCmdInterpNoArg,
  cCmdInterpNoArg,
  cCmdInterpNoArg, // size 2
  cCmdInterpNoArg,
  cCmdInterpUnop1, // size 4
  cCmdInterpNoArg,
  cCmdInterpUnop2, // size 6 general poly is cCmdInterpUnop2, scalar is cCmdInterpScalarUnop2
  cCmdInterpNoArg,
  cCmdInterpBinop, // size 8, general poly is cCmdInterpBinop, scalar is cCmdInterpScalarBinop
  cCmdInterpNoArg,
  cCmdInterpOther, // size 10
  cCmdInterpNoArg,
  cCmdInterpOther, // size 12
  cCmdInterpNoArg,
  cCmdInterpOther, // size 14
  cCmdInterpNoArg
};

static pInterpShort ShortInterpFuncs[NUM_SHORT_INTERP_FUNCS] =
{
  cCmdInterpShortMove,
  cCmdInterpShortAcquire,
  cCmdInterpShortRelease,
  cCmdInterpShortSubCall,
  cCmdInterpShortError,
  cCmdInterpShortError,
  cCmdInterpShortError,
  cCmdInterpShortError
};

ULONG TC_Size_Table[]= {
  0, // void
  SIZE_UBYTE,
  SIZE_SBYTE,
  SIZE_UWORD,
  SIZE_SWORD,
  SIZE_ULONG,
  SIZE_SLONG,
  SIZE_UWORD, // array
  0, // cluster
  SIZE_MUTEX,
  SIZE_FLOAT
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
  cCmdWrapFileClose, // 5
  cCmdWrapFileResolveHandle,
  cCmdWrapFileRename,
  cCmdWrapFileDelete,
  cCmdWrapSoundPlayFile,
  cCmdWrapSoundPlayTone, // 10
  cCmdWrapSoundGetState,
  cCmdWrapSoundSetState,
  cCmdWrapDrawText,
  cCmdWrapDrawPoint,
  cCmdWrapDrawLine, // 15
  cCmdWrapDrawCircle,
  cCmdWrapDrawRect,
  cCmdWrapDrawPicture,
  cCmdWrapSetScreenMode,
  cCmdWrapReadButton, // 20
  cCmdWrapCommLSWrite,
  cCmdWrapCommLSRead,
  cCmdWrapCommLSCheckStatus,
  cCmdWrapRandomNumber,
  cCmdWrapGetStartTick, // 25
  cCmdWrapMessageWrite,
  cCmdWrapMessageRead,
  cCmdWrapCommBTCheckStatus,
  cCmdWrapCommBTWrite,
  cCmdWrapCommBTRead, // 30
  cCmdWrapKeepAlive,
  cCmdWrapIOMapRead,
  cCmdWrapIOMapWrite,
  cCmdWrapColorSensorRead, // new in 2.0 
  cCmdWrapCommBTOnOff, // 35
  cCmdWrapCommBTConnection,
  cCmdWrapCommHSWrite,
  cCmdWrapCommHSRead,
  cCmdWrapCommHSCheckStatus,
  cCmdWrapReadSemData, //40
  cCmdWrapWriteSemData,
  cCmdWrapComputeCalibValue,
  cCmdWrapUpdateCalibCacheInfo,
  cCmdWrapDatalogWrite,
  cCmdWrapDatalogGetTimes,  //45
  cCmdWrapSetSleepTimeout,
  cCmdWrapListFiles, //47
  cCmdWrapUndefinedSysCall, // leave a gap so that I don't have to keep renumbering system calls
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, // 50
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, // 55
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall,
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, // 60
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, // 65
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall,
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, // 70
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, 
  cCmdWrapUndefinedSysCall, // 75
  cCmdWrapUndefinedSysCall, 
// enhanced NBC/NXC
  cCmdWrapInputPinFunction, // 77
  cCmdWrapIOMapReadByID,
  cCmdWrapIOMapWriteByID,
  cCmdWrapDisplayExecuteFunction, // 80
  cCmdWrapCommExecuteFunction,
  cCmdWrapLoaderExecuteFunction,
  cCmdWrapFileFindFirst,
  cCmdWrapFileFindNext,
  cCmdWrapFileOpenWriteLinear, // 85
  cCmdWrapFileOpenWriteNonLinear,
  cCmdWrapFileOpenReadLinear,
  cCmdWrapCommHSControl,
  cCmdWrapCommLSWriteEx,
  cCmdWrapFileSeek, // 90
  cCmdWrapFileResize,
  cCmdWrapDrawPictureArray,
  cCmdWrapDrawPolygon, 
  cCmdWrapDrawEllipse,
  cCmdWrapDrawFont, // 95
  cCmdWrapMemoryManager, 
  cCmdWrapReadLastResponse, 
  cCmdWrapFileTell,
  cCmdWrapRandomEx //  100 system call slots
    
  // don't forget to update SYSCALL_COUNT in c_cmd.h
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
  TC_UBYTE, //IO_OUT_OPTIONS
  TC_SBYTE, //IO_OUT_MAX_SPEED
  TC_SBYTE, //IO_OUT_MAX_ACCELERATION

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
  TC_UBYTE, //IO_OUT_OPTIONS
  TC_SBYTE, //IO_OUT_MAX_SPEED
  TC_SBYTE, //IO_OUT_MAX_ACCELERATION

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
  TC_UBYTE, //IO_OUT_OPTIONS
  TC_SBYTE, //IO_OUT_MAX_SPEED
  TC_SBYTE, //IO_OUT_MAX_ACCELERATION
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

// Data used to indicate usage of motor ports, or usage requests
UBYTE gUsageSemData, gRequestSemData;

UBYTE     cCmdBTGetDeviceType(UBYTE *pCOD)
{
  ULONG   COD;
  UBYTE   Result;
  UBYTE   Tmp;

  COD      = 0;
  for (Tmp = 0;Tmp < SIZE_OF_CLASS_OF_DEVICE;Tmp++)
  {
    COD  <<= 8;
    COD   |= (ULONG)*pCOD;
    pCOD++;
  }

  Result   = DEVICETYPE_UNKNOWN;
  if ((COD & 0x00001FFF) == 0x00000804)
  {
    Result = DEVICETYPE_NXT;
  }
  if ((COD & 0x00001F00) == 0x00000200)
  {
    Result = DEVICETYPE_PHONE;
  }
  if ((COD & 0x00001F00) == 0x00000100)
  {
    Result = DEVICETYPE_PC;
  }

  return (Result);
}

void cCmdSetVMState(VM_STATE newState)
{
  VarsCmd.VMState = newState;
}

UBYTE CMD_RESPONSE_LENGTH[256] = 
{
   3, // DCStartProgram (x00)
   3, // DCStopProgram (x01)
   3, // DCPlaySoundFile (x02)
   3, // DCPlayTone (x03)
   3, // DCSetOutputState (x04)
   3, // DCSetInputMode (x05)
  25, // DCGetOutputState (x06)
  16, // DCGetInputValues (x07)
   3, // DCResetInputScaledValue (x08)
   3, // DCMessageWrite (x09)
   3, // DCResetMotorPosition (x0a)
   5, // DCGetBatteryLevel (x0b)
   3, // DCStopSoundPlayback (x0c)
   7, // DCKeepAlive (x0d)
   4, // DCLSGetStatus (x0e)
   3, // DCLSWrite (x0f)
  20, // DCLSRead (x10)
  23, // DCGetCurrentProgramName (x11)
   0, // DCGetButtonState (not implemented) (x12)
  64, // DCMessageRead (x13)
   0, // DCRESERVED1 (x14)
   0, // DCRESERVED2 (x15)
   0, // DCRESERVED3 (x16)
   0, // DCRESERVED4 (x17)
   0, // DCRESERVED5 (x18)
  64, // DCDatalogRead (1.28+) (x19)
   3, // DCDatalogSetTimes (1.28+) (x1a)
   4, // DCBTGetContactCount (1.28+) (x1b)
  21, // DCBTGetContactName (1.28+) (x1c)
   4, // DCBTGetConnCount (1.28+) (x1d)
  21, // DCBTGetConnName (1.28+) (x1e)
   3, // DCSetProperty(1.28+) (x1f)
   7, // DCGetProperty (1.28+) (x20)
   3, // DCUpdateResetCount (1.28+) (x21)
   7, // RC_SET_VM_STATE (enhanced only) (x22)
   7, // RC_GET_VM_STATE (enhanced only) (x23)
  15, // RC_SET_BREAKPOINTS (enhanced only) (x24)
  15, // RC_GET_BREAKPOINTS (enhanced only) (x25)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (x26-x2f)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (x30-x3f)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (x40-x4f)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (x50-x5f)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (x60-x6f)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (x70-x7f)
   8, //   OPENREAD        = 0x80,
   4, //   OPENWRITE       = 0x81,
  64, //   READ            = 0x82, (actually is a variable length response)
   6, //   WRITE           = 0x83,
   4, //   CLOSE           = 0x84,
  23, //   DELETE          = 0x85,
  28, //   FINDFIRST       = 0x86,
  28, //   FINDNEXT        = 0x87,
   7, //   VERSIONS        = 0x88,
   4, //   OPENWRITELINEAR = 0x89,
   7, //   OPENREADLINEAR  = 0x8A, (not actually implemented)
   4, //   OPENWRITEDATA   = 0x8B,
   8, //   OPENAPPENDDATA  = 0x8C,
   4, //   CROPDATAFILE    = 0x8D,    /* New cmd for datalogging */
   0, //   XXXXXXXXXXXXXX  = 0x8E,
   0, //   XXXXXXXXXXXXXX  = 0x8F,
  34, //   FINDFIRSTMODULE = 0x90,
  34, //   FINDNEXTMODULE  = 0x91,
   4, //   CLOSEMODHANDLE  = 0x92,
   0, //   XXXXXXXXXXXXXX  = 0x93,
  64, //   IOMAPREAD       = 0x94, (actually is a variable length response)
   9, //   IOMAPWRITE      = 0x95,
   0, //   XXXXXXXXXXXXXX  = 0x96,
   7, //   BOOTCMD         = 0x97,  (can only be executed via USB)
   3, //   SETBRICKNAME    = 0x98,
   0, //   XXXXXXXXXXXXXX  = 0x99,
  10, //   BTGETADR        = 0x9A,
  33, //   DEVICEINFO      = 0x9B,
   0, //   XXXXXXXXXXXXXX  = 0x9C,
   0, //   XXXXXXXXXXXXXX  = 0x9D,
   0, //   XXXXXXXXXXXXXX  = 0x9E,
   0, //   XXXXXXXXXXXXXX  = 0x9F,
   3, //   DELETEUSERFLASH = 0xA0,
   5, //   POLLCMDLEN      = 0xA1,
  64, //   POLLCMD         = 0xA2,
  44, //   RENAMEFILE      = 0xA3,
   3, //   BTFACTORYRESET  = 0xA4,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (xA5-xAF)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (xB0-xBf)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (xC0-xCf)
   0, //   RESIZEDATAFILE  = 0xD0,
   0, //   SEEKFROMSTART   = 0xD1,
   0, //   SEEKFROMCURRENT = 0xD2,
   0, //   SEEKFROMEND     = 0xD3
   0, //   FILEPOSITION    = 0xD4
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (xD5-xDF)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // (xE0-xEF)
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // (xF0-xFF)
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
        if (LOADER_ERR(LStatus = pMapLoader->pFunc(FINDFIRST, (&pInBuf[2]), NULL, NULL)) != SUCCESS)
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
        UBYTE Port = pInBuf[1];
        //Don't do anything if illegal port specification is made
        // supported ports are 0, 1, 2 == A, B, C
        // 3 == AB, 4 == AC, 5 == BC, 6 == ABC
        if (Port > RC_OUT_ABC && Port < RC_PORTS_AB)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        //0xFF is protocol defined to mean "all ports".
        switch(Port) {
        case RC_PORTS_ALL:
        case RC_OUT_ABC:
        case RC_PORTS_AC:
        case RC_OUT_AC:
          {
            FirstPort = 0;
            LastPort = NO_OF_OUTPUTS - 1;
          }
          break;
        case RC_PORTS_BC:
        case RC_OUT_BC:
          {
            // B&C
            FirstPort = 1;
            LastPort = NO_OF_OUTPUTS - 1;
          }
          break;
        case RC_PORTS_AB:
        case RC_OUT_AB:
          {
            // A&B
            FirstPort = 0;
            LastPort  = 1;
          }
          break;
        default:
          {
            FirstPort = LastPort = Port;
          }
          break;
        }

        for (i = FirstPort; i <= LastPort; i++)
        {
          if (((Port == RC_PORTS_AC) || (Port == RC_OUT_AC)) && (i > FirstPort) && (i < LastPort))
            continue;
          OUTPUT * pOut = &(pMapOutPut->Outputs[i]);
          pOut->Speed             = pInBuf[2];
          pOut->Mode              = pInBuf[3];
          pOut->RegMode           = pInBuf[4];
          pOut->SyncTurnParameter = pInBuf[5];
          pOut->RunState          = pInBuf[6];
          pOut->Options           = pOut->Mode & REG_METHOD;
          memcpy((PSZ)(&(pOut->TachoLimit)), (PSZ)(&pInBuf[7]), 4);

          pOut->Flags |= UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT;
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
        INPUTSTRUCT * pIn = &(pMapInput->Inputs[i]);

        pIn->SensorType = pInBuf[2];
        pIn->SensorMode = pInBuf[3];
        //Set InvalidData flag automatically since type may have changed
        pIn->InvalidData = TRUE;
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
          OUTPUT * pOut = &(pMapOutPut->Outputs[i]);

          //Echo port
          pOutBuf[ResponseLen] = i;
          ResponseLen++;

          //Power
          pOutBuf[ResponseLen] = pOut->Speed;
          ResponseLen++;

          //Mode
          pOutBuf[ResponseLen] = pOut->Mode;
          ResponseLen++;

          //RegMode
          pOutBuf[ResponseLen] = pOut->RegMode;
          ResponseLen++;

          //TurnRatio
          pOutBuf[ResponseLen] = pOut->SyncTurnParameter;
          ResponseLen++;

          //RunState
          pOutBuf[ResponseLen] = pOut->RunState;
          ResponseLen++;

          //TachoLimit ULONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pOut->TachoLimit)), 4);
          ResponseLen += 4;

          //TachoCount SLONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pOut->TachoCnt)), 4);
          ResponseLen += 4;

          //BlockTachoCount SLONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pOut->BlockTachoCount)), 4);
          ResponseLen += 4;

          //RotationCount SLONG
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pOut->RotationCount)), 4);
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
          
          INPUTSTRUCT * pIn = &(pMapInput->Inputs[i]);

          //Set "Valid?" boolean
          if (pIn->InvalidData)
            pOutBuf[ResponseLen] = FALSE;
          else
            pOutBuf[ResponseLen] = TRUE;

          ResponseLen++;

          //Set "Calibrated?" boolean
          //!!! "Calibrated?" is a placeholder in the protocol.  Always FALSE for now.
          pOutBuf[ResponseLen] = FALSE;
          ResponseLen++;

          pOutBuf[ResponseLen] = pIn->SensorType;
          ResponseLen++;

          pOutBuf[ResponseLen] = pIn->SensorMode;
          ResponseLen++;

          //Set Raw, Normalized, and Scaled values
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pIn->ADRaw)), 2);
          ResponseLen += 2;

          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pIn->SensorRaw)), 2);
          ResponseLen += 2;

          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pIn->SensorValue)), 2);
          ResponseLen += 2;

          //!!! Return normalized raw value in place of calibrated value for now -- see comment above
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)(&(pIn->SensorRaw)), 2);
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
        pMapOutPut->Outputs[i].Flags |= (pInBuf[2] ? UPDATE_RESET_BLOCK_COUNT : UPDATE_RESET_ROTATION_COUNT);
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
        i = (pInBuf[1] & 0x03);
        UBYTE NoRestartOnRead = (pInBuf[1] & 0x04);
        UBYTE bFast = (pInBuf[1] & 0x08);
        Count = pInBuf[2];

        //Don't do anything if illegal port specification is made
        if (i >= NO_OF_LOWSPEED_COM_CHANNEL)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        RCStatus = cCmdLSWrite(i, Count, &(pInBuf[4]), pInBuf[3], NoRestartOnRead, bFast);
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

      // remote-only command to read from datalog buffer
      // pInBuf[1] = Remove? (bool)
      case RC_DATALOG_READ:
      {
#ifndef STRIPPED
        if (SendResponse == TRUE)
        {
          RCStatus = cCmdDatalogGetSize(&Count);
          pOutBuf[ResponseLen] = Count;
          ResponseLen++;

          if (!IS_ERR(RCStatus) && Count > 0)
          {
            pData = &(pOutBuf[ResponseLen]);
            RCStatus = cCmdDatalogRead(pData, Count, (pInBuf[1]));
            //If cCmdDatalogRead encountered an error, there is no real data in the buffer, so clear it out (below)
            if (IS_ERR(RCStatus))
              Count = 0;
            else
              ResponseLen += Count;
          }

          //Pad remaining data bytes with zeroes
          Count = MAX_DATALOG_SIZE - Count;
          memset(&(pOutBuf[ResponseLen]), 0, Count);
          ResponseLen += Count;
        }
#endif
      }
      break;
      case RC_DATALOG_SET_TIMES:
      {
#ifndef STRIPPED
        //SyncTime SLONG
        memcpy((PSZ)&IOMapCmd.SyncTime, (PSZ)&(pInBuf[1]), 4);
        IOMapCmd.SyncTick= dTimerReadNoPoll();
#endif
      }
      break;

      case RC_BT_GET_CONN_COUNT:
        if (SendResponse == TRUE) {
          pOutBuf[ResponseLen]= SIZE_OF_BT_CONNECT_TABLE;
          ResponseLen++;
          }
      break;
      case RC_BT_GET_CONN_NAME: // param in is index, param out is name
        if (SendResponse == TRUE) { // get index from inbuf
          i = pInBuf[1];
          if(i < SIZE_OF_BT_CONNECT_TABLE) { // unsigned, so guaranteed >= 0
            pOutBuf[ResponseLen] = cCmdBTGetDeviceType(pMapComm->BtConnectTable[i].ClassOfDevice);
            memcpy((PSZ)(&(pOutBuf[ResponseLen+1])), (PSZ)(pMapComm->BtConnectTable[i].Name), SIZE_OF_BT_NAME + 1);
            ResponseLen += SIZE_OF_BT_NAME + 2;
          }
          else {
            pOutBuf[ResponseLen] = 0;
            ResponseLen += SIZE_OF_BT_NAME + 2;
          }
        }
      break;
      case RC_BT_GET_CONTACT_COUNT:
        if (SendResponse == TRUE) {
          pOutBuf[ResponseLen]= SIZE_OF_BT_DEVICE_TABLE;
          ResponseLen++;
          }
      break;
      case RC_BT_GET_CONTACT_NAME:
        if (SendResponse == TRUE) { // get index from inbuf
          i = pInBuf[1];
          if(i < SIZE_OF_BT_DEVICE_TABLE && (pMapComm->BtDeviceTable[i].DeviceStatus & BT_DEVICE_KNOWN)) {  // unsigned, so guaranteed >= 0
            (pOutBuf[ResponseLen])= cCmdBTGetDeviceType(pMapComm->BtDeviceTable[i].ClassOfDevice);
            memcpy((PSZ)(&(pOutBuf[ResponseLen+1])), (PSZ)(pMapComm->BtDeviceTable[i].Name), SIZE_OF_BT_NAME + 1);
            ResponseLen += SIZE_OF_BT_NAME + 2;
          }
          else
          {
            pOutBuf[ResponseLen] = 0;
            memset((PSZ)(&(pOutBuf[ResponseLen+1])), 0, SIZE_OF_BT_NAME + 1);
            ResponseLen += SIZE_OF_BT_NAME + 2;
          }
        }
      break;
    case RC_SET_PROPERTY: // label/value pairs
        i = pInBuf[1];
        switch(i) {
          case RC_PROP_BTONOFF: {
            UWORD retVal, status;
            if(pInBuf[2])
              status= pMapComm->pFunc(BTON, 0, 0, 0, NULL, &retVal);
            else
              status= pMapComm->pFunc(BTOFF, 0, 0, 0, NULL, &retVal);

            RCStatus= (status == SUCCESS) ? retVal : status;
          }
          break;
          case RC_PROP_SOUND_LEVEL: {
            UBYTE volume= pInBuf[2];
            if(volume > 4)
              volume= 4;
            pMapSound->Volume= volume; // apparently stored in two places
            pMapUi->Volume= volume;
          }
          break;
          case RC_PROP_SLEEP_TIMEOUT: { // ulong millisecs to sleep
            ULONG value;
            memcpy((PSZ)&value, (PSZ)&(pInBuf[2]), 4);
            pMapUi->SleepTimeout= value / 60000;
          }
          break;
          case RC_PROP_DEBUGGING: { // ulong debug info
            ULONG value;
            memcpy((PSZ)&value, (PSZ)&(pInBuf[2]), 4);
            VarsCmd.Debugging  = (UBYTE)((value>>24)&0xFF);
            VarsCmd.PauseClump = (UBYTE)((value>>16)&0xFF);
            VarsCmd.PausePC    = (CODE_INDEX)(value&0xFFFF);
          }
          break;
          default:
            //Unknown property -- still inform client to not expect any response bytes
            NXT_BREAK;
            RCStatus = ERR_RC_UNKNOWN_CMD;
          break;
        }
      break;
    case RC_GET_PROPERTY: // label/value pairs
        if (SendResponse == TRUE) { // get index from inbuf
          i = pInBuf[1];
          switch(i) {
            case RC_PROP_BTONOFF:
              pOutBuf[ResponseLen]= pMapUi->BluetoothState != BT_STATE_OFF;
              ResponseLen++;
            break;
            case RC_PROP_SOUND_LEVEL: {
              pOutBuf[ResponseLen]= pMapSound->Volume;
              ResponseLen++;
              }
            break;
            case RC_PROP_SLEEP_TIMEOUT: {
              ULONG value= (pMapUi->SleepTimeout * 60000);
              memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)&value, 4);
              ResponseLen += 4;
            }
            break;
            case RC_PROP_DEBUGGING: { // ulong debug info
              ULONG value;
              value = ((VarsCmd.Debugging<<24)|(VarsCmd.PauseClump<<16)|VarsCmd.PausePC);
              memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)&value, 4);
              ResponseLen += 4;
            }
            break;
            default:
              //Unknown property -- still inform client to not expect any response bytes
              NXT_BREAK;
              RCStatus = ERR_RC_UNKNOWN_CMD;
            break;
          }
        }
      break;
    case RC_UPDATE_RESET_COUNT:
      {
        i = pInBuf[1];

        //Don't do anything if illegal port specification is made
        if (i >= NO_OF_OUTPUTS)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }

        pMapOutPut->Outputs[i].Flags |= UPDATE_RESET_COUNT;
      }
      break;
      case RC_SET_VM_STATE:
      {
        // don't change the VM state if the state is currently idle or resetting
        if (VarsCmd.VMState > VM_IDLE && VarsCmd.VMState < VM_RESET1) 
        {
          cCmdSetVMState((VM_STATE)pInBuf[1]);
          // setting the VM state turns on debugging
          VarsCmd.Debugging = TRUE;
          if (VarsCmd.VMState == VM_RESET1)
            IOMapCmd.ProgStatus = PROG_ABORT;
        }
        // fall through to RC_GET_VM_STATE
      }
      case RC_GET_VM_STATE:
      {
        if (SendResponse == TRUE)
        {
          // output the vm state, current clump and its relative program counter (4 bytes)
          pOutBuf[ResponseLen] = VarsCmd.VMState;
          ResponseLen++;
          pOutBuf[ResponseLen] = VarsCmd.RunQ.Head;
          ResponseLen++;
          CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[VarsCmd.RunQ.Head]);
          CODE_INDEX pc = (CODE_INDEX)(pClumpRec->PC-pClumpRec->CodeStart);
          memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)&(pc), 2);
          ResponseLen += 2;
        }
      }
      break;
      
      case RC_SET_BREAKPOINTS:
      {
        CLUMP_ID Clump = (CLUMP_ID)pInBuf[1];
        //Don't do anything if illegal clump specification is made
        if (Clump >= VarsCmd.AllClumpsCount)
        {
          RCStatus = ERR_RC_ILLEGAL_VAL;
          break;
        }
        // setting breakpoint information turns on debugging mode
        VarsCmd.Debugging = TRUE;
        CLUMP_BREAK_REC* pBreakpoints = VarsCmd.pAllClumps[Clump].Breakpoints;
        // length varies from 6 bytes min to 18 bytes max
        // clump byte, bpidx, bplocation (2 bytes), bp enabled, [...] terminal byte 0xFF 
        UBYTE idx = 2;
        UBYTE bDone = FALSE;
        while (!bDone) {
          UBYTE bpIdx = (UBYTE)pInBuf[idx];
          idx++;
          memcpy((PSZ)(&(pBreakpoints[bpIdx].Location)), (PSZ)(&pInBuf[idx]), 2);
          idx += 2;
          pBreakpoints[bpIdx].Enabled = (UBYTE)pInBuf[idx];
          idx++;
          bDone = (((UBYTE)pInBuf[idx] == 0xFF) || (idx >= 18));
        }
        // fall through to RC_GET_BREAKPOINTS
      }
      
      case RC_GET_BREAKPOINTS:
      {
        if (SendResponse == TRUE)
        {
          // output the list of breakpoints for the specified clump ID
          CLUMP_ID Clump = (CLUMP_ID)pInBuf[1];
          //Don't do anything if illegal clump specification is made
          if (Clump >= VarsCmd.AllClumpsCount)
          {
            RCStatus = ERR_RC_ILLEGAL_VAL;
            break;
          }
          CLUMP_BREAK_REC* pBreakpoints = VarsCmd.pAllClumps[Clump].Breakpoints;
          for(int j = 0; j < MAX_BREAKPOINTS; j++)
          {
            memcpy((PSZ)&(pOutBuf[ResponseLen]), (PSZ)&(pBreakpoints[j].Location), 2);
            ResponseLen += 2;
            pOutBuf[ResponseLen] = pBreakpoints[j].Enabled;
            ResponseLen++;
          }
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
      // fall through to the default case
//      break;

      default:
      {
        //Unhandled reply telegram.  Do nothing.
        //!!! Could/should stash unhandled/all replies somewhere so a syscall could read them
        VarsCmd.LastResponseLength = CMD_RESPONSE_LENGTH[pInBuf[0]];
        memset((PSZ)VarsCmd.LastResponseBuffer, 0, 64);
        UBYTE len = VarsCmd.LastResponseLength - 1;
        if (*pLen < len)
          len = *pLen;
        if (VarsCmd.LastResponseLength > 1)
          memcpy((PSZ)VarsCmd.LastResponseBuffer, (PSZ)(&pInBuf[0]), len);
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
    OUTPUT * pOut = &(pMapOutPut->Outputs[i]);
    IO_PTRS_OUT[IO_OUT_FLAGS            + i * IO_OUT_FPP] = (void*)&(pOut->Flags);
    IO_PTRS_OUT[IO_OUT_MODE             + i * IO_OUT_FPP] = (void*)&(pOut->Mode);
    IO_PTRS_OUT[IO_OUT_SPEED            + i * IO_OUT_FPP] = (void*)&(pOut->Speed);
    IO_PTRS_OUT[IO_OUT_ACTUAL_SPEED     + i * IO_OUT_FPP] = (void*)&(pOut->ActualSpeed);
    IO_PTRS_OUT[IO_OUT_TACH_COUNT       + i * IO_OUT_FPP] = (void*)&(pOut->TachoCnt);
    IO_PTRS_OUT[IO_OUT_TACH_LIMIT       + i * IO_OUT_FPP] = (void*)&(pOut->TachoLimit);
    IO_PTRS_OUT[IO_OUT_RUN_STATE        + i * IO_OUT_FPP] = (void*)&(pOut->RunState);
    IO_PTRS_OUT[IO_OUT_TURN_RATIO       + i * IO_OUT_FPP] = (void*)&(pOut->SyncTurnParameter);
    IO_PTRS_OUT[IO_OUT_REG_MODE         + i * IO_OUT_FPP] = (void*)&(pOut->RegMode);
    IO_PTRS_OUT[IO_OUT_OVERLOAD         + i * IO_OUT_FPP] = (void*)&(pOut->Overloaded);
    IO_PTRS_OUT[IO_OUT_REG_P_VAL        + i * IO_OUT_FPP] = (void*)&(pOut->RegPParameter);
    IO_PTRS_OUT[IO_OUT_REG_I_VAL        + i * IO_OUT_FPP] = (void*)&(pOut->RegIParameter);
    IO_PTRS_OUT[IO_OUT_REG_D_VAL        + i * IO_OUT_FPP] = (void*)&(pOut->RegDParameter);
    IO_PTRS_OUT[IO_OUT_BLOCK_TACH_COUNT + i * IO_OUT_FPP] = (void*)&(pOut->BlockTachoCount);
    IO_PTRS_OUT[IO_OUT_ROTATION_COUNT   + i * IO_OUT_FPP] = (void*)&(pOut->RotationCount);
    IO_PTRS_OUT[IO_OUT_OPTIONS          + i * IO_OUT_FPP] = (void*)&(pOut->Options);
    IO_PTRS_OUT[IO_OUT_MAX_SPEED        + i * IO_OUT_FPP] = (void*)&(pOut->MaxSpeed);
    IO_PTRS_OUT[IO_OUT_MAX_ACCELERATION + i * IO_OUT_FPP] = (void*)&(pOut->MaxAcceleration);
  }

  //Initialize IO_PTRS_IN
  for (i = 0; i < NO_OF_INPUTS; i++)
  {
    INPUTSTRUCT * pIn = &(pMapInput->Inputs[i]);
    IO_PTRS_IN[IO_IN_TYPE         + i * IO_IN_FPP] = (void*)&(pIn->SensorType);
    IO_PTRS_IN[IO_IN_MODE         + i * IO_IN_FPP] = (void*)&(pIn->SensorMode);
    IO_PTRS_IN[IO_IN_ADRAW        + i * IO_IN_FPP] = (void*)&(pIn->ADRaw);
    IO_PTRS_IN[IO_IN_NORMRAW      + i * IO_IN_FPP] = (void*)&(pIn->SensorRaw);
    IO_PTRS_IN[IO_IN_SCALEDVAL    + i * IO_IN_FPP] = (void*)&(pIn->SensorValue);
    IO_PTRS_IN[IO_IN_INVALID_DATA + i * IO_IN_FPP] = (void*)&(pIn->InvalidData);
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

  cCmdSetVMState(VM_IDLE);

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
  IOMapCmd.SyncTime= 0;
  IOMapCmd.SyncTick= 0;

  return;
}


void cCmdCtrl(void)
{
  NXT_STATUS Status = NO_ERR;

  switch (VarsCmd.VMState)
  {
    case VM_RUN_FREE:
    case VM_RUN_SINGLE:
    {
 #if VMProfilingCode
    ULONG EnterTime= dTimerReadHiRes(), FinishTime;
    CmdCtrlCalls ++;
#endif
    ULONG Continue;

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
      if (IOMapCmd.DeactivateFlag == TRUE || 
          ((pMapButton->State[BTN1] & pMapUi->AbortFlag) && 
           ((pMapButton->State[BTN4] & PRESSED_EV) != PRESSED_EV))) // JCH 2010-01-13 Make sure enter button is not also pressed
      {
        IOMapCmd.DeactivateFlag = FALSE;

        //Clear pressed event so it doesn't get double-counted by UI
        pMapButton->State[BTN1] &= ~(pMapUi->AbortFlag);

        //Go to VM_RESET1 state and report abort
        cCmdSetVMState(VM_RESET1);
        IOMapCmd.ProgStatus = PROG_ABORT;
        break;
      }

      //Assert that we have an active program
      NXT_ASSERT(VarsCmd.ActiveProgHandle != NOT_A_HANDLE);

      //Handle any resting clumps that are ready to awaken
      cCmdCheckRestQ(IOMapCmd.Tick); // not using result, yet
      //Execute from at least one clump
      do
      {
        //Execute instructions from a clump up to INSTR_MAX, to end of millisec,
        //Finishing/suspending a clump, BREAKOUT_REQ, or any errors will cause a return
#if VMProfilingCode
          ULONG ClumpEnterTime= dTimerReadHiRes();
          CLUMP_ID clump= VarsCmd.RunQ.Head;
#endif
          Status = cCmdInterpFromClump();
#if VMProfilingCode
          CmdCtrlClumpTime[clump] += dTimerReadHiRes() - ClumpEnterTime;
#endif
        // automatically switch from RUN_SINGLE to RUN_PAUSE after a single step  
        if (VarsCmd.VMState == VM_RUN_SINGLE)
          cCmdSetVMState(VM_RUN_PAUSE);

        //If RunQ and RestQ are empty, program is done, or wacko
        if (!cCmdIsClumpIDSane(VarsCmd.RunQ.Head)) {
          Continue = FALSE;
          if(!cCmdIsClumpIDSane(VarsCmd.RestQ.Head)) {
            cCmdSetVMState(VM_RESET1);
            IOMapCmd.ProgStatus = PROG_OK;
          }
        }
        else if (Status == CLUMP_SUSPEND || Status == CLUMP_DONE)
          Continue = TRUE; // queue isn't empty, didn't timeout
          //Only rotate RunQ on a "normal" finish, i.e. no error, clump end, or breakout request
        else if (Status == ROTATE_QUEUE) { // done and suspend do their own
          cCmdRotateQ();
          Continue= TRUE;
        }
       else if (Status == TIMES_UP) {
          cCmdRotateQ();
          Continue = FALSE;
        }
        else if (IS_ERR(Status)) // mem error is handled in InterpFromClump if possible
        {
          Continue = FALSE;
          cCmdSetVMState(VM_RESET1);
          IOMapCmd.ProgStatus = Status;
        }
        else if (Status == STOP_REQ)
        {
          Continue = FALSE;
          cCmdSetVMState(VM_RESET1);
          IOMapCmd.ProgStatus = PROG_OK;
        }
        else if (Status == BREAKOUT_REQ)
        {
          Continue = FALSE;
        }
      } while (Continue == TRUE && VarsCmd.VMState == VM_RUN_FREE);
#if VMProfilingCode
      FinishTime= dTimerReadHiRes();
      if(NotFirstCall)
        OverheadTime += EnterTime - LeaveTime;
      else
        NotFirstCall= 1;
      CmdCtrlTime += FinishTime - EnterTime;
      LeaveTime= FinishTime;
#endif
      // May busy wait to postpone to 1ms schedule
      while (IOMapCmd.Tick == dTimerRead());
    }
    break;
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
          IOMapCmd.ProgStatus = Status;
          cCmdSetVMState(VM_RESET1);
        }
        //Else start running program
        else
        {
          cCmdSetVMState(VM_RUN_FREE);
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
    while (IOMapCmd.Tick == dTimerRead()); // delay until scheduled time
    }
    break;

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

      cCmdSetVMState(VM_RESET2);
      while (IOMapCmd.Tick == dTimerRead()); // delay until scheduled time
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
        cCmdSetVMState(VM_IDLE);
        IOMapCmd.ProgStatus = PROG_IDLE;
      }
    while (IOMapCmd.Tick == dTimerRead()); // delay until scheduled time
    }
      break;
    
    case VM_RUN_PAUSE:
    {
      while (IOMapCmd.Tick == dTimerRead()); // delay until scheduled time
    }
    break;
  }//END state machine switch

  //Set tick to new value for next time 'round
  IOMapCmd.Tick = dTimerReadNoPoll();

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

  if (strncmp((PSZ)pData, "NXTBINARY", VM_FORMAT_STRING_SIZE) == 0)
  {
    ULONG NativeOffset;
    pCursor = (pData + 12);
    NativeOffset = (ULONG)(*pCursor);
    void (*native)(ULONG, ULONG) = (void (*)())(pData + NativeOffset);
    (*native)((ULONG)pData, DataSize);
    NXT_BREAK;
    return (ERR_VER);
  }
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
    return (ERR_CLUMP_COUNT);
  else
    VarsCmd.AllClumpsCount = (CLUMP_ID)FileClumpCount;

  VarsCmd.CodespaceCount = *((UWORD*)(pData + CurrOffset));
  CurrOffset += 2;

  //Can't have a valid program with no code
  if (VarsCmd.CodespaceCount == 0)
    return (ERR_NO_CODE);

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
    return (ERR_INSANE_OFFSET);
  }

  //
  // Finally, update VarsCmd fields
  //

  VarsCmd.RunQ.Head = NOT_A_CLUMP;
  VarsCmd.RunQ.Tail = NOT_A_CLUMP;
  VarsCmd.RestQ.Head = NOT_A_CLUMP;
  VarsCmd.RestQ.Tail  = NOT_A_CLUMP;

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
    return (ERR_BAD_POOL_SIZE);
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

  if (TypeCode > TC_LAST_VALID)
    return ERR_INSTR;
  else if (TypeCode == TC_CLUSTER)
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

void cCmdRefreshActiveClump(CLUMP_ID CurrID)
{
  CLUMP_REC *  clumpRecPtr= &(VarsCmd.pAllClumps[CurrID]);

  if(clumpRecPtr->clumpScalarDispatchHints & scalarBinopDispatchMask)
    InterpFuncs[8]= cCmdInterpScalarBinop;
  else
    InterpFuncs[8]= cCmdInterpBinop;
  if(clumpRecPtr->clumpScalarDispatchHints & scalarUnop2DispatchMask)
    InterpFuncs[6]= cCmdInterpScalarUnop2;
  else
    InterpFuncs[6]= cCmdInterpUnop2;
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
    return (ERR_LOADER_ERR);

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
    return ERR_SPOTCHECK_FAIL;
  }

  //Initialize CLUMP_RECs as contiguous list in RAM
  pCursor = (pData + FileOffsets.Clumps);
  for (i = 0; i < VarsCmd.AllClumpsCount; i++)
  {
    CLUMP_REC *clumpPtr= &VarsCmd.pAllClumps[i];
    clumpPtr->InitFireCount  = *(UBYTE*)(pCursor + i * VM_FILE_CLUMP_REC_SIZE);
    clumpPtr->DependentCount = *(UBYTE*)(pCursor + (i * VM_FILE_CLUMP_REC_SIZE) + 1);
    clumpPtr->CodeStart      = *(UWORD*)(pCursor + (i * VM_FILE_CLUMP_REC_SIZE) + 2) + VarsCmd.pCodespace;

    //Initialize remaining CLUMP_REC fields
    clumpPtr->PC = clumpPtr->CodeStart;
    clumpPtr->Link = NOT_A_CLUMP;
    clumpPtr->Priority = INSTR_MAX_COUNT;
    clumpPtr->CalledClump = NOT_A_CLUMP;
    
    CLUMP_BREAK_REC* pBreakpoints = clumpPtr->Breakpoints;
    for (j = 0; j < MAX_BREAKPOINTS; j++)
    {
      pBreakpoints[j].Location = 0;
      pBreakpoints[j].Enabled = FALSE;
    }

    //Activate any clumps with CurrFireCount of 0
    clumpPtr->CurrFireCount = clumpPtr->InitFireCount;
    if (clumpPtr->CurrFireCount == 0)
      cCmdEnQClump(&(VarsCmd.RunQ), (CLUMP_ID)i);
  }

  //Patch up dependents in separate pass (reuse of pCursor)
  pCursor += VarsCmd.AllClumpsCount * VM_FILE_CLUMP_REC_SIZE;
  for (i = 0; i < VarsCmd.AllClumpsCount; i++)
  {
    CLUMP_REC *clumpPtr= &VarsCmd.pAllClumps[i];
    if (clumpPtr->DependentCount > 0)
    {
      clumpPtr->pDependents = (CLUMP_ID*)(pCursor);

      pCursor += (clumpPtr->DependentCount * sizeof(CLUMP_ID));
    }
    else
      clumpPtr->pDependents = NULL;

    //Patch up CodeEnd value based on CodeStart of next clump or last overall codeword
    if (i < (VarsCmd.AllClumpsCount - 1))
      clumpPtr->CodeEnd = (clumpPtr+1)->CodeStart - 1;
    else
      clumpPtr->CodeEnd = VarsCmd.CodespaceCount - 1 + VarsCmd.pCodespace;

    //Test for empty/insane clump code definitions
    NXT_ASSERT(clumpPtr->CodeStart < clumpPtr->CodeEnd);
  }

  // Check if the instructions within a clump are polymorphic and mark which table to dispatch from
  for (i = 0; i < VarsCmd.AllClumpsCount; i++)
  { // Check type on Boolean, math, ArrInit and ArrIndex, ingore GetSet I/O as these are always scalar
    // do we need to check for DataArg encodings to I/O map??? GM
    // Get Opcode and size of each instr, if ^^, check Arg types for Array or Cluster
    CLUMP_REC *clumpPtr= &VarsCmd.pAllClumps[i];
    CODE_WORD *pInstr = clumpPtr->CodeStart, *lastPC = clumpPtr->CodeEnd;
    ULONG InstrSize, opCode, shortOp, isT2Agg, isT3Agg, isScalarBinop= TRUE, isScalarUnop2= TRUE;
    TYPE_CODE t1, t2, t3;
    ULONG instrWord;
   do
    {
      instrWord= *(UWORD*)pInstr;
      opCode= OP_CODE(pInstr);
      shortOp= (instrWord>>8) & 0x0F;
      InstrSize = INSTR_SIZE(instrWord);
      if (InstrSize == VAR_INSTR_SIZE)
        InstrSize = ((UWORD*)pInstr)[1];
      if(shortOp <= 7) // no shorts are binOps
      {
        t2= cCmdDSType(pInstr[2]);
        isT2Agg= IS_AGGREGATE_TYPE(t2);
        if(InstrSize == 8) {
        t3= cCmdDSType(pInstr[3]);
        isT3Agg= IS_AGGREGATE_TYPE(t3);
          if(isT2Agg || isT3Agg) {
            if(opCode == OP_CMP) {
              UBYTE isString2, isString3;
              isString2= (t2  == TC_ARRAY) && cCmdDSType(INC_ID(pInstr[2])) == TC_UBYTE;
              isString3= (t3  == TC_ARRAY) && cCmdDSType(INC_ID(pInstr[3])) == TC_UBYTE;
              t1= cCmdDSType(pInstr[1]);
              if((!isString2 || !isString3) || t1 == TC_ARRAY) // allow strings to go scalar, don't let through element compares of bytes or Bools
                isScalarBinop= FALSE;
            }
            else if(opCode == OP_BRCMP || opCode == OP_BRCMPABSVAR)
              isScalarBinop= FALSE;
          }
        }
        else if(InstrSize == 6 && isT2Agg && (opCode == OP_NOT || opCode == OP_BRTST || opCode == OP_BRTSTABSVAR))
          isScalarUnop2= FALSE;
      }
      pInstr += InstrSize/2;
    } while((isScalarBinop || isScalarUnop2) && pInstr < lastPC);
    if(isScalarBinop)
      clumpPtr->clumpScalarDispatchHints |= scalarBinopDispatchMask;
    else
      clumpPtr->clumpScalarDispatchHints &= ~scalarBinopDispatchMask;

    if(isScalarUnop2)
      clumpPtr->clumpScalarDispatchHints |= scalarUnop2DispatchMask;
    else
      clumpPtr->clumpScalarDispatchHints &= ~scalarUnop2DispatchMask;

  }
  //Programs with no active clumps constitutes an activation error
  if (VarsCmd.RunQ.Head == NOT_A_CLUMP)
    return (ERR_NO_ACTIVE_CLUMP);
  else
  {
    // now that we know which clumps are scalar and poly, refresh dispatch table to match head
    cCmdRefreshActiveClump(VarsCmd.RunQ.Head);

  }

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
    return (ERR_DEFAULT_OFFSETS);
  }

  //Copy Dynamic defaults from file
  memmove(VarsCmd.pDataspace + VarsCmd.DSStaticSize, pData + FileOffsets.DSDefaults + FileOffsets.DynamicDefaults, FileOffsets.DynamicDefaultsSize);

  // fix memmgr links. old files contain unused backPtrs, we now use these to store backLink
  DV_INDEX prev= NOT_A_DS_ID;
  for (i = VarsCmd.MemMgr.Head; i != NOT_A_DS_ID; i = DV_ARRAY[i].Link) {
    DV_ARRAY[i].BackLink= prev;
    prev= i;
  }

  //Verify the MemMgr ended up where we said it would
  if ((UBYTE *)VarsCmd.MemMgr.pDopeVectorArray != VarsCmd.pDataspace + DV_ARRAY[0].Offset)
  {
    NXT_BREAK;
    return (ERR_MEMMGR_FAIL);
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

#ifndef STRIPPED
  //Initialize datalog queue
  VarsCmd.DatalogBuffer.ReadIndex = 0;
  VarsCmd.DatalogBuffer.WriteIndex = 0;
  for (j = 0; j < DATALOG_QUEUE_DEPTH; j++)
  {
    VarsCmd.DatalogBuffer.Datalogs[j] = NOT_A_DS_ID;
  }
#endif

  // now that we've loaded program, prime memmgr dopevectors based upon number of handles in ds.
  ULONG numHandles= DV_ARRAY[0].Count/2;
  if(numHandles > 200)
    numHandles= 200;
  Status = cCmdGrowDopeVectorArray(numHandles);

  if (cCmdVerifyMemMgr() != TRUE)
    return (ERR_FILE);

  gUsageSemData= 0;
  gRequestSemData= 0;
  // preload all calibration coefficients into mem
  cCmdLoadCalibrationFiles();
  // initialize the graphic globals
  gpPassedImgVars = NULL;  
  memset(gpImgData,0,sizeof(gpImgData));
  gPassedVarsCount = 0;
  // configure debugging flags in VarsCmd
  VarsCmd.Debugging  = FALSE;
  VarsCmd.PauseClump = NOT_A_CLUMP;
  VarsCmd.PausePC    = 0xFFFF;
  return (Status);
}


void cCmdDeactivateProgram()
{
  UBYTE i, tmp;

  // reset the DS and DVA Offsets
  IOMapCmd.OffsetDVA = 0xFFFF;
  IOMapCmd.OffsetDS = 0xFFFF;
  
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
      pMapLoader->pFunc(CROPDATAFILE, &tmp, NULL, NULL); /*CLOSE*/
  }

  //Clear FileHandleTable
  memset(VarsCmd.FileHandleTable, 0, sizeof(VarsCmd.FileHandleTable));
  
  // reset AbortFlag to default value
  pMapUi->AbortFlag = PRESSED_EV;
  
  // reset Contrast to default value
  pMapDisplay->Contrast = DISPLAY_CONTRAST_DEFAULT;
  
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
    INPUTSTRUCT * pIn = &(pMapInput->Inputs[i]); 
    //Clear type and mode to defaults
    pIn->SensorType = NO_SENSOR;
    pIn->SensorMode = RAWMODE;

    //Reset input values to 0 prior to running (clear things like stale rotation counts)
    pIn->ADRaw       = 0;
    pIn->SensorRaw   = 0;
    pIn->SensorValue = 0;

    //Assert invalid data flag so future code is aware of these changes
    pIn->InvalidData = TRUE;
  }

  for (i = 0; i < NO_OF_OUTPUTS; i++)
  {
    //Coast and reset all motor parameters
    OUTPUT * pOut = &(pMapOutPut->Outputs[i]);
    pOut->Mode = 0;
    pOut->RegMode = REGULATION_MODE_IDLE;
    pOut->RunState = MOTOR_RUN_STATE_IDLE;
    pOut->Speed = 0;
    pOut->TachoLimit = 0;
    pOut->SyncTurnParameter = 0;
    pOut->Flags = UPDATE_MODE | UPDATE_SPEED | UPDATE_TACHO_LIMIT | UPDATE_RESET_COUNT | UPDATE_RESET_BLOCK_COUNT | UPDATE_RESET_ROTATION_COUNT;
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
    if(Queue == &(VarsCmd.RunQ))
      cCmdRefreshActiveClump(NewClump);
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
    else if(Queue == &(VarsCmd.RunQ))
      cCmdRefreshActiveClump(Queue->Head);
  }
  //Else, look through rest of list looking for a link to our clump
  else
  {
    do
    {
      CLUMP_REC *clumpPtr= &VarsCmd.pAllClumps[CurrID];
      LinkID = clumpPtr->Link;

      //If we find a link to our clump, patch up predecessor's link
      if (clumpPtr->Link == Clump)
      {
        clumpPtr->Link = VarsCmd.pAllClumps[Clump].Link;
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
void cCmdRotateQ()
{
  CLUMP_ID CurrID;
  CLUMP_REC * pClumpRec;
  CLUMP_Q * Queue = &VarsCmd.RunQ;

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

    // reinit clump info
    CurrID= Queue->Head;
    cCmdRefreshActiveClump(Queue->Head);

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

NXT_STATUS cCmdAcquireMutex(MUTEX_Q * Mutex)
{
  NXT_STATUS Status = NO_ERR;
  CLUMP_ID Clump= VarsCmd.RunQ.Head; // save off before queue changes below

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

UBYTE cCmdIsClumpOnAMutexWaitQ(CLUMP_ID Clump)
{
  //Make sure Clump's ID is valid
  NXT_ASSERT(cCmdIsClumpIDSane(Clump));
  DATA_ARG Arg1;
  MUTEX_Q * Mutex;
  for (Arg1=0; Arg1 < VarsCmd.DataspaceCount; Arg1++)
  {
    if (VarsCmd.pDataspaceTOC[Arg1].TypeCode == TC_MUTEX)
    {
      Mutex = cCmdDSPtr(Arg1, 0);
      if (cCmdIsClumpOnQ(&(Mutex->WaitQ), Clump))
        return TRUE;
    }
  }
  return FALSE;
}

NXT_STATUS cCmdReleaseAllMutexes(CLUMP_ID Clump)
{
  //Make sure Clump's ID is valid
  NXT_ASSERT(cCmdIsClumpIDSane(Clump));
  DATA_ARG Arg1;
  MUTEX_Q * Mutex;
  UBYTE bFoundWaitingMutex = FALSE;
  for (Arg1=0; Arg1 < VarsCmd.DataspaceCount; Arg1++)
  {
    if (VarsCmd.pDataspaceTOC[Arg1].TypeCode == TC_MUTEX)
    {
      Mutex = cCmdDSPtr(Arg1, 0);
      // if this clump owns the Mutex then release it
      if (Mutex->Owner == Clump)
        cCmdReleaseMutex(Mutex);
      // also make sure that this Clump is not waiting in this mutex's wait queue
      if (!bFoundWaitingMutex && cCmdIsClumpOnQ(&(Mutex->WaitQ), Clump)) {
        bFoundWaitingMutex = TRUE;
        cCmdDeQClump(&(Mutex->WaitQ), Clump);
        cCmdEnQClump(&(VarsCmd.RunQ), Clump);
      }
    }
  }
  return (NO_ERR);
}

NXT_STATUS cCmdReleaseMutex(MUTEX_Q * Mutex)
{
#if WIN_DEBUG || defined(ARM_DEBUG)
  CLUMP_ID Clump= VarsCmd.RunQ.Head;
#endif
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

NXT_STATUS cCmdStopClump(CLUMP_ID Clump)
{
  // first check whether this clump has called another clump
  CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[Clump]);
  if (pClumpRec->CalledClump != NOT_A_CLUMP) {
    // in this situation we know that this clump
    // is not on any queues of any kind (run, rest, or wait)
    // so instead of trying to stop THIS clump we will
    // try to stop the clump it called instead
    cCmdStopClump(pClumpRec->CalledClump);
  }
  else
  {
    // release any mutexes owned by this clump
    // and remove it from any wait queues that it might be on
    cCmdReleaseAllMutexes(Clump);
    if (cCmdIsClumpOnQ(&(VarsCmd.RunQ), Clump)) {
        // remove the specified clump from the run queue if it is on it
        cCmdDeQClump(&(VarsCmd.RunQ), Clump);
    }
    else if (cCmdIsClumpOnQ(&(VarsCmd.RestQ), Clump)) {
        // if the specified clump happened to be sleeping then
        // remove it from the rest queue
        cCmdDeQClump(&(VarsCmd.RestQ), Clump);
    }
    // since we have stopped that clump we should reset its clump rec values.
    pClumpRec->PC = pClumpRec->CodeStart;
    pClumpRec->CurrFireCount = pClumpRec->InitFireCount;
    pClumpRec->awakenTime = 0;
  }
  return (NO_ERR);
}

// No instruction to do this yet, but put current clump to sleep until awakeTime occurs
NXT_STATUS cCmdSleepClump(ULONG time)
{
  CLUMP_ID Clump= VarsCmd.RunQ.Head; // save off before queue changes below
  CLUMP_REC * pClump = &(VarsCmd.pAllClumps[Clump]);
  cCmdDeQClump(&(VarsCmd.RunQ), Clump);
  cCmdEnQClump(&(VarsCmd.RestQ), Clump);
  pClump->awakenTime= time;
  return CLUMP_SUSPEND;
}

UBYTE cCmdCheckRestQ(ULONG currTime)
{
  UBYTE awakened= FALSE;
  CLUMP_ID curr, next;
  CLUMP_REC * pClump;
  curr= VarsCmd.RestQ.Head;
  while(curr != NOT_A_CLUMP) {
    pClump= &(VarsCmd.pAllClumps[curr]);
    next= pClump->Link;
    if(pClump->awakenTime <= currTime) {
      pClump->awakenTime= 0; // not necessary, but for debugging identification
      cCmdDeQClump(&(VarsCmd.RestQ), curr);
      cCmdEnQClump(&(VarsCmd.RunQ), curr);
      awakened= TRUE;
    }
    curr= next;
  }
  return awakened;
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

  CLUMP_REC *clumpPtr= &VarsCmd.pAllClumps[TargetClump];
  clumpPtr->CurrFireCount--;
  if (clumpPtr->CurrFireCount == 0)
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
  ULONG *poolPtr;

  //VarsCmd.Pool is a UBYTE pointer to ULONG array
  //This was done to enforce portable alignment.
  VarsCmd.Pool = (UBYTE*)(IOMapCmd.MemoryPool);

  for (i = (POOL_MAX_SIZE / 4), poolPtr= (ULONG*)&(POOL_START)[0]; i>0; i--, poolPtr++)
    *poolPtr = 0xDEADBEEF;

  VarsCmd.PoolSize = 0;
}


#if VMProfilingCode
ULONG memMgrTime= 0;
#endif
NXT_STATUS cCmdDSArrayAlloc(DS_ELEMENT_ID DSElementID, UWORD Offset, UWORD NewCount)
{
  NXT_STATUS Status = NO_ERR;
  UWORD DVIndex;
  UWORD OldCount;
  UWORD i;
#if VMProfilingCode
  ULONG enterTime= dTimerReadHiRes();
#endif
  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));

  //Only arrays are valid here
  //!!! Recommended to upgrade NXT_ASSERT to ERR_INSTR return
  NXT_ASSERT(cCmdDSType(DSElementID) == TC_ARRAY);

  DVIndex = cCmdGetDVIndex(DSElementID, Offset);
  OldCount = DV_ARRAY[DVIndex].Count;

  if(OldCount == NewCount)
    goto allocExit;
  Status = cCmdDVArrayAlloc(DVIndex, NewCount);

  if (Status < NO_ERR)
    goto allocExit;

  if(!IS_AGGREGATE_TYPE(cCmdDSType(INC_ID(DSElementID))))
    goto allocExit;

  if (OldCount > NewCount)
  {
    //Free dope vectors for sub-arrays.
    for (i = NewCount; i < OldCount; i++)
    {
      Status = cCmdFreeSubArrayDopeVectors(INC_ID(DSElementID), ARRAY_ELEM_OFFSET(DVIndex, i));
      if (IS_ERR(Status))
        goto allocExit;
    }
  }
  else if (OldCount < NewCount)
  {
    //Alloc dope vectors for sub-arrays. Set up DVIndexes
    for (i = OldCount; i < NewCount;  i++)
    {
      Status = cCmdAllocSubArrayDopeVectors(INC_ID(DSElementID), ARRAY_ELEM_OFFSET(DVIndex, i));
      if (IS_ERR(Status))
        goto allocExit;
    }
  }

  NXT_ASSERT(cCmdVerifyMemMgr());
allocExit:
#if VMProfilingCode
  memMgrTime += dTimerReadHiRes() - enterTime;
#endif
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

    //Move old Array Data to new allocation
    if(OldCount)
      memmove(pData, VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset, (UWORD)(DV_ARRAY[DVIndex].ElemSize * OldCount));
    //!!! Clear mem so old mem doesn't contain stale data. Not strictly needed.
#if WIN_DEBUG || defined(ARM_DEBUG)
    memset(VarsCmd.pDataspace + DV_ARRAY[DVIndex].Offset, 0xFF, (UWORD)(DV_ARRAY[DVIndex].ElemSize * OldCount));
#endif
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
      Status = cCmdAllocDopeVector(&DVIndex, ElemSize);
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


NXT_STATUS cCmdAllocDopeVector(DV_INDEX *pIndex, UWORD ElemSize)
{
  NXT_STATUS Status = NO_ERR;

  if (VarsCmd.MemMgr.FreeHead == NOT_A_DS_ID)
  {
    //No free DVs. Need to grow DopeVector array.
    Status = cCmdGrowDopeVectorArray(DV_ARRAY_GROWTH_COUNT);
    if (IS_ERR(Status))
      return Status;
  }

  if(VarsCmd.MemMgr.FreeHead == NOT_A_DS_ID)
    return ERR_MEM;

  //Remove DV from free list
  *pIndex = VarsCmd.MemMgr.FreeHead;
  VarsCmd.MemMgr.FreeHead = DV_ARRAY[*pIndex].Link;
  if(VarsCmd.MemMgr.FreeHead != NOT_A_DS_ID)
    DV_ARRAY[VarsCmd.MemMgr.FreeHead].BackLink= NOT_A_DS_ID;
  //Add DV to tail of MemMgr list
  Status = cCmdMemMgrInsertAtTail(*pIndex);

  //Initialize values
  DV_ARRAY[*pIndex].Offset = NOT_AN_OFFSET;
  DV_ARRAY[*pIndex].ElemSize = ElemSize;
  DV_ARRAY[*pIndex].Count = 0;

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
  DV_INDEX prev, post;

  //Bounds check
  NXT_ASSERT(DVIndex < DV_ARRAY[0].Count);

  //Reset dope vector fields
  DV_ARRAY[DVIndex].Count = 0;
  DV_ARRAY[DVIndex].ElemSize = 0;
  DV_ARRAY[DVIndex].Offset = NOT_AN_OFFSET;

  //Remove from MemMgr list
  if (DVIndex == VarsCmd.MemMgr.Head)
  {
    VarsCmd.MemMgr.Head = DV_ARRAY[DVIndex].Link;
    if(VarsCmd.MemMgr.Head != NOT_A_DS_ID)
      DV_ARRAY[VarsCmd.MemMgr.Head].BackLink= NOT_A_DS_ID;
  }
  else
  {
    // patchup middle or end of list.
    prev= DV_ARRAY[DVIndex].BackLink;
    post= DV_ARRAY[DVIndex].Link;
    NXT_ASSERT(prev != NOT_A_DS_ID);

    DV_ARRAY[prev].Link = post;
    if(post != NOT_A_DS_ID)
      DV_ARRAY[post].BackLink= prev;
    if (DVIndex == VarsCmd.MemMgr.Tail)
      VarsCmd.MemMgr.Tail = prev;
    //Make sure we found the previous DV, otherwise this DV was not in the the list (already freed?)
  }

  //Push onto free list
  DV_ARRAY[DVIndex].Link = VarsCmd.MemMgr.FreeHead;
  DV_ARRAY[DVIndex].BackLink = NOT_A_DS_ID;
  DV_ARRAY[VarsCmd.MemMgr.FreeHead].BackLink= DVIndex;
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
    DV_ARRAY[i].BackLink = NOT_A_DS_ID;
    if(VarsCmd.MemMgr.FreeHead != NOT_A_DS_ID)
      DV_ARRAY[VarsCmd.MemMgr.FreeHead].BackLink = i;
    DV_ARRAY[i].Link = VarsCmd.MemMgr.FreeHead;
    VarsCmd.MemMgr.FreeHead = i;
  }

  //Move dope vector to end of MemMgr list
  Status = cCmdMemMgrMoveToTail(0);

  NXT_ASSERT(cCmdVerifyMemMgr());

  return Status;
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
  DV_INDEX prev, post;

  //Bounds check
  NXT_ASSERT(DVIndex < DV_ARRAY[0].Count);

  //Short circut if its already at the tail
  if (DVIndex == VarsCmd.MemMgr.Tail)
    return NO_ERR;

  if (DVIndex == VarsCmd.MemMgr.Head) {
    VarsCmd.MemMgr.Head = DV_ARRAY[DVIndex].Link;
    DV_ARRAY[VarsCmd.MemMgr.Head].BackLink= NOT_A_DS_ID;
  }
  else
  {
    // connect to middle or end of list.
    prev= DV_ARRAY[DVIndex].BackLink;
    post= DV_ARRAY[DVIndex].Link;
    NXT_ASSERT(prev != NOT_A_DS_ID);
    DV_ARRAY[prev].Link = post;
    if(post != NOT_A_DS_ID)
      DV_ARRAY[post].BackLink= prev;
  }

  DV_ARRAY[DVIndex].Link = NOT_A_DS_ID;
  DV_ARRAY[DVIndex].BackLink = VarsCmd.MemMgr.Tail;
  if(VarsCmd.MemMgr.Tail != NOT_A_DS_ID)
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
  DV_ARRAY[DVIndex].BackLink= VarsCmd.MemMgr.Tail;
  DV_ARRAY[DVIndex].Link = NOT_A_DS_ID;
  VarsCmd.MemMgr.Tail = DVIndex;

  NXT_ASSERT(cCmdVerifyMemMgr());

  return NO_ERR;
}


UBYTE cCmdVerifyMemMgr()
{
  DV_INDEX i, prev, post;
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

    prev= DV_ARRAY[i].BackLink;
    post= DV_ARRAY[i].Link;
    if (post == NOT_A_DS_ID && i != VarsCmd.MemMgr.Tail)
      return FALSE;
    else if(prev == NOT_A_DS_ID && i != VarsCmd.MemMgr.Head)
      return FALSE;
    else if(prev != NOT_A_DS_ID && DV_ARRAY[prev].Link != i)
      return FALSE;
    else if(post != NOT_A_DS_ID && DV_ARRAY[post].BackLink != i)
      return FALSE;

    DVCount++;
  }

  // could check link and backlinks too
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
    Status = cCmdAllocDopeVector(&GET_WRITE_MSG(QueueID), 1);
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
// Datalog Queue function(s)
//

NXT_STATUS cCmdDatalogWrite(UBYTE * pData, UWORD Length)
{
  NXT_STATUS Status = NO_ERR;

#ifndef STRIPPED
  if (pData == NULL)
    return ERR_ARG;

  if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
    return (ERR_NO_PROG);

  //Can't accept oversize messages because we treat them as strings (truncation would remove null termination)
  if (Length > MAX_DATALOG_SIZE)
    return ERR_INVALID_SIZE;

  if (IS_DV_INDEX_SANE(GET_WRITE_DTLG()))
  {
    //A message is already there, the queue is full
    NXT_ASSERT(VarsCmd.DatalogBuffer.WriteIndex == VarsCmd.DatalogBuffer.ReadIndex);
    Status = STAT_MSG_BUFFERWRAP;
    //Bump read index, drop existing message to make room for our newly acquired datalog
    VarsCmd.DatalogBuffer.ReadIndex = (VarsCmd.DatalogBuffer.ReadIndex + 1) % DATALOG_QUEUE_DEPTH;
  }
  else
  {
    //Allocate dope vector for message
    Status = cCmdAllocDopeVector(&GET_WRITE_DTLG(), 1);
    if (IS_ERR(Status))
      return Status;
  }

  //Allocate storage for message
  Status |= cCmdDVArrayAlloc(GET_WRITE_DTLG(), Length);
  if (IS_ERR(Status))
  {
    //Clear the dope vector for the message, since we're unable to put a message there.
    cCmdFreeDopeVector(GET_WRITE_DTLG());
    SET_WRITE_DTLG(NOT_A_DS_ID);
    return Status;
  }

  //Copy message
  memmove(cCmdDVPtr(GET_WRITE_DTLG()), pData, Length);

  //Advance write index
  VarsCmd.DatalogBuffer.WriteIndex = (VarsCmd.DatalogBuffer.WriteIndex + 1) % DATALOG_QUEUE_DEPTH;

#endif
  return Status;
}

NXT_STATUS cCmdDatalogGetSize(UWORD * Size)
{
#ifndef STRIPPED
  DV_INDEX ReadDVIndex;

  if (Size == NULL)
    return (ERR_ARG);

  if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
  {
    *Size = 0;
    return (ERR_NO_PROG);
  }

  ReadDVIndex = GET_READ_DTLG();

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
#else
  *Size = 0;
  return (NO_ERR);
#endif
}

NXT_STATUS cCmdDatalogRead(UBYTE * pBuffer, UWORD Length, UBYTE Remove)
{
  NXT_STATUS Status = NO_ERR;
#ifndef STRIPPED
  DV_INDEX ReadDVIndex;

  if (pBuffer == NULL)
    return (ERR_ARG);

  if (VarsCmd.ActiveProgHandle == NOT_A_HANDLE)
    return (ERR_NO_PROG);

  ReadDVIndex = GET_READ_DTLG();

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

      SET_READ_DTLG(NOT_A_DS_ID);

      //Advance read index
      VarsCmd.DatalogBuffer.ReadIndex = (VarsCmd.DatalogBuffer.ReadIndex + 1) % DATALOG_QUEUE_DEPTH;
    }
  }
  else
  {
    //No message to read, datalog Queue is empty
    NXT_ASSERT(VarsCmd.DatalogBuffer.ReadIndex == VarsCmd.DatalogBuffer.WriteIndex);

    return (STAT_MSG_EMPTY_MAILBOX);
  }
#endif
  return Status;
}


//
// Color Sensor Functions
//
NXT_STATUS cCmdColorSensorRead (UBYTE Port, SWORD * SensorValue, UWORD * RawArray, UWORD * NormalizedArray,
								SWORD * ScaledArray, UBYTE * InvalidData)
{
  ULONG i;
	//Make sure Port is valid for Color Sensor
        INPUTSTRUCT * pIn = &(pMapInput->Inputs[Port]);
	UBYTE sType = pIn->SensorType;
	if (!(sType == COLORFULL || sType == COLORRED || sType == COLORGREEN || 
              sType == COLORBLUE || sType == COLORNONE))
	{
		return (ERR_COMM_CHAN_NOT_READY); //TODO - is this the right error?
	}
	//Copy Detected Color
	*SensorValue = pIn->SensorValue;

	//Copy all raw, normalized and scaled data from I/O Map
	for (i=0; i<NO_OF_COLORS; i++){
		COLORSTRUCT * pColor = &(pMapInput->Colors[Port]);
		RawArray[i]        = pColor->ADRaw[i];
		NormalizedArray[i] = pColor->SensorRaw[i];
		ScaledArray[i]     = pColor->SensorValue[i];
	}
	//Copy the Invalid Data Flag
	*InvalidData = pIn->InvalidData;

	return NO_ERR;

}


//
// Dataspace Support functions
//

UBYTE cCmdIsDSElementIDSane(DS_ELEMENT_ID Index)
{
  if ((Index & DATA_ARG_IMM_MASK) < VarsCmd.DataspaceCount)
    return TRUE;
  else
    return FALSE;
}

void * cCmdResolveDataArg(DATA_ARG DataArg, UWORD Offset, TYPE_CODE * TypeCode)
{
  void * ret_val = NULL;

  //!!! DATA_ARG masking system only for internal c_cmd use!
  //    All normal bytecode arguments should go through top if() block.

    NXT_ASSERT(cCmdIsDSElementIDSane(DataArg));
    ret_val = cCmdDSPtr(DataArg, Offset);
    if (TypeCode)
      *TypeCode = VarsCmd.pDataspaceTOC[DataArg].TypeCode;

  //!!! Caller beware! If DataArg isn't sane, ret_val may be out of range or NULL!
  return ret_val;
}

// normal Resolve handles both, but this is specific to I/O args
void * cCmdResolveIODataArg(DATA_ARG DataArg, ULONG Offset, TYPE_CODE * TypeCode)
  {
  void * ret_val = NULL;

  ULONG ModuleID;
  ULONG FieldID;
    //DataArg refers to a field in the IO map
  // ModuleID = ((DataArg >> 9) & 0x1F);
  ModuleID = ((DataArg & 0x3FFF) >> 9);
  FieldID = (DataArg & 0x01FF);

    //!!! Preliminary bounds check -- still could allow invalid combos through
    if (ModuleID > MOD_OUTPUT || FieldID >= IO_OUT_FIELD_COUNT)
    {
      NXT_BREAK;
      return NULL;
    }

    ret_val = IO_PTRS[ModuleID][FieldID];
    if (TypeCode)
      *TypeCode = IO_TYPES[ModuleID][FieldID];
  return ret_val;
}

void cCmdSetValFlt(void * pVal, TYPE_CODE TypeCode, float NewVal)
{

  if (pVal)
  {
    switch (TypeCode)
    {
      case TC_ULONG:
      {
        *(ULONG*)pVal = (ULONG)NewVal;
      }
      break;

      case TC_SLONG:
      {
        *(SLONG*)pVal = (SLONG)NewVal;
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

      case TC_FLOAT:
      {
        *(float*)pVal = (float)NewVal;
      }
      break;
    }
  }

  return;
}

ULONG cCmdGetUByte(void * pVal);
ULONG cCmdGetSByte(void * pVal);
ULONG cCmdGetUWord(void * pVal);
ULONG cCmdGetSWord(void * pVal);
ULONG cCmdGetULong(void * pVal);
ULONG cCmdGetSLong(void * pVal);
ULONG cCmdGetError(void * pVal);
ULONG cCmdGetFloat(void * pVal);

void cCmdSetByte(void * pVal, ULONG NewVal);
void cCmdSetWord(void * pVal, ULONG NewVal);
void cCmdSetLong(void * pVal, ULONG NewVal);
void cCmdSetError(void * pVal, ULONG NewVal);
void cCmdSetFloat(void * pVal, ULONG NewVal);


typedef ULONG (*pGetOperand)(void *);
static pGetOperand GetProcArray[11]= {cCmdGetUByte, cCmdGetUByte, cCmdGetSByte, cCmdGetUWord, cCmdGetSWord, cCmdGetULong, cCmdGetSLong, cCmdGetError, cCmdGetError, cCmdGetError, cCmdGetFloat}; // dup UByte to line up

typedef void (*pSetOperand)(void *, ULONG);
static pSetOperand SetProcArray[11]= {cCmdSetByte, cCmdSetByte, cCmdSetByte, cCmdSetWord, cCmdSetWord, cCmdSetLong, cCmdSetLong, cCmdSetError, cCmdSetError, cCmdSetError, cCmdSetFloat}; // dup UByte to line up

void cCmdSetError(void * pVal, ULONG NewVal) {
  NXT_BREAK;
}

void cCmdSetLong(void * pVal, ULONG NewVal) {
  *(ULONG*)pVal = NewVal;
}

void cCmdSetWord(void * pVal, ULONG NewVal) {
  *(UWORD*)pVal = (UWORD)NewVal;
}

void cCmdSetByte(void * pVal, ULONG NewVal) {
  *(UBYTE*)pVal = (UBYTE)NewVal;
}

void cCmdSetFloat(void * pVal, ULONG NewVal) {
  *(float*)pVal = (float)NewVal;
}

// only works on simple types, equivalent to resolve and get, but faster
ULONG cCmdGetScalarValFromDataArg(DATA_ARG DataArg, UWORD Offset)
{
  DS_TOC_ENTRY *dsTOCPtr= &VarsCmd.pDataspaceTOC[DataArg];
  return GetProcArray[dsTOCPtr->TypeCode](VarsCmd.pDataspace + dsTOCPtr->DSOffset + Offset);
}

float cCmdGetFloatValFromDataArg(DATA_ARG DataArg, UWORD Offset)
{
  DS_TOC_ENTRY *dsTOCPtr= &VarsCmd.pDataspaceTOC[DataArg];
  return (float)(*(float*)(VarsCmd.pDataspace + dsTOCPtr->DSOffset + Offset));
}

ULONG cCmdGetError(void * pVal) {
  NXT_BREAK;
  return 0;
}

ULONG cCmdGetULong(void * pVal) {
  return (ULONG)(*(ULONG*)pVal);
}

ULONG cCmdGetSLong(void * pVal) {
  return (SLONG)(*(SLONG*)pVal);
}

ULONG cCmdGetUWord(void * pVal) {
  return (UWORD)(*(UWORD*)pVal);
}

ULONG cCmdGetSWord(void * pVal) {
  return (SWORD)(*(SWORD*)pVal);
}

ULONG cCmdGetUByte(void * pVal) {
  return (UBYTE)(*(UBYTE*)pVal);
}

ULONG cCmdGetSByte(void * pVal) {
  return (SBYTE)(*(SBYTE*)pVal);
}

ULONG cCmdGetFloat(void * pVal) {
  float tempVal = *(float*)pVal;
  if (tempVal >= (float)0.0) {
    tempVal += (float)0.5;
  }
  else {
    tempVal -= (float)0.5;
  }
  return (ULONG)tempVal;
}

ULONG cCmdGetVal(void * pVal, TYPE_CODE TypeCode)
{
  if (pVal)
  return GetProcArray[TypeCode](pVal);
  else
  //!!! Default return value places responsibility on caller to use this function wisely
  return 0;
}


float cCmdGetValFlt(void * pVal, TYPE_CODE TypeCode)
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

      case TC_FLOAT:
      {
        return (float)(*(float*)pVal);
      }

      default:
      break;
    }
  }

  //!!! Default return value places responsibility on caller to use this function wisely
  return 0;
}



// Only for scalar types and no offset
void cCmdSetScalarValFromDataArg(DATA_ARG DataArg, ULONG NewVal)
{
  DS_TOC_ENTRY *dsTOCPtr= &VarsCmd.pDataspaceTOC[DataArg];
  SetProcArray[dsTOCPtr->TypeCode](VarsCmd.pDataspace + dsTOCPtr->DSOffset, NewVal);
}

void cCmdSetVal(void * pVal, TYPE_CODE TypeCode, ULONG NewVal)
{
  if (pVal)
    SetProcArray[TypeCode](pVal, NewVal);
}

void* cCmdDSPtr(DS_ELEMENT_ID DSElementID, UWORD Offset)
{
  void * pDSItem;
  DV_INDEX DVIndex;
  TYPE_CODE TypeCode;
  UBYTE bPointer = (DSElementID & 0x8000) != 0;

  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));
  
  DSElementID &= DATA_ARG_IMM_MASK;
  // pointers are only valid if the type is UWORD

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
  {
    if (bPointer && (TypeCode == TC_UWORD))
    {
      pDSItem = (VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[DSElementID].DSOffset + Offset);
      DSElementID = cCmdGetVal(pDSItem, TypeCode);
      pDSItem = cCmdDSPtr(DSElementID, Offset);
    }
    else
      pDSItem = (VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[DSElementID].DSOffset + Offset);
  }
  
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

UWORD cCmdArrayDimensions(DS_ELEMENT_ID DSElementID)
{
  NXT_ASSERT(cCmdIsDSElementIDSane(DSElementID));
  UWORD result = 0;
  while (cCmdDSType(DSElementID) == TC_ARRAY) 
  {
    result++;
    DSElementID = INC_ID(DSElementID);
  }
  return result;
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


UBYTE cCmdCompare(UBYTE CompCode, ULONG Val1, ULONG Val2, TYPE_CODE TypeCode1, TYPE_CODE TypeCode2)
{
  SLONG SVal1, SVal2;
  if (QUICK_UNSIGNED_TEST(TypeCode1) || QUICK_UNSIGNED_TEST(TypeCode2))
  {
    return ((CompCode == OPCC1_LT   && Val1 <  Val2)
         || (CompCode == OPCC1_GT   && Val1 >  Val2)
         || (CompCode == OPCC1_LTEQ && Val1 <= Val2)
         || (CompCode == OPCC1_GTEQ && Val1 >= Val2)
         || (CompCode == OPCC1_EQ   && Val1 == Val2)
         || (CompCode == OPCC1_NEQ  && Val1 != Val2));
  }
  else
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
}

UBYTE cCmdCompareFlt(UBYTE CompCode, float Val1, float Val2, TYPE_CODE TypeCode1, TYPE_CODE TypeCode2)
{
  //!!! add threshold to equality comparisons
  return ((CompCode == OPCC1_LT   && Val1 <  Val2)
       || (CompCode == OPCC1_GT   && Val1 >  Val2)
       || (CompCode == OPCC1_LTEQ && Val1 <= Val2)
       || (CompCode == OPCC1_GTEQ && Val1 >= Val2)
       || (CompCode == OPCC1_EQ   && Val1 == Val2)
       || (CompCode == OPCC1_NEQ  && Val1 != Val2));
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
    ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, Offset2);
    ArgVal3 = cCmdGetScalarValFromDataArg(Arg3, Offset3);

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

ULONG gClearProfileInfo= 0, bigExecTime= 0;
#if VMProfilingCode
void UpdateProfileInfo(ULONG shortOp, CODE_WORD *pInstr, ULONG execTime, ULONG InstrSize)
{
  ULONG j;
  ULONG opCode;

  if(execTime > 500 && shortOp == 8)
    bigExecTime= shortOp;
  if(gClearProfileInfo) {
    ExecutedInstrs= 0;
    CmdCtrlTime= 0;
    OverheadTime= 0;
    CmdCtrlCalls= 0;
    LastAvgCount= 0;
    for(j= 0; j < 255; j++)
      CmdCtrlClumpTime[j]= 0;
    for(j= 0; j < OPCODE_COUNT; j++) {
      InstrProfile[j].Avg= 0;
      InstrProfile[j].Time= 0;
      InstrProfile[j].Count= 0;
      InstrProfile[j].Max= 0;
    }
    for(j= 0; j < SYSCALL_COUNT; j++) {
      SysCallProfile[j].Avg= 0;
      SysCallProfile[j].Time= 0;
      SysCallProfile[j].Count= 0;
      SysCallProfile[j].Max= 0;
    }
    for(j= 0; j < NUM_SHORT_OPCODE_COUNT; j++) {
      ShortInstrProfile[j].Avg= 0;
      ShortInstrProfile[j].Time= 0;
      ShortInstrProfile[j].Count= 0;
      ShortInstrProfile[j].Max= 0;
    }
    for(j= 0; j < NUM_INTERP_FUNCS; j++) {
      InterpFuncProfile[j].Avg= 0;
      InterpFuncProfile[j].Time= 0;
      InterpFuncProfile[j].Count= 0;
      InterpFuncProfile[j].Max= 0;
    }
    gClearProfileInfo= FALSE;
  }
  ExecutedInstrs ++;
  if(shortOp > 7) // shortop bit set
  {
    ShortInstrProfile[shortOp-8].Time += execTime;
    ShortInstrProfile[shortOp-8].Count++;
    if(execTime > ShortInstrProfile[shortOp-8].Max)
      ShortInstrProfile[shortOp-8].Max= execTime;
  }
  else
  {
    opCode = OP_CODE(pInstr);
    InstrProfile[opCode].Time += execTime;
    InstrProfile[opCode].Count++;
    if(execTime > InstrProfile[opCode].Max)
      InstrProfile[opCode].Max= execTime;
    if(opCode == OP_SYSCALL)
    {
      SysCallProfile[GetDataArg(pInstr[1])].Time += execTime;
      SysCallProfile[GetDataArg(pInstr[1])].Count++;
      if(execTime > SysCallProfile[GetDataArg(pInstr[1])].Max)
        SysCallProfile[GetDataArg(pInstr[1])].Max= execTime;
    }

    InterpFuncProfile[InstrSize].Time += execTime;
    InterpFuncProfile[InstrSize].Count++;
    if(execTime > InterpFuncProfile[InstrSize].Max)
      InterpFuncProfile[InstrSize].Max= execTime;
  }
  if(ExecutedInstrs - LastAvgCount > 999) // every N instrs, update avgs
  {
    for(j= 0; j < OPCODE_COUNT; j++)
      if(InstrProfile[j].Count)
        InstrProfile[j].Avg= InstrProfile[j].Time/InstrProfile[j].Count;
    for(j= 0; j < SYSCALL_COUNT; j++)
      if(SysCallProfile[j].Count)
        SysCallProfile[j].Avg= SysCallProfile[j].Time/SysCallProfile[j].Count;
    for(j= 0; j < NUM_SHORT_OPCODE_COUNT; j++)
      if(ShortInstrProfile[j].Count)
        ShortInstrProfile[j].Avg= ShortInstrProfile[j].Time/ShortInstrProfile[j].Count;
    for(j= 0; j < NUM_INTERP_FUNCS; j++)
      if(InterpFuncProfile[j].Count)
        InterpFuncProfile[j].Avg= InterpFuncProfile[j].Time/InterpFuncProfile[j].Count;
    LastAvgCount= ExecutedInstrs;
  }
}
#endif


//
// Interpreter Functions
//

NXT_STATUS cCmdInterpFromClump()
{
  CLUMP_ID Clump= VarsCmd.RunQ.Head;
  NXT_STATUS Status = NO_ERR;
  CLUMP_REC * pClumpRec;
  CODE_WORD * pInstr, *lastClumpInstr;
  UBYTE InstrSize;
  ULONG shortOp, nextMSTick;
  SLONG i;
#if VM_BENCHMARK
  ULONG InstrTime = dTimerRead();
#endif

  if (!cCmdIsClumpIDSane(Clump)) // this means all clumps are asleep
    return TIMES_UP;

  //Resolve clump record structure and current instruction pointer
  pClumpRec = &(VarsCmd.pAllClumps[Clump]);
  pInstr = pClumpRec->PC; // abs
  lastClumpInstr= pClumpRec->CodeEnd; // abs

  if(VarsCmd.VMState == VM_RUN_FREE)
    i = pClumpRec->Priority;
  else
    i = 1;
  nextMSTick= dTimerGetNextMSTickCnt();
  do
  {
    // are we debugging and are free running and reach a breakpoint/autopause?
    if (VarsCmd.Debugging && (VarsCmd.VMState == VM_RUN_FREE))
    {
      CLUMP_BREAK_REC* pBreakpoints = pClumpRec->Breakpoints;
      for(int j = 0; j < MAX_BREAKPOINTS; j++)
      {
        if (pBreakpoints[j].Enabled && 
            (pBreakpoints[j].Location == (CODE_INDEX)(pClumpRec->PC-pClumpRec->CodeStart)))
        {
          cCmdSetVMState(VM_RUN_PAUSE);
          return BREAKOUT_REQ;
        }
      }
      // auto pause at clump == pauseClump and relative PC = pausePC
      if ((Clump == VarsCmd.PauseClump) && 
          ((CODE_INDEX)(pClumpRec->PC-pClumpRec->CodeStart) == VarsCmd.PausePC))
      {
        // pause the VM
        cCmdSetVMState(VM_RUN_PAUSE);
        // and turn off the auto pause flags
        VarsCmd.PauseClump = NOT_A_CLUMP;
        VarsCmd.PausePC    = 0xFFFF;
        return BREAKOUT_REQ;
      }
    }

#if VMProfilingCode
    ULONG instrStartTime;
    instrStartTime= dTimerReadHiRes();
#endif

    ULONG instrWord= *(UWORD*)pInstr;
    shortOp= (instrWord>>8) & 0x0F;
    if(shortOp > 7) // shortop bit set
      Status= ShortInterpFuncs[shortOp - 8](pInstr);
    else
    { // we know this is a long instr, dispatch on num params, which correlates to size
      InstrSize = INSTR_SIZE(instrWord); // keep in a local for profiling
      Status = (*InterpFuncs[InstrSize])(pInstr);
    }

#if VMProfilingCode
    UpdateProfileInfo(shortOp, pInstr, dTimerReadHiRes() - instrStartTime, InstrSize);
#endif

afterCompaction:
    if (Status == NO_ERR)
      pInstr += gPCDelta;
    else if (Status == CLUMP_DONE) // already requeued
    {
      pClumpRec->PC = pClumpRec->CodeStart;
      pClumpRec->CurrFireCount = pClumpRec->InitFireCount;
      return Status;
    }
    else if (Status == CLUMP_SUSPEND || Status == BREAKOUT_REQ || Status == ROTATE_QUEUE)  // already requeued
    {
      pClumpRec->PC = pInstr + gPCDelta;
    //Throw error if we ever advance beyond the clump's codespace
    if (pInstr > lastClumpInstr)
      {
        NXT_BREAK;
        Status = ERR_INSTR;
      }
      return Status;
    }
    else if (Status == ERR_MEM)
    {
    //Memory is full. Compact dataspace and try the instruction again.
    //!!! Could compact DopeVectorArray here
    cCmdDSCompact();
      if(shortOp > 7) // shortop bit set
        Status= ShortInterpFuncs[shortOp - 8](pInstr);
      else
        Status = (*InterpFuncs[InstrSize])(pInstr);
	  if(Status == ERR_MEM)
        return Status;
    else
        goto afterCompaction;
    }
    else // other errors, breakout, stop
      return Status;

    //Throw error if we ever advance beyond the clump's codespace
    if (pInstr > lastClumpInstr)
    {
      NXT_BREAK;
      Status = ERR_INSTR;
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
    VarsCmd.InstrCount++;
#endif

    //Count one more instruction for this pass
    if ((SLONG)(nextMSTick - dTimerReadTicks()) <= 0) // HWTimer has passed ms tick limit
      Status= TIMES_UP;
    else if(--i <= 0)
      Status= ROTATE_QUEUE;
  } while (!Status);
  pClumpRec->PC= pInstr;
  return (Status);
}


NXT_STATUS cCmdInterpUnop1(CODE_WORD * const pCode)
{
  CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[VarsCmd.RunQ.Head]);
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1;

  NXT_ASSERT(pCode != NULL);

  gPCDelta= 2;
  opCode = OP_CODE(pCode);
  Arg1   = pCode[1];

  switch (opCode)
  {
    case OP_JMP:
    {
      gPCDelta= (SWORD)Arg1;
      Status = NO_ERR;
    }
    break;

    case OP_JMPABSVAR:
    {
      CODE_INDEX pc = (CODE_INDEX)(pClumpRec->PC-pClumpRec->CodeStart);
      gPCDelta = (SWORD)Arg1-(SWORD)pc;
      Status = NO_ERR;
    }
    break;

    case OP_ACQUIRE:
    {
      NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
      NXT_ASSERT(VarsCmd.pDataspaceTOC[Arg1].TypeCode == TC_MUTEX);

      Status = cCmdAcquireMutex((MUTEX_Q *)cCmdDSScalarPtr(Arg1, 0));
    }
    break;

    case OP_RELEASE:
    {
      NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
      NXT_ASSERT(VarsCmd.pDataspaceTOC[Arg1].TypeCode == TC_MUTEX);

      Status = cCmdReleaseMutex((MUTEX_Q *)cCmdDSScalarPtr(Arg1, 0));
    }
    break;

    case OP_SUBRET:
    {
      NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
      CLUMP_ID clump = *((CLUMP_ID *)cCmdDSScalarPtr(Arg1, 0));
      
      //Take Subroutine off RunQ
      //Add Subroutine's caller to RunQ
      cCmdDeQClump(&(VarsCmd.RunQ), VarsCmd.RunQ.Head);
      cCmdEnQClump(&(VarsCmd.RunQ), clump);

      CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[clump]);
      pClumpRec->CalledClump = NOT_A_CLUMP;
      
      Status = CLUMP_DONE;
    }
    break;

    case OP_FINCLUMPIMMED:
    case OP_FINCLUMPVAR:
    {
        CLUMP_ID Clump= VarsCmd.RunQ.Head; // DeQ changes Head, use local val
        cCmdDeQClump(&(VarsCmd.RunQ), Clump); //Dequeue finalized clump
        if (opCode == OP_FINCLUMPVAR)
        {
          // indirect clump reference
          if (cCmdDSType(Arg1) <= TC_LAST_INT_SCALAR)
            Arg1 = cCmdGetScalarValFromDataArg(Arg1, 0);
          else
            return (ERR_INSTR);
        }
        cCmdSchedDependent(Clump, (CLUMP_ID)Arg1);  // Use immediate form

        Status = CLUMP_DONE;
    }
    break;

    case OP_WAITI:
    case OP_WAITV:
    {
      ULONG wait= 0;
      if (opCode == OP_WAITV) {
        wait = cCmdGetScalarValFromDataArg(Arg1, 0);
      }
      else
        wait = Arg1;
      if(wait == 0)
        Status= ROTATE_QUEUE;
      else
        Status = cCmdSleepClump(wait + IOMapCmd.Tick); // put to sleep, to wake up wait ms in future
    }
    break;

    case OP_GETTICK:
    {
      cCmdSetScalarValFromDataArg(Arg1, dTimerReadNoPoll());
    }
    break;

    case OP_STOP:
    {
      //Unwired Arg1 means always stop
      if (Arg1 == NOT_A_DS_ID)
        Status = STOP_REQ;
      else if (cCmdGetScalarValFromDataArg(Arg1, 0) > 0)
        Status = STOP_REQ;
    }
    break;

    case OP_STOPCLUMPIMMED:
    case OP_STOPCLUMPVAR:
    {
        if (opCode == OP_STOPCLUMPVAR)
        {
          // indirect clump reference
          if (cCmdDSType(Arg1) <= TC_LAST_INT_SCALAR)
            Arg1 = cCmdGetScalarValFromDataArg(Arg1, 0);
          else
            return (ERR_INSTR);
        }
        // Release any mutexes that the clump we are stopping owns
        cCmdStopClump((CLUMP_ID)Arg1);
    }
    break;
    
    case OP_STARTCLUMPIMMED:
    case OP_STARTCLUMPVAR:
    {
        if (opCode == OP_STARTCLUMPVAR)
        {
          // indirect clump reference
          if (cCmdDSType(Arg1) <= TC_LAST_INT_SCALAR)
            Arg1 = cCmdGetScalarValFromDataArg(Arg1, 0);
          else
            return (ERR_INSTR);
        }
        CLUMP_ID Clump = (CLUMP_ID)Arg1;
        // only enqueue the clump if it is not already on one of the queues
        // otherwise this is a no-op
        if (!cCmdIsClumpOnQ(&(VarsCmd.RunQ), Clump) && 
            !cCmdIsClumpOnQ(&(VarsCmd.RestQ), Clump) &&
            !cCmdIsClumpOnAMutexWaitQ(Clump)) 
        {
            cCmdEnQClump(&(VarsCmd.RunQ), Clump); //Enqueue the specified clump
            Status = CLUMP_SUSPEND;
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

ULONG scalarNots= 0, scalarBrtst= 0, scalarUn2Other= 0, scalarUn2Dispatch= 0, polyUn2Dispatch= 0;
NXT_STATUS cCmdInterpScalarUnop2(CODE_WORD * const pCode)
{
  CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[VarsCmd.RunQ.Head]);
  NXT_STATUS Status;
  UBYTE opCode;

  NXT_ASSERT(pCode != NULL);
  opCode = OP_CODE(pCode);
  DATA_ARG Arg1, Arg2;

  scalarUn2Dispatch ++;
  if(opCode == OP_NOT) // t2 && t3 guaranteed scalar
  {
    gPCDelta= 3;
    Arg1 = pCode[1];
    Arg2 = pCode[2];
    ULONG ArgVal1, ArgVal2;

    ArgVal2= cCmdGetScalarValFromDataArg(Arg2, 0);
    //!!! OP_NOT is logical, *not* bit-wise.
    //This differs from the other logical ops because we don't distinguish booleans from UBYTEs.
    ArgVal1=  (!ArgVal2);
    cCmdSetScalarValFromDataArg(Arg1, ArgVal1);
    Status = NO_ERR;
    scalarNots ++;
  }
  else if(opCode == OP_BRTST || opCode == OP_BRTSTABSVAR)
  {
    ULONG Branch, compare= COMP_CODE(pCode);
    ULONG TypeCode;

    Arg1 = pCode[1];
    Arg2 = pCode[2];
    TypeCode = cCmdDSType(Arg2);

    if(Arg2 == NOT_A_DS_ID)
    {
      Branch= ((compare == OPCC1_EQ)
              || (compare == OPCC1_LTEQ)
              || (compare == OPCC1_GTEQ));
    }
    else
    {
      if(compare == OPCC1_EQ && TypeCode == TC_UBYTE) // very common for loops
      {
        UBYTE *pBRVal = (VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[Arg2].DSOffset);
        Branch= *pBRVal == 0;
      }
      else
      {
        SLONG SVal1 = (SLONG)cCmdGetScalarValFromDataArg(Arg2, 0);
        Branch= ((compare == OPCC1_EQ   && SVal1 == 0)
              || (compare == OPCC1_NEQ  && SVal1 != 0)
              || (compare == OPCC1_GT   && SVal1 >  0)
              || (compare == OPCC1_LT   && SVal1 <  0)
              || (compare == OPCC1_LTEQ && SVal1 <= 0)
              || (compare == OPCC1_GTEQ && SVal1 >= 0));
      }
    }
    if (Branch) {
      if (opCode == OP_BRTST)
        gPCDelta =  (SWORD)Arg1;
      else
        gPCDelta =  (UWORD)Arg1 - (pClumpRec->PC-pClumpRec->CodeStart);
    }
    else
      gPCDelta= 3;
    Status = NO_ERR;
    scalarBrtst ++;
  }
  else {
    Status= cCmdInterpUnop2(pCode);
    scalarUn2Other ++;
  }
  return Status;
}

NXT_STATUS cCmdInterpUnop2(CODE_WORD * const pCode)
{
  CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[VarsCmd.RunQ.Head]);
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1;
  DATA_ARG Arg2;
  void *pArg1 = NULL, *pArg2 = NULL;
  TYPE_CODE TypeCode1, TypeCode2;

  ULONG i;
  UWORD ArgC;
  static UBYTE * ArgV[MAX_CALL_ARGS + 1];

  polyUn2Dispatch ++;
  UWORD Count;
  UWORD Offset;
//  SLONG TmpSLong;
//  ULONG TmpULong;
  ULONG ArgVal2;
  float FltArgVal2;

  NXT_ASSERT(pCode != NULL);

  gPCDelta= 3;
  opCode = OP_CODE(pCode);
  Arg1   = pCode[1];
  Arg2   = pCode[2];

  if (opCode == OP_NEG || opCode == OP_NOT || opCode == OP_TST || 
      opCode == OP_CMNT || opCode == OP_SQRT || opCode == OP_ABS || opCode == OP_SIGN ||
      (opCode >= OP_ACOS && opCode <= OP_FRAC) || 
      (opCode >= OP_ACOSD && opCode <= OP_SINHD))
  {
    return cCmdInterpPolyUnop2(*pCode, Arg1, 0, Arg2, 0);
  }

  switch (opCode)
  {
    case OP_MOV:
    {
      Status= cCmdMove(Arg1, Arg2);
    }
    break;

    case OP_SET:
    {
      //!!! Should throw error if TypeCode1 is non-scalar
      //    Accepting non-scalar destinations could have unpredictable results!
      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
      if (TypeCode1 == TC_SLONG)
        *(ULONG*)pArg1 = (SWORD)Arg2;
      else if (TypeCode1 == TC_ULONG)
        *(ULONG*)pArg1 = (UWORD)Arg2;
      else if (TypeCode1 < TC_ULONG)
        cCmdSetScalarValFromDataArg(Arg1, Arg2);
    }
    break;

    case OP_BRTST:
    case OP_BRTSTABSVAR:
    {
        //!!!BDUGGAN BRTST w/ Float?
      ULONG Branch, compare= COMP_CODE(pCode);
      ULONG TypeCode = cCmdDSType(Arg2);
      if(compare == OPCC1_EQ && TypeCode == TC_UBYTE) // very common for loops
      {
        UBYTE *pBRVal = (VarsCmd.pDataspace + VarsCmd.pDataspaceTOC[Arg2].DSOffset);
        Branch= *pBRVal == 0;
      }
      else
      {
        SLONG SVal1 = (SLONG)cCmdGetScalarValFromDataArg(Arg2, 0);
        Branch= ((compare == OPCC1_EQ   && SVal1 == 0)
              || (compare == OPCC1_NEQ  && SVal1 != 0)
              || (compare == OPCC1_GT   && SVal1 >  0)
              || (compare == OPCC1_LT   && SVal1 <  0)
              || (compare == OPCC1_LTEQ && SVal1 <= 0)
              || (compare == OPCC1_GTEQ && SVal1 >= 0));
      }
      if (Branch)

      {
        if (opCode == OP_BRTST)
          gPCDelta =  (SWORD)Arg1;
        else
          gPCDelta =  (UWORD)Arg1 - (pClumpRec->PC-pClumpRec->CodeStart);
        Status = NO_ERR;
      }
    }
    break;

    case OP_FINCLUMP:
    {
      CLUMP_ID Clump= VarsCmd.RunQ.Head; // DeQ changes Head, use local val
      cCmdDeQClump(&(VarsCmd.RunQ), Clump); //Dequeue finalized clump
      cCmdSchedDependents(Clump, (SWORD)Arg1, (SWORD)Arg2);
      Status = CLUMP_DONE;
    }
    break;

    case OP_PRIORITY:
    {
      // set the priority of the specified clump
      CLUMP_ID clump;
      if (Arg2 != NOT_A_DS_ID)
        clump = (CLUMP_ID)Arg1;
      else
        clump = VarsCmd.RunQ.Head;
      CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[clump]);
      pClumpRec->Priority = (UBYTE)Arg2;
    }
    break;

    case OP_SUBCALL:
    case OP_SUBCALLVAR:
    {
      if (opCode == OP_SUBCALLVAR)
      {
        if (cCmdDSType(Arg1) <= TC_LAST_INT_SCALAR)
          Arg1 = cCmdGetScalarValFromDataArg(Arg1, 0);
        else
          return (ERR_INSTR);
      }
      NXT_ASSERT(cCmdIsClumpIDSane((CLUMP_ID)Arg1));
      NXT_ASSERT(!cCmdIsClumpOnQ(&(VarsCmd.RunQ), (CLUMP_ID)Arg1));

      NXT_ASSERT(cCmdIsDSElementIDSane(Arg2));
      
      CLUMP_ID clump = VarsCmd.RunQ.Head;
      CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[clump]);
      pClumpRec->CalledClump = (CLUMP_ID)Arg1;

      *((CLUMP_ID *)(cCmdDSScalarPtr(Arg2, 0))) = clump;

      cCmdDeQClump(&(VarsCmd.RunQ), clump); //Take caller off RunQ
      cCmdEnQClump(&(VarsCmd.RunQ), (CLUMP_ID)Arg1);  //Add callee to RunQ

      Status = CLUMP_SUSPEND;
    }
    break;

    case OP_ARRSIZE:
    {
      cCmdSetScalarValFromDataArg(Arg1, cCmdArrayCount(Arg2, 0));
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
      char Buffer[36];
      //Assert that the destination is a string (array of bytes)
      NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg1)) == TC_UBYTE);

      //Make sure we're trying to convert a scalar to a string
      TypeCode2= cCmdDSType(Arg2);
      NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode2));

      if (TypeCode2 == TC_FLOAT)
      {
        FltArgVal2 = cCmdGetFloatValFromDataArg(Arg2, 0);
        if ((FltArgVal2 > (float)99999999999.9999)||(FltArgVal2 < (float)-9999999999.9999)){ // these are the widest %.4f numbers that will fit on display
          Count = sprintf(Buffer, "%.6g", FltArgVal2);
        }
        else
          Count = sprintf(Buffer, "%.4f", FltArgVal2);
        Count++; //add room for null terminator
        // remove trailing zeros
        while (Buffer[Count-2] == 0x30) {
          Buffer[Count-2] = 0x00;
          Count--;
        }
        // if last character is now a period then delete it too
        if (Buffer[Count-2] == '.') {
          Buffer[Count-2] = 0x00;
          Count--;
        }
      }
      else
      {
        ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, 0);
  
        // Calculate size of array
        if (IS_SIGNED_TYPE(TypeCode2))
        {
          Count = sprintf(Buffer, "%d", (SLONG)ArgVal2);
        }
        else
        {
          Count = sprintf(Buffer, "%u", ArgVal2);
        }
  
        //add room for NULL terminator
        Count++;
      }

      //Allocate array
      Status = cCmdDSArrayAlloc(Arg1, 0, Count);
      if (IS_ERR(Status))
        return Status;

      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);

      //Populate array
      memcpy(pArg1, Buffer, Count);
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

    case OP_WAIT:
    {
      ULONG wait= 0;
      //Unwired Arg2 defaults to wait 0, which rotates queue
      if (Arg2 != NOT_A_DS_ID)
        wait= cCmdGetScalarValFromDataArg(Arg2, 0);
      if(wait == 0)
        Status= ROTATE_QUEUE;
      else
        Status = cCmdSleepClump(wait + IOMapCmd.Tick); // put to sleep, to wake up wait ms in future
      if(Arg1 != NOT_A_DS_ID)
        cCmdSetScalarValFromDataArg(Arg1, dTimerReadNoPoll());
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


NXT_STATUS cCmdInterpPolyUnop2(CODE_WORD const Code, DATA_ARG Arg1, UWORD Offset1Param, DATA_ARG Arg2, UWORD Offset2Param)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode1, TypeCode2;
  DV_INDEX DVIndex1, DVIndex2;
  ULONG ArgVal1, ArgVal2;
  float FltArgVal1, FltArgVal2;
  UWORD Count1, Count2, Offset1= Offset1Param, Offset2= Offset2Param;
  UWORD MinArrayCount;
  UWORD i;
  //!!! AdvCluster is intended to catch the case where sources are Cluster and an Array of Clusters.
  // In practice, the logic it uses is broken, leading to some source cluster elements being ignored.
  UBYTE AdvCluster;

  void * pArg1 = NULL,
        *pArg2 = NULL;

  TypeCode1 = cCmdDSType(Arg1);
  TypeCode2 = cCmdDSType(Arg2);

  UBYTE opCode = OP_CODE(&Code);
  
  //Simple case, scalar. Solve and return.
  if (!IS_AGGREGATE_TYPE(TypeCode2))
  {
    NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode1));

    pArg1 = cCmdResolveDataArg(Arg1, Offset1, &TypeCode1);

    if (TypeCode1 == TC_FLOAT || TypeCode2 == TC_FLOAT)
    {
      pArg2 = cCmdResolveDataArg(Arg2, Offset2, &TypeCode2);
      FltArgVal2 = cCmdGetValFlt(pArg2, TypeCode2);
      FltArgVal1 = cCmdUnop2Flt(Code, FltArgVal2, TypeCode2);
      cCmdSetValFlt(pArg1, TypeCode1, FltArgVal1);
    }
    else
    {
      ArgVal2= cCmdGetScalarValFromDataArg(Arg2, Offset2);
      if(opCode == OP_MOV)
        ArgVal1= ArgVal2;
      else
        ArgVal1 = cCmdUnop2(Code, ArgVal2, TypeCode2);
      cCmdSetVal(pArg1, TypeCode1, ArgVal1);
    }
    return Status;
  }

  //At least one of the args is an aggregate type

  if(TypeCode1 == TC_ARRAY && TypeCode2 == TC_ARRAY && opCode == OP_MOV) {
    TYPE_CODE tc1, tc2;
    tc1=  cCmdDSType(INC_ID(Arg1));
    tc2=  cCmdDSType(INC_ID(Arg2));
    if((tc1 <= TC_LAST_INT_SCALAR || tc1 == TC_FLOAT) && tc1 == tc2) {
      void *pArg1, *pArg2;
      ULONG Count = cCmdArrayCount(Arg2, Offset2);
      Status = cCmdDSArrayAlloc(Arg1, Offset1, Count);
      if (IS_ERR(Status))
        return Status;
      pArg1 = cCmdResolveDataArg(Arg1, Offset1, NULL);
      pArg2 = cCmdResolveDataArg(Arg2, Offset2, NULL);
      memmove(pArg1, pArg2, Count * cCmdSizeOf(tc1));
      return Status;
    }
  }


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
    if(Count2 == 0) { // both arrays, input is empty, is output already empty?
      Count1= cCmdArrayCount(Arg1, Offset1);
      if(Count1 == 0)
        return NO_ERR;
    }

    MinArrayCount = Count2;

    //Make sure the destination array is the proper size to hold the result
    Status = cCmdDSArrayAlloc(Arg1, Offset1, MinArrayCount);
    if (IS_ERR(Status))
      return Status;

    if(MinArrayCount == 0) // if we emptied array, nothing else to do.
      return NO_ERR;
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
  if(opCode == OP_MOV)
    return Operand;
  else if(opCode == OP_NEG)
    return (-((SLONG)Operand));
  else if(opCode == OP_NOT)
    //!!! OP_NOT is logical, *not* bit-wise.
    //This differs from the other logical ops because we don't distinguish booleans from UBYTEs.
    return (!Operand);
  else if(opCode == OP_TST)
    return cCmdCompare(COMP_CODE((&Code)), Operand, 0, TypeCode, TypeCode);
  else if(opCode == OP_ABS)
    return abs(Operand);
  else if (opCode == OP_CMNT)
    return (~Operand);
  else if (opCode == OP_SIGN)
    return (((SLONG)Operand) < 0) ? -1 : ((Operand == 0) ? 0 : 1);
  else
  {
    //Unrecognized instruction, NXT_BREAK for easy debugging (ERR_INSTR handled in caller)
    NXT_BREAK;
    return 0;
  }
}

#define DEG2RAD 0.017453F
#define RAD2DEG 57.29578F

float cCmdUnop2Flt(CODE_WORD const Code, float Operand, TYPE_CODE TypeCode)
{
  UBYTE opCode;
  float ip, fp;

  opCode = OP_CODE((&Code));
  if(opCode == OP_MOV)
    return Operand;
  else if(opCode ==  OP_NEG)
    return (-(Operand));
  else if(opCode == OP_NOT)
    //!!! OP_NOT is logical, *not* bit-wise.
    //This differs from the other logical ops because we don't distinguish booleans from UBYTEs.
    return (!Operand);
  else if(opCode == OP_TST)
    return cCmdCompareFlt(COMP_CODE((&Code)), Operand, 0, TypeCode, TypeCode);
  else if(opCode == OP_ABS)
    return fabsf(Operand);
  else if (opCode == OP_SIGN)
    return (Operand < 0) ? -1 : ((Operand == 0) ? 0 : 1);
  else if(opCode == OP_SQRT)
    return sqrtf(Operand);
  else if(opCode == OP_SIN)
    return sinf(Operand);
  else if(opCode == OP_COS)
    return cosf(Operand);
  else if(opCode == OP_TAN)
    return tanf(Operand);
  else if(opCode == OP_ASIN)
    return asinf(Operand);
  else if(opCode == OP_ACOS)
    return acosf(Operand);
  else if(opCode == OP_ATAN)
    return atanf(Operand);
  else if(opCode == OP_CEIL)
    return ceilf(Operand);
  else if(opCode == OP_EXP)
    return expf(Operand);
  else if(opCode == OP_FLOOR)
    return floorf(Operand);
  else if(opCode == OP_LOG)
    return logf(Operand);
  else if(opCode == OP_LOG10)
    return log10f(Operand);
  else if (opCode == OP_TRUNC)
  {
    modff(Operand, &ip);
    return ip;
  }
  else if (opCode == OP_FRAC)
  {
    fp = modff(Operand, &ip);
    return fp;
  }
  else if(opCode == OP_SIND)
    return sinf((float)Operand*DEG2RAD);
  else if(opCode == OP_COSD)
    return cosf((float)Operand*DEG2RAD);
  else if(opCode == OP_TAND)
    return tanf((float)Operand*DEG2RAD);
  else if(opCode == OP_ASIND)
    return (float)(asinf(Operand)*RAD2DEG);
  else if(opCode == OP_ACOSD)
    return (float)(acosf(Operand)*RAD2DEG);
  else if(opCode == OP_ATAND)
    return (float)(atanf(Operand)*RAD2DEG);
  else if(opCode == OP_TANH)
    return tanhf(Operand);
  else if(opCode == OP_COSH)
    return coshf(Operand);
  else if(opCode == OP_SINH)
    return sinhf(Operand);
  else if(opCode == OP_TANHD)
    return tanhf((float)Operand*DEG2RAD);
  else if(opCode == OP_COSHD)
    return coshf((float)Operand*DEG2RAD);
  else if(opCode == OP_SINHD)
    return sinhf((float)Operand*DEG2RAD);
  else
  {
    //Unrecognized instruction, NXT_BREAK for easy debugging (ERR_INSTR handled in caller)
    NXT_BREAK;
    return 0;
  }
}

NXT_STATUS cCmdIOGetSet(ULONG opCode, DATA_ARG Arg1, DATA_ARG Arg2, DATA_ARG Arg3)
{
  ULONG ArgVal1, ArgVal2;
  TYPE_CODE TypeCode2;
  void *pArg2 = NULL;
  switch(opCode)
  {
    case OP_GETOUT:
    {
      ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, 0);
      Arg2 = (UWORD)(0x200 | (Arg3 + ArgVal2 * IO_OUT_FPP));
      pArg2 = cCmdResolveIODataArg(Arg2, 0, &TypeCode2);
      cCmdSetScalarValFromDataArg(Arg1, cCmdGetVal(pArg2, TypeCode2));
    }
    break;
    //!!! All IO map access commands should screen illegal port values!
    //    Right now, cCmdResolveIODataArg's implementation allows SETIN/GETIN to access arbitrary RAM!
    case OP_SETIN:
    {
      ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, 0);
      Arg2 = (UWORD)(Arg3 + ArgVal2 * IO_IN_FPP);
      pArg2 = cCmdResolveIODataArg(Arg2, 0, &TypeCode2);
      ArgVal1 = cCmdGetScalarValFromDataArg(Arg1, 0);
      cCmdSetVal(pArg2, TypeCode2, ArgVal1);
    }
    break;
    case OP_GETIN:
    {
      TYPE_CODE TypeCode1;
      void * pArg1;
      ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, 0);
      Arg2 = (UWORD)(Arg3 + ArgVal2 * IO_IN_FPP);
      pArg2 = cCmdResolveIODataArg(Arg2, 0, &TypeCode2);
      TypeCode1= cCmdDSType(Arg1);
      pArg1= cCmdDSScalarPtr(Arg1, 0);
      if(TypeCode1 <= TC_SBYTE && TypeCode2 <= TC_SBYTE) // seems really common
        *(UBYTE*)pArg1= *(UBYTE*)pArg2;
      else
        cCmdSetVal(pArg1, TypeCode1, cCmdGetVal(pArg2, TypeCode2));
    }
    break;
  }
  return NO_ERR;
}

ULONG scalarCmp= 0, scalarFloatCmp= 0, recursiveCmp= 0, PolyScalarCmp= 0, polyPolyCmp= 0, scalarOther= 0, scalarBinopDispatch= 0, polyBinopDispatch= 0;
NXT_STATUS cCmdInterpScalarBinop(CODE_WORD * const pCode)
{
  CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[VarsCmd.RunQ.Head]);
  NXT_STATUS Status;
  UBYTE opCode;
  UBYTE CmpBool;

  NXT_ASSERT(pCode != NULL);
  opCode = OP_CODE(pCode);
    DATA_ARG Arg1, Arg2, Arg3;

  scalarBinopDispatch ++;
  if(opCode == OP_CMP) // t2 && t3 guaranteed scalar or string
  {
    gPCDelta= 4;
    Arg1 = pCode[1];
    Arg2 = pCode[2];
    Arg3 = pCode[3];
    ULONG ArgVal1, ArgVal2, ArgVal3;
    TYPE_CODE TypeCode2, TypeCode3;
    DS_TOC_ENTRY *dsTOC2Ptr= &VarsCmd.pDataspaceTOC[Arg2];
    DS_TOC_ENTRY *dsTOC3Ptr= &VarsCmd.pDataspaceTOC[Arg3];

    TypeCode2 = dsTOC2Ptr->TypeCode;
    TypeCode3 = dsTOC3Ptr->TypeCode;
    if(TypeCode2 <= TC_LAST_INT_SCALAR && TypeCode3 <= TC_LAST_INT_SCALAR) {
      ArgVal2= GetProcArray[TypeCode2](VarsCmd.pDataspace + dsTOC2Ptr->DSOffset);
      ArgVal3= GetProcArray[TypeCode3](VarsCmd.pDataspace + dsTOC3Ptr->DSOffset);
    ArgVal1= cCmdCompare(COMP_CODE(pCode), ArgVal2, ArgVal3, TypeCode2, TypeCode3);
      DS_TOC_ENTRY *dsTOC1Ptr= &VarsCmd.pDataspaceTOC[Arg1];
      SetProcArray[dsTOC1Ptr->TypeCode](VarsCmd.pDataspace + dsTOC1Ptr->DSOffset, ArgVal1);
    scalarCmp++;
  Status = NO_ERR;
  }
    else if (TypeCode2 == TC_ARRAY) // two strings
    {
      // memcmp(); here or in compareagg, could use memcmp to speed up string compares ???
      Status = cCmdCompareAggregates(COMP_CODE(pCode), &CmpBool, Arg2, 0, Arg3, 0);
      cCmdSetScalarValFromDataArg(Arg1, CmpBool);
      recursiveCmp++;
    }
    else { // floats
      Status = cCmdInterpPolyBinop(*pCode, Arg1, 0, Arg2, 0, Arg3, 0);
      scalarFloatCmp++;
    }
  }
  else if(opCode == OP_BRCMP || opCode == OP_BRCMPABSVAR) { // t2 and t3 guaranteed scalar
      TYPE_CODE TypeCode2, TypeCode3;
      ULONG ArgVal2, ArgVal3;

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      TypeCode2= cCmdDSType(Arg2);
      TypeCode3= cCmdDSType(Arg3);
      ArgVal2= cCmdGetScalarValFromDataArg(Arg2, 0);
      ArgVal3= cCmdGetScalarValFromDataArg(Arg3, 0);
      CmpBool= cCmdCompare(COMP_CODE(pCode), ArgVal2, ArgVal3, TypeCode2, TypeCode3);

      if (CmpBool) {
        if (opCode == OP_BRCMP)
          gPCDelta = (SWORD)Arg1;
        else
          gPCDelta = (UWORD)Arg1-(pClumpRec->PC-pClumpRec->CodeStart);
      }
      else
        gPCDelta = 4;
      Status= NO_ERR;
    }
  else if(opCode >= OP_SETIN && opCode <= OP_GETOUT) {
      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
    Status= cCmdIOGetSet(opCode, Arg1, Arg2, Arg3);
    gPCDelta= 4;
    }
  else {
    scalarOther ++;
    Status= cCmdInterpBinop(pCode);
  }
  return Status;
}


NXT_STATUS cCmdInterpBinop(CODE_WORD * const pCode)
{
  CLUMP_REC* pClumpRec = &(VarsCmd.pAllClumps[VarsCmd.RunQ.Head]);
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1, Arg2, Arg3;
  ULONG ArgVal3;
  UBYTE CmpBool;
  DV_INDEX DVIndex1, DVIndex2;
  UWORD i;
  void *pArg1 = NULL, *pArg2 = NULL;
  UWORD Count;
  TYPE_CODE TypeCode1;

  polyBinopDispatch ++;
  gPCDelta= 4;

  NXT_ASSERT(pCode != NULL);
    opCode = OP_CODE(pCode);
    Arg1 = pCode[1];
    Arg2 = pCode[2];
    Arg3 = pCode[3];

  if ((opCode <= OP_XOR) || 
      (opCode >= OP_LSL && opCode <= OP_ROTR) ||
      (opCode == OP_ATAN2) || (opCode == OP_POW) || (opCode == OP_ATAN2D)) // && ! OP_NEG, can't happen since it is unop
    Status= cCmdInterpPolyBinop(opCode, Arg1, 0, Arg2, 0, Arg3, 0);
  else if(opCode >= OP_SETIN && opCode <= OP_GETOUT)
    Status= cCmdIOGetSet(opCode, Arg1, Arg2, Arg3);
  else {
  switch (opCode)
  {
    case OP_CMP:
    {
        TYPE_CODE TypeCode2= cCmdDSType(Arg2), TypeCode3= cCmdDSType(Arg3);
      if(TypeCode2 <= TC_LAST_INT_SCALAR && TypeCode3 <= TC_LAST_INT_SCALAR) {
          ULONG ArgVal1, ArgVal2, ArgVal3;
          ArgVal2= cCmdGetScalarValFromDataArg(Arg2, 0);
          ArgVal3= cCmdGetScalarValFromDataArg(Arg3, 0);
          ArgVal1= cCmdCompare(COMP_CODE(pCode), ArgVal2, ArgVal3, TypeCode2, TypeCode3);
          cCmdSetScalarValFromDataArg(Arg1, ArgVal1);
          PolyScalarCmp++;
        }
      else if (IS_AGGREGATE_TYPE(TypeCode2) && IS_AGGREGATE_TYPE(TypeCode3) && !IS_AGGREGATE_TYPE(cCmdDSType(Arg1)))
      {
        //Compare Aggregates
        Status = cCmdCompareAggregates(COMP_CODE(pCode), &CmpBool, Arg2, 0, Arg3, 0);
        cCmdSetScalarValFromDataArg(Arg1, CmpBool);
        recursiveCmp++;
      }
      else
      {
        //Compare Elements
        Status = cCmdInterpPolyBinop(*pCode, Arg1, 0, Arg2, 0, Arg3, 0);
        polyPolyCmp++;
      }
    }
    break;

    case OP_BRCMP:
    case OP_BRCMPABSVAR:
    {
      TYPE_CODE TypeCode2= cCmdDSType(Arg2), TypeCode3= cCmdDSType(Arg3);
      if(TypeCode2 <= TC_LAST_INT_SCALAR && TypeCode3 <= TC_LAST_INT_SCALAR) {
        ULONG ArgVal2, ArgVal3;
        ArgVal2= cCmdGetScalarValFromDataArg(Arg2, 0);
        ArgVal3= cCmdGetScalarValFromDataArg(Arg3, 0);
        CmpBool= cCmdCompare(COMP_CODE(pCode), ArgVal2, ArgVal3, TypeCode2, TypeCode3);
      }
      else //Compare Aggregates
      Status = cCmdCompareAggregates(COMP_CODE(pCode), &CmpBool, Arg2, 0, Arg3, 0);

      if (CmpBool)
          gPCDelta =  (SWORD)Arg1;
      else
          gPCDelta =  (UWORD)Arg1 - (pClumpRec->PC-pClumpRec->CodeStart);
    }
    break;

    case OP_INDEX:
    {
        ArgVal3 = (Arg3 != NOT_A_DS_ID) ? cCmdGetScalarValFromDataArg(Arg3, 0) : 0;

      DVIndex2 = cCmdGetDVIndex(Arg2, 0);
      if (ArgVal3 >= DV_ARRAY[DVIndex2].Count)
        return (ERR_ARG);

      Status = cCmdInterpPolyUnop2(OP_MOV, Arg1, 0, INC_ID(Arg2), ARRAY_ELEM_OFFSET(DVIndex2, ArgVal3));
    }
    break;

    case OP_ARRINIT:
    {
      //Arg1 - Dst, Arg2 - element type/default val, Arg3 - length

        NXT_ASSERT(cCmdDSType(Arg1) == TC_ARRAY);

        // determine the type of the array destination arg
        TYPE_CODE TypeCode = cCmdDSType(INC_ID(Arg1));

        // How many elements do we want?
        ArgVal3 = (Arg3 != NOT_A_DS_ID) ? cCmdGetScalarValFromDataArg(Arg3, 0) : 0;

      Status = cCmdDSArrayAlloc(Arg1, 0, (UWORD)ArgVal3);
        if (!IS_ERR(Status))
        {
      DVIndex1 = cCmdGetDVIndex(Arg1, 0);
          if(cCmdDSType(Arg2) <= TC_LAST_INT_SCALAR && TypeCode <= TC_LAST_INT_SCALAR)
          {
            ULONG val= cCmdGetScalarValFromDataArg(Arg2, 0);
            for (i = 0; i < ArgVal3; i++) // could init ptr and incr by offset GM???
      {
        //copy Arg2 into each element of Arg1
              cCmdSetVal(VarsCmd.pDataspace + ARRAY_ELEM_OFFSET(DVIndex1, i), TypeCode, val);
            }
          }
          else
            for (i = 0; i < ArgVal3; i++)  //copy Arg2 into each element of Arg1
        Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg1), ARRAY_ELEM_OFFSET(DVIndex1, i), Arg2, 0);
      }
    }
    break;

      case OP_FMTNUM:
      {
        //Check that the destination is a string (array of bytes)
        if (cCmdDSType(Arg1) != TC_ARRAY || cCmdDSType(INC_ID(Arg1)) != TC_UBYTE) {
          Status = ERR_INSTR;
          return (Status);
        }

        //Check that the format is a string (array of bytes)
        if (cCmdDSType(Arg2) != TC_ARRAY || cCmdDSType(INC_ID(Arg2)) != TC_UBYTE) {
          Status = ERR_INSTR;
          return (Status);
        }

        pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);
        TYPE_CODE TypeCode3 = cCmdDSType(Arg3);

        //Make sure we're trying to convert a scalar/float to a string
        if (TypeCode3 == TC_VOID || (TypeCode3 > TC_LAST_INT_SCALAR && TypeCode3 != TC_FLOAT)) {
          Status = ERR_INSTR;
          return (Status);
        }

        char fmtBuf[256]; // arbitrary limit!!!
        // handle floats separately from scalar types
        if (TypeCode3 == TC_FLOAT) {
          float FltArgVal3 = cCmdGetFloatValFromDataArg(Arg3, 0);
          Count = sprintf(fmtBuf, pArg2, FltArgVal3);
        }
        else
        {
          ArgVal3 = cCmdGetScalarValFromDataArg(Arg3, 0);
          // Calculate size of array
          if (IS_SIGNED_TYPE(TypeCode3))
          {
            Count = sprintf(fmtBuf, pArg2, (SLONG)ArgVal3);
          }
          else
          {
            Count = sprintf(fmtBuf, pArg2, ArgVal3);
          }
        }

        //add room for NULL terminator
        Count++;

        //Allocate array
        Status = cCmdDSArrayAlloc(Arg1, 0, Count);
        if (IS_ERR(Status))
          return Status;

        pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);

        //Populate array
        memcpy(pArg1, fmtBuf, Count);
      }
      break;
      
      case OP_ADDROF:
      {
        pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
        if (TypeCode1 == TC_ULONG) {
          pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);
          if ((UBYTE)Arg3) // relative address requested
            *(ULONG*)pArg1 = (ULONG)pArg2 - (ULONG)(IOMapCmd.MemoryPool);
          else
            *(ULONG*)pArg1 = (ULONG)pArg2;
        }
        else
          Status = ERR_INSTR; // output argument MUST be an unsigned long type
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
  }
  return (Status);
}


NXT_STATUS cCmdInterpPolyBinop(CODE_WORD const Code, DATA_ARG Arg1, UWORD Offset1, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3)
{
  NXT_STATUS Status = NO_ERR;
  TYPE_CODE TypeCode1, TypeCode2, TypeCode3;
  DV_INDEX DVIndex1, DVIndex2, DVIndex3;
  ULONG ArgVal1, ArgVal2, ArgVal3;
  float FltArgVal1, FltArgVal2, FltArgVal3;
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

    pArg1 = cCmdResolveDataArg(Arg1, Offset1, NULL);

    if (TypeCode1 == TC_FLOAT || TypeCode2 == TC_FLOAT || TypeCode3 == TC_FLOAT){
      pArg2 = cCmdResolveDataArg(Arg2, Offset2, NULL);
      pArg3 = cCmdResolveDataArg(Arg3, Offset3, NULL);
      FltArgVal2 = cCmdGetValFlt(pArg2, TypeCode2);
      FltArgVal3 = cCmdGetValFlt(pArg3, TypeCode3);
      FltArgVal1 = cCmdBinopFlt(Code, FltArgVal2, FltArgVal3, TypeCode2, TypeCode3);
      cCmdSetValFlt(pArg1, TypeCode1, FltArgVal1);
    }
    else
    {
    ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, Offset2);
    ArgVal3 = cCmdGetScalarValFromDataArg(Arg3, Offset3);
    ArgVal1 = cCmdBinop(Code, ArgVal2, ArgVal3, TypeCode2, TypeCode3);
    cCmdSetVal(pArg1, TypeCode1, ArgVal1);
    }
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

    case OP_LSL:
    {
      if (((SLONG)RightOp) <= 0)
        return LeftOp; // negative shifts == shifting by zero
      else
        return LeftOp << RightOp;
    }

    case OP_LSR:
    {
      if (((SLONG)RightOp) <= 0)
        return LeftOp; // negative shifts == shifting by zero
      else
        return LeftOp >> RightOp;
    }
    
    case OP_ASL:
    {
      if (((SLONG)RightOp) <= 0)
        return LeftOp; // negative shifts == shifting by zero
      else if (!IS_SIGNED_TYPE(LeftType))
        return LeftOp << RightOp;
      else
        return LeftOp * (1 << RightOp);
    }

    case OP_ASR:
    {
      if (((SLONG)RightOp) <= 0)
        return LeftOp; // negative shifts == shifting by zero
      else if (!IS_SIGNED_TYPE(LeftType))
        return LeftOp >> RightOp;
      else 
        return ((SLONG)LeftOp) / (1 << RightOp);
    }
    
    case OP_ROTL:
    {
      if (((SLONG)RightOp) <= 0)
        return LeftOp; // negative rotates == rotating by zero
      else {
        if (LeftType == TC_ULONG || LeftType == TC_SLONG)
          return (LeftOp << RightOp) | (LeftOp >> (32 - RightOp));
        else if (LeftType == TC_UWORD || LeftType == TC_SWORD)
          return (((UWORD)LeftOp) << RightOp) | (((UWORD)LeftOp) >> (16 - RightOp));
        else if (LeftType == TC_UBYTE || LeftType == TC_SBYTE)
          return (((UBYTE)LeftOp) << RightOp) | (((UBYTE)LeftOp) >> (8 - RightOp));
      }
    }

    case OP_ROTR:
    {
      if (((SLONG)RightOp) <= 0)
        return LeftOp; // negative rotates == rotating by zero
      else {
        if (LeftType == TC_ULONG || LeftType == TC_SLONG)
          return (LeftOp >> RightOp) | (LeftOp << (32 - RightOp));
        else if (LeftType == TC_UWORD || LeftType == TC_SWORD)
          return (((UWORD)LeftOp) >> RightOp) | (((UWORD)LeftOp) << (16 - RightOp));
        else if (LeftType == TC_UBYTE || LeftType == TC_SBYTE)
          return (((UBYTE)LeftOp) >> RightOp) | (((UBYTE)LeftOp) << (8 - RightOp));
      }
    }
    
    default:
    {
      //Unrecognized instruction, NXT_BREAK for easy debugging (ERR_INSTR handled in caller)
      NXT_BREAK;
      return 0;
    }
  }
}


float cCmdBinopFlt(CODE_WORD const Code, float LeftOp, float RightOp, TYPE_CODE LeftType, TYPE_CODE RightType)
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

      return LeftOp / RightOp;
    }

    case OP_MOD:
    {
      //As with OP_DIV, make sure (x % 0) = x is well-defined
      if (RightOp == 0)
        return (LeftOp);

      return fmodf(LeftOp, RightOp);
    }

    case OP_AND:
    {
      return ((SLONG)LeftOp & (SLONG)RightOp);
    }

    case OP_OR:
    {
      return ((SLONG)LeftOp | (SLONG)RightOp);
    }

    case OP_XOR:
    {
      return (((SLONG)LeftOp | (SLONG)RightOp) & (~((SLONG)LeftOp & (SLONG)RightOp)));
    }

    case OP_CMP:
    {
      return cCmdCompareFlt(COMP_CODE((&Code)), LeftOp, RightOp, LeftType, RightType);
    }

    case OP_ATAN2:
    {
      return atan2f(LeftOp, RightOp);
    }

    case OP_POW:
    {
      float intpart, fracpart;
      fracpart = modff(RightOp, &intpart);
      if (LeftOp < 0 && fracpart != 0)
        return 0; // make the result zero if you try to raise a negative number to a fractional exponent
      else
        return powf(LeftOp, RightOp);
    }

    case OP_ATAN2D:
    {
      return (float)(atan2f(LeftOp, RightOp)*RAD2DEG);
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

NXT_STATUS cCmdInterpShortError(CODE_WORD * const pCode)
{
  //Fatal error: Unrecognized instruction (no current opcodes have zero instructions)
  NXT_BREAK;
  return (ERR_INSTR);
}

NXT_STATUS cCmdInterpShortSubCall(CODE_WORD * const pCode)
{
  NXT_STATUS Status;
  DATA_ARG Arg1, Arg2;

  gPCDelta= 2;
  Arg1 = GetDataArg(SHORT_ARG(pCode) + pCode[1]);
  Arg2 = GetDataArg(pCode[1]);
  NXT_ASSERT(cCmdIsClumpIDSane((CLUMP_ID)Arg1));
  NXT_ASSERT(!cCmdIsClumpOnQ(&(VarsCmd.RunQ), (CLUMP_ID)Arg1));

  NXT_ASSERT(cCmdIsDSElementIDSane(Arg2));

  *((CLUMP_ID *)(cCmdDSScalarPtr(Arg2, 0))) = VarsCmd.RunQ.Head;

  cCmdDeQClump(&(VarsCmd.RunQ), VarsCmd.RunQ.Head); //Take caller off RunQ
  cCmdEnQClump(&(VarsCmd.RunQ), (CLUMP_ID)Arg1);  //Add callee to RunQ

  Status = CLUMP_SUSPEND;

  return Status;
}

ULONG moveSameInt= 0, moveDiffInt= 0, moveFloat= 0, moveIntFloat= 0, moveFloatInt= 0, moveArrInt= 0, moveOther= 0;
NXT_STATUS cCmdMove(DATA_ARG Arg1, DATA_ARG Arg2)
{
  NXT_STATUS Status;
  DS_TOC_ENTRY  *TOC1Ptr= &VarsCmd.pDataspaceTOC[Arg1],
                *TOC2Ptr= &VarsCmd.pDataspaceTOC[Arg2];
  TYPE_CODE tc1= TOC1Ptr->TypeCode, tc2= TOC2Ptr->TypeCode;
  UBYTE ElemSize1 = cCmdSizeOf((TOC1Ptr+1)->TypeCode), 
        ElemSize2 = cCmdSizeOf((TOC2Ptr+1)->TypeCode);
  void *pArg1, *pArg2;

  if(tc1 <= TC_LAST_INT_SCALAR && tc2 <= TC_LAST_INT_SCALAR)
  {
    // if tc1 == tc2, do long, byte, then word assignment
    if(tc1 == tc2)
    {
      moveSameInt++;
      pArg1= VarsCmd.pDataspace + TOC1Ptr->DSOffset;
      pArg2= VarsCmd.pDataspace + TOC2Ptr->DSOffset;
      if(tc1 >= TC_ULONG)
        *(ULONG*)pArg1= *(ULONG*)pArg2;
      else if(tc1 <= TC_SBYTE)
        *(UBYTE*)pArg1= *(UBYTE*)pArg2;
      else
        *(UWORD*)pArg1= *(UWORD*)pArg2;
      Status= NO_ERR;
    }
  else
    {
      moveDiffInt++;
      ULONG val= cCmdGetScalarValFromDataArg(Arg2, 0);
      cCmdSetScalarValFromDataArg(Arg1, val);
      Status= NO_ERR;
    }
  }
  else if(tc1 == TC_FLOAT && tc2 == TC_FLOAT) { // may also need to speed up float to int and int to float conversions
    moveFloat++;
    pArg1= VarsCmd.pDataspace + TOC1Ptr->DSOffset;
    pArg2= VarsCmd.pDataspace + TOC2Ptr->DSOffset;
    *(float*)pArg1= *(float*)pArg2;
    Status= NO_ERR;
  }
  else if(tc1 == TC_FLOAT && tc2 <= TC_LAST_INT_SCALAR) { // int to float
    moveIntFloat++;
    pArg1= VarsCmd.pDataspace + TOC1Ptr->DSOffset;
    pArg2= VarsCmd.pDataspace + TOC2Ptr->DSOffset;
    if (tc2 == TC_SLONG)
      *(float*)pArg1 = *(SLONG*)pArg2;
    else if (tc2 == TC_ULONG)
      *(float*)pArg1 = *(ULONG*)pArg2;
    else if (tc2 == TC_SBYTE)
      *(float*)pArg1 = *(SBYTE*)pArg2;
    else if (tc2 == TC_UBYTE)
      *(float*)pArg1 = *(UBYTE*)pArg2;
    else if (tc2 == TC_UWORD)
      *(float*)pArg1 = *(UWORD*)pArg2;
    else
      *(float*)pArg1= *(SWORD*)pArg2;
    Status= NO_ERR;
  }
  else if(tc2 == TC_FLOAT && tc1 <= TC_LAST_INT_SCALAR) { // float to int
    moveFloatInt++;
    pArg1= VarsCmd.pDataspace + TOC1Ptr->DSOffset;
    pArg2= VarsCmd.pDataspace + TOC2Ptr->DSOffset;
    if (tc1 == TC_SLONG)
      *(SLONG*)pArg1 = *(float*)pArg2;
    else if (tc1 == TC_ULONG)
      *(ULONG*)pArg1 = *(float*)pArg2;
    else if (tc1 == TC_SBYTE)
      *(SBYTE*)pArg1 = *(float*)pArg2;
    else if (tc1 == TC_UBYTE)
      *(UBYTE*)pArg1 = *(float*)pArg2;
    else if (tc1 == TC_UWORD)
      *(UWORD*)pArg1 = *(float*)pArg2;
    else
      *(SWORD*)pArg1 = *(float*)pArg2;
    Status= NO_ERR;
  }
  //!!! Optimized move for arrays of ints and floats.
  else if ((tc1 == TC_ARRAY) && (tc2 == TC_ARRAY) &&
           (((TOC1Ptr+1)->TypeCode <= TC_LAST_INT_SCALAR && ElemSize1 == ElemSize2) || 
            ((TOC1Ptr+1)->TypeCode == TC_FLOAT && (TOC2Ptr+1)->TypeCode == TC_FLOAT)))
  {
    ULONG Count;
    moveArrInt++;
    Count = cCmdArrayCount(Arg2, 0);
    Status = cCmdDSArrayAlloc(Arg1, 0, Count);
    if (IS_ERR(Status))
      return Status;

    pArg1 = cCmdResolveDataArg(Arg1, 0, NULL);
    pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);

    memmove(pArg1, pArg2, Count * cCmdSizeOf((TOC1Ptr+1)->TypeCode));
  }
  else { // if ((tc1 == TC_CLUSTER) && (tc2 == TC_CLUSTER))
    moveOther++;
    Status = cCmdInterpPolyUnop2(OP_MOV, Arg1, 0, Arg2, 0);
  }
  return Status;
}


NXT_STATUS cCmdInterpShortMove(CODE_WORD * const pCode)
{
  NXT_STATUS Status;
  DATA_ARG Arg1, Arg2;

  Arg1 = GetDataArg(SHORT_ARG(pCode) + pCode[1]);
  Arg2 = GetDataArg(pCode[1]);
  Status= cCmdMove(Arg1, Arg2);

  gPCDelta= 2;
  return Status;
}

NXT_STATUS cCmdInterpShortAcquire(CODE_WORD * const pCode)
{
  NXT_STATUS Status;
  DATA_ARG Arg1;

  gPCDelta= 1;
  Arg1 = GetDataArg(SHORT_ARG(pCode));
  NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
  NXT_ASSERT(cCmdDSType(Arg1) == TC_MUTEX);

  Status = cCmdAcquireMutex((MUTEX_Q *)cCmdDSScalarPtr(Arg1, 0));

  return Status;
}

NXT_STATUS cCmdInterpShortRelease(CODE_WORD * const pCode)
{
  NXT_STATUS Status;
  DATA_ARG Arg1;

  gPCDelta= 1;
  Arg1 = GetDataArg(SHORT_ARG(pCode));
  NXT_ASSERT(cCmdIsDSElementIDSane(Arg1));
  NXT_ASSERT(cCmdDSType(Arg1) == TC_MUTEX);

  Status = cCmdReleaseMutex((MUTEX_Q *)cCmdDSScalarPtr(Arg1, 0));

  return Status;
}


ULONG cCmdGetPortFromValue(ULONG val, ULONG i)
{
  ULONG result = NO_OF_OUTPUTS; // invalid NO-OP output
  if (val < NO_OF_OUTPUTS)
    result = val;
  else
  {
    if (val <= RC_OUT_ABC)
    {
      result = i;
      if ((val == RC_OUT_BC) || (val == RC_OUT_AC && i))
        result++;
    }
  }
  return result;
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
  ULONG Port, FieldTableIndex, i, j, val = 0;
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
  {
    // arg may refer to multiple ports 
    // (0, 1, 2 are single ports; 
    val = cCmdGetScalarValFromDataArg(PortArg, 0);
    if (val < NO_OF_OUTPUTS)
      PortCount = 1;
    else if (val < RC_OUT_ABC)
      PortCount = 2;
    else
      PortCount = 3;
  }

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
      Port = cCmdGetPortFromValue(val, i);
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


void shell_sort_u1(UBYTE* A, UWORD size)
{ 
  UWORD i, j, increment; 
  UBYTE temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

void shell_sort_s1(SBYTE* A, UWORD size)
{ 
  UWORD i, j, increment; 
  SBYTE temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

void shell_sort_u2(UWORD* A, UWORD size)
{ 
  UWORD i, j, increment;
  UWORD temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

void shell_sort_s2(SWORD* A, UWORD size)
{ 
  UWORD i, j, increment;
  SWORD temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

void shell_sort_u4(ULONG* A, UWORD size)
{ 
  UWORD i, j, increment;
  ULONG temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

void shell_sort_s4(SLONG* A, UWORD size)
{ 
  UWORD i, j, increment;
  SLONG temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

void shell_sort_flt(float* A, UWORD size)
{ 
  UWORD i, j, increment;
  float temp;
  increment = size / 2;
 
  while (increment > 0) { 
    for (i = increment; i < size; i++) { 
      j = i;
      temp = A[i];
      while ((j >= increment) && (A[j-increment] > temp)) { 
        A[j] = A[j - increment];
        j = j - increment;
      }
      A[j] = temp;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (UWORD)((float)increment / (float)2.2);
  }
}

NXT_STATUS cCmdInterpOther(CODE_WORD * const pCode)
{
  NXT_STATUS Status = NO_ERR;
  UBYTE opCode;
  DATA_ARG Arg1, Arg2, Arg3, Arg4, Arg5;
  TYPE_CODE TypeCode1, TypeCode2, TypeCode3, TypeCode5;
  ULONG ArgVal2, ArgVal3, ArgVal4, ArgVal5;
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
  void  *pArg5 = NULL;

  NXT_ASSERT(pCode != NULL);

  ULONG sz= INSTR_SIZE(*(UWORD*)pCode);
  if (sz == VAR_INSTR_SIZE)
    sz = ((UWORD*)pCode)[1];
  gPCDelta= sz/2; // advance words, sz is in bytes

  opCode = OP_CODE(pCode);

  switch (opCode)
  {

    case OP_REPLACE:
    {
      //Arg1 - Dst
      //Arg2 - Src
      //Arg3 - Index
      //Arg4 - New val / array of vals
      UWORD SrcDims, NewValDims;

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
//        Status= cCmdMove(Arg1, Arg2);
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

      SrcDims    = cCmdArrayDimensions(Arg2);
      NewValDims = cCmdArrayDimensions(Arg4);
      // if the new value argument has an array dimension that is 1 less than
      // the array dimension of the source array then use MOV to copy data
      if (NewValDims == (SrcDims-1))
      {
        Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg1), ARRAY_ELEM_OFFSET(DVIndex1, ArgVal3), Arg4, 0);
        if (IS_ERR(Status))
          return Status;
      }
      else if (NewValDims == SrcDims)
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
      else
      {
        // any other situation is unsupported
        NXT_BREAK;
        return 0;
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
        ArgVal3 = cCmdGetScalarValFromDataArg(Arg3, 0);
      else //Index input unwired
        ArgVal3 = 0;

      if (Arg4 != NOT_A_DS_ID)
        ArgVal4 = cCmdGetScalarValFromDataArg(Arg4, 0);
      else //Length input unwired, set to "rest"
        ArgVal4 = (UWORD)(ArrayCount2 - ArgVal3);

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
        ArgVal3 = cCmdGetScalarValFromDataArg(Arg3, 0);
      else //Index input unwired
        ArgVal3 = 0;

      if (Arg4 != NOT_A_DS_ID)
        ArgVal4 = cCmdGetScalarValFromDataArg(Arg4, 0);
      else //Length input unwired, set to "rest"
        ArgVal4 = (UWORD)(ArrayCount2 - ArgVal3);

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
          // if flat, use memmove, otherwise this stuff
          if(cCmdDSType(INC_ID(TmpDSID)) <= TC_LAST_INT_SCALAR)
          {
            memmove(VarsCmd.pDataspace + ARRAY_ELEM_OFFSET(DVIndex2, DstIndex), VarsCmd.pDataspace + DV_ARRAY[TmpDVIndex].Offset, (UWORD)(DV_ARRAY[TmpDVIndex].ElemSize * DV_ARRAY[TmpDVIndex].Count));
            DstIndex += DV_ARRAY[TmpDVIndex].Count;
          }
          else
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
      Status= cCmdMove(Arg1, Arg4);
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
      ULONG ArgVal1;
      float ArgValF;
      SLONG decimals= 0;
      UBYTE exponent=FALSE;
      UBYTE cont= TRUE;
      // Arg1 - Dst number (output)
      // Arg2 - Offset past match (output)
      // Arg3 - Src string
      // Arg4 - Offset
      // Arg5 - Default (type/value)

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];
      Arg5 = pCode[5];

      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
      pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);

      if (Arg4 != NOT_A_DS_ID)
        ArgVal4 = cCmdGetScalarValFromDataArg(Arg4, 0);
      else //Offset input unwired
        ArgVal4 = 0;

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
      // scan either to integer or float, depending on TypeCode1
      int scanResult;
      if (TypeCode1 == TC_FLOAT)
        scanResult = sscanf(((PSZ)pArg3 + ArgVal4), "%f", &ArgValF);
      else
        scanResult = sscanf(((PSZ)pArg3 + ArgVal4), "%d", &ArgVal1);
      // check the result
      if (scanResult == 1)
      {
        if (Arg2 != NOT_A_DS_ID) 
        {
          i = (UWORD)ArgVal4;
          //Scan until we see the number, consumes negative sign too
          while ((((UBYTE *)pArg3)[i] < '0') || (((UBYTE *)pArg3)[i] > '9'))
            i++;
  
          if (TypeCode1 == TC_FLOAT)
          {
            //Scan until we get past the number and no more than one decimal
            // optionally there can also be a single "e" or "E" followed by 
            // one or more digits (but the decimal cannot come after this)
            while (cont) {
              UBYTE ch = ((UBYTE *)pArg3)[i];
              if ((ch >= '0') && (ch <= '9'))
                i++;
              else if(ch == '.' && !decimals && !exponent) {
                i++;
                decimals++;
              }
              else if (((ch == 'E') || (ch == 'e')) && !exponent) {
                i++;
                exponent = TRUE;
              }
              else
                cont= FALSE;
            }
          }
          else {
            //Scan until we get past the number
            while ((((UBYTE *)pArg3)[i] >= '0') && (((UBYTE *)pArg3)[i] <= '9'))
              i++;
          }
          ArgVal2 = i;
        }
      }
      else
      {
        //Number wasn't found in string, use defaults
        ArgValF = ArgVal5;
        ArgVal1 = ArgVal5;
        ArgVal2 = 0;
      }

      //Set outputs
      if (TypeCode1 == TC_FLOAT)
        cCmdSetValFlt(pArg1, TypeCode1, ArgValF);
      else
        cCmdSetVal(pArg1, TypeCode1, ArgVal1);
      if (Arg2 != NOT_A_DS_ID)
        cCmdSetScalarValFromDataArg(Arg2, ArgVal2);
    }
    break;

    case OP_ARROP:
    {
      //Arg1 - Command (immediate constant)
      //Arg2 - Dst (scalar|array)
      //Arg3 - Src (scalar array)
      //Arg4 - Index
      //Arg5 - Length

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];
      Arg5 = pCode[5];

      // array operation
      if (Arg1 == OPARR_SORT) {
        // destination must be an array of non-aggregate type
        NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);
        TypeCode2 = cCmdDSType(INC_ID(Arg2));
        NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode2));
      }
      else if (Arg1 == OPARR_TOUPPER || Arg1 == OPARR_TOLOWER) {
        // destination must be an array of ubyte type
        NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);
        TypeCode2 = cCmdDSType(INC_ID(Arg2));
        NXT_ASSERT(TypeCode2 == TC_UBYTE);
      }
      else {
        // destination must be a non-aggregate type
        NXT_ASSERT(!IS_AGGREGATE_TYPE(cCmdDSType(Arg2)));
      }
      // source must be an array of non-aggregate type
      NXT_ASSERT(cCmdDSType(Arg3) == TC_ARRAY);
      TypeCode3 = cCmdDSType(INC_ID(Arg3));
      NXT_ASSERT(!IS_AGGREGATE_TYPE(TypeCode3));

      ArrayCount3 = cCmdArrayCount(Arg3, 0);

      if (Arg4 != NOT_A_DS_ID)
        ArgVal4 = cCmdGetScalarValFromDataArg(Arg4, 0);
      else //Index input unwired
        ArgVal4 = 0;

      if (Arg5 != NOT_A_DS_ID)
        ArgVal5 = cCmdGetScalarValFromDataArg(Arg5, 0);
      else //Length input unwired, set to "rest"
        ArgVal5 = 0xFFFF;

      //Bounds check
      if (ArgVal4 > ArrayCount3)
      {
        if (Arg1 == OPARR_SORT) {
          //Illegal range - return empty subset
          Status = cCmdDSArrayAlloc(Arg2, 0, 0);
          return Status;
        }
        else {
          //Illegal range - return zero
          pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
          cCmdSetVal(pArg2, TypeCode2, 0);
          return NO_ERR;
        }
      }

      //Set MinCount to "rest"
      MinCount = (UWORD)(ArrayCount3 - ArgVal4);

      // Copy "Length" if it is less than "rest"
      if (ArgVal5 < (ULONG)MinCount)
        MinCount = (UWORD)ArgVal5;

      DV_INDEX DVIndex3 = cCmdGetDVIndex(Arg3, 0);

      SLONG sval, svaltmp;
      ULONG uval, uvaltmp;
      float fval, fvaltmp;
      float numElements = (float)MinCount;
      //sum elements from src subset to dst
      if ((Arg1 == OPARR_SUM) || (Arg1 == OPARR_MEAN) ||
          (Arg1 == OPARR_SUMSQR) || (Arg1 == OPARR_STD)) 
      {
        pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
        if (TypeCode2 == TC_FLOAT)
        {
          fval = 0;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            fvaltmp = cCmdGetValFlt(pArg3, TypeCode3);
            if (Arg1 == OPARR_SUMSQR)
              fvaltmp *= fvaltmp;
            fval += fvaltmp;
          }
          if (Arg1 == OPARR_MEAN)
            cCmdSetValFlt(pArg2, TypeCode2, fval/numElements);
          else if (Arg1 != OPARR_STD)
            cCmdSetValFlt(pArg2, TypeCode2, fval);
        }
        else if (IS_SIGNED_TYPE(TypeCode2) && (Arg1 != OPARR_SUMSQR))
        {
          sval = 0;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            svaltmp = (SLONG)cCmdGetVal(pArg3, TypeCode3);
            sval += svaltmp;
          }
          if (Arg1 == OPARR_MEAN)
            cCmdSetVal(pArg2, TypeCode2, (SLONG)(float)sval/numElements);
          else if (Arg1 != OPARR_STD)
            cCmdSetVal(pArg2, TypeCode2, sval);
        }
        else
        {
          uval = 0;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            if (IS_SIGNED_TYPE(TypeCode2)) 
            {
              // this can only be the SUMSQR operation (given the IF statement above)
              svaltmp = cCmdGetVal(pArg3, TypeCode3);
              uvaltmp = (ULONG)abs(svaltmp) * (ULONG)abs(svaltmp);
              uval += uvaltmp;
            }
            else {
              uvaltmp = cCmdGetVal(pArg3, TypeCode3);
              if (Arg1 == OPARR_SUMSQR)
                uvaltmp *= uvaltmp;
              uval += uvaltmp;
            }
          }
          if (Arg1 == OPARR_MEAN)
            cCmdSetVal(pArg2, TypeCode2, (ULONG)(float)uval/numElements);
          else if (Arg1 != OPARR_STD)
            cCmdSetVal(pArg2, TypeCode2, uval);
        }
        // calculate standard deviation
        if (Arg1 == OPARR_STD) 
        {
          float avg, delta, sumSqr;
          if (TypeCode2 == TC_FLOAT)
            avg = fval/numElements;
          else if (IS_SIGNED_TYPE(TypeCode2)) 
            avg = (float)sval/numElements;
          else
            avg = (float)uval/numElements;
          sumSqr = 0;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            if (TypeCode2 == TC_FLOAT)
              delta = cCmdGetValFlt(pArg3, TypeCode3) - avg;              
            else if (IS_SIGNED_TYPE(TypeCode2))
              delta = (float)(SLONG)cCmdGetVal(pArg3, TypeCode3) - avg;
            else // unsigned types
              delta = (float)cCmdGetVal(pArg3, TypeCode3) - avg;
            sumSqr += (delta*delta);
          }
          delta = sqrtf(sumSqr / (numElements - (float)1.0));
          if (TypeCode2 == TC_FLOAT)
            cCmdSetValFlt(pArg2, TypeCode2, delta);
          else if (IS_SIGNED_TYPE(TypeCode2))
            cCmdSetVal(pArg2, TypeCode2, (SLONG)delta);
          else
            cCmdSetVal(pArg2, TypeCode2, (ULONG)delta);
        }
      }
      else if ((Arg1 == OPARR_MIN) || (Arg1 == OPARR_MAX)) 
      {
        pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
        if (TypeCode2 == TC_FLOAT)
        {
          if (Arg1 == OPARR_MIN)
            fval = FLT_MAX;
          else
            fval = -FLT_MAX;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            fvaltmp = cCmdGetValFlt(pArg3, TypeCode3);
            if (((Arg1 == OPARR_MIN) && (fvaltmp < fval)) ||
                ((Arg1 == OPARR_MAX) && (fvaltmp > fval)))
              fval = fvaltmp;
          }
          cCmdSetValFlt(pArg2, TypeCode2, fval);
        }
        else if (IS_SIGNED_TYPE(TypeCode2))
        {
          if (Arg1 == OPARR_MIN)
            sval = LONG_MAX;
          else
            sval = LONG_MIN;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            svaltmp = (SLONG)cCmdGetVal(pArg3, TypeCode3);
            if (((Arg1 == OPARR_MIN) && (svaltmp < sval)) ||
                ((Arg1 == OPARR_MAX) && (svaltmp > sval)))
              sval = svaltmp;
          }
          cCmdSetVal(pArg2, TypeCode2, sval);
        }
        else
        {
          if (Arg1 == OPARR_MIN)
            uval = ULONG_MAX;
          else
            uval = 0;
          for (i = 0; i < MinCount; i++)
          {
            pArg3 = cCmdResolveDataArg(INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i), NULL);
            uvaltmp = cCmdGetVal(pArg3, TypeCode3);
            if (((Arg1 == OPARR_MIN) && (uvaltmp < uval)) ||
                ((Arg1 == OPARR_MAX) && (uvaltmp > uval)))
              uval = uvaltmp;
          }
          cCmdSetVal(pArg2, TypeCode2, uval);
        }
      }
      else if (Arg1 == OPARR_SORT)
      {
        //Allocate Dst array
        Status = cCmdDSArrayAlloc(Arg2, 0, MinCount);
        if (IS_ERR(Status))
          return Status;

        DVIndex2 = cCmdGetDVIndex(Arg2, 0);

        //Move src subset to dst
        for (i = 0; i < MinCount; i++)
        {
          Status = cCmdInterpPolyUnop2(OP_MOV, INC_ID(Arg2), ARRAY_ELEM_OFFSET(DVIndex2, i), INC_ID(Arg3), ARRAY_ELEM_OFFSET(DVIndex3, ArgVal4 + i));
          if (IS_ERR(Status))
            return Status;
        }
        // now dst is ready to be sorted
        pArg2 = cCmdResolveDataArg(Arg2, 0, NULL);
        Size = cCmdSizeOf(TypeCode2);
        if (TypeCode2 == TC_SBYTE)
          shell_sort_s1(pArg2, MinCount);
        else if (TypeCode2 == TC_SWORD)
          shell_sort_s2(pArg2, MinCount);
        else if (TypeCode2 == TC_SLONG)
          shell_sort_s4(pArg2, MinCount);
        else if (TypeCode2 == TC_UBYTE)
          shell_sort_u1(pArg2, MinCount);
        else if (TypeCode2 == TC_UWORD)
          shell_sort_u2(pArg2, MinCount);
        else if (TypeCode2 == TC_ULONG)
          shell_sort_u4(pArg2, MinCount);
        else if (TypeCode2 == TC_FLOAT)
          shell_sort_flt(pArg2, MinCount);
      }
      else if (Arg1 == OPARR_TOUPPER || Arg1 == OPARR_TOLOWER)
      {
        NXT_ASSERT((cCmdDSType(Arg3) == TC_ARRAY) && (cCmdDSType(INC_ID(Arg3)) == TC_UBYTE));
        //Allocate Dst array
        Status = cCmdDSArrayAlloc(Arg2, 0, MinCount);
        if (IS_ERR(Status))
          return Status;

        UBYTE *pDst = cCmdResolveDataArg(Arg2, 0, NULL);
        UBYTE *pSrc = cCmdResolveDataArg(Arg3, 0, NULL);

        //Move src to dst
        for (i = 0; i < ArrayCount3; i++)
        {
          UBYTE ch = *pSrc;
          if ((i >= ArgVal4) && (i <= ArgVal4+MinCount))
          {
            if ((Arg1 == OPARR_TOUPPER) && (ch >= 'a') && (ch <= 'z'))
              ch -= 0x20;
            else if ((Arg1 == OPARR_TOLOWER) && (ch >= 'A') && (ch <= 'Z'))
              ch += 0x20;
          }
          *pDst = ch;
          pDst++;
          pSrc++;
        }
      }
      else
      {
        //Fatal error: Unrecognized instruction
        NXT_BREAK;
        Status = ERR_INSTR;
      }
    }
    break;

    case OP_MULDIV:
    {
      //Arg1 - Dst (scalar)
      //Arg2 - SrcA (scalar)
      //Arg3 - SrcB (scalar)
      //Arg4 - SrcC (scalar)

      Arg1 = pCode[1];
      Arg2 = pCode[2];
      Arg3 = pCode[3];
      Arg4 = pCode[4];
      ArgVal2 = cCmdGetScalarValFromDataArg(Arg2, 0);
      ArgVal3 = cCmdGetScalarValFromDataArg(Arg3, 0);
      ArgVal4 = cCmdGetScalarValFromDataArg(Arg4, 0);
      ArgVal3 = (ULONG)(((long long)ArgVal2*(long long)ArgVal3)/(long long)ArgVal4);
      pArg1 = cCmdResolveDataArg(Arg1, 0, &TypeCode1);
      cCmdSetVal(pArg1, TypeCode1, ArgVal3);
    }
    break;

/*
    case OP_PRINTF:
    {
      // Arg1 - Instruction Size in bytes
      // Arg2 - Dst
      // Arg3 - Fmtstr
      // Arg4-N - Srcs (max args = 8)
      void *srcPtrs[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
      void *pArg2 = NULL, *pArg3 = NULL;

      Arg2 = pCode[2];
      Arg3 = pCode[3];

      //Make sure Dst arg is a string
      NXT_ASSERT(cCmdDSType(Arg2) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg2)) == TC_UBYTE);

      //Make sure Fmtstr arg is a string
      NXT_ASSERT(cCmdDSType(Arg3) == TC_ARRAY);
      NXT_ASSERT(cCmdDSType(INC_ID(Arg3)) == TC_UBYTE);

      //Number of Srcs = total code words - 4 (account for opcode word, size, Dst, and Fmtstr)
      //!!! Argument access like this is potentially unsafe.
      //A function/macro which checks proper encoding would be better
      SrcCount = (pCode[1] / 2) - 4;
      if (SrcCount > 8) {
        Status = ERR_INSTR;
        return (Status);
      }

      // get pointers to Dst and FmtSt
      pArg2 = cCmdResolveDataArg(Arg2, 0, &TypeCode2);
      pArg3 = cCmdResolveDataArg(Arg3, 0, &TypeCode3);

      // resolve src pointers for all our sources
      for (i = 0; i < SrcCount; i++)
      {
        TmpDSID = pCode[4 + i];
        TYPE_CODE tc = cCmdDSType(TmpDSID);
        if ((tc == TC_ARRAY && cCmdDSType(INC_ID(TmpDSID)) != TC_UBYTE) ||
            (tc == TC_VOID) || (tc > TC_LAST_INT_SCALAR && tc != TC_FLOAT))
        {
          // invalid source (only scalars, floats, and strings are supported)
          Status = ERR_INSTR;
          return (Status);
        }
        srcPtrs[i] = cCmdResolveDataArg(TmpDSID, 0, &TypeCode1);
      }
      
      //Calculate Dst array count
      ArrayCount2 = sprintf(NULL, pArg3, srcPtrs[0], srcPtrs[1], srcPtrs[2], 
                                         srcPtrs[3], srcPtrs[4], srcPtrs[5], 
                                         srcPtrs[6], srcPtrs[7], srcPtrs[8]);
      }
      break;
*/  
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

  INPUTSTRUCT * pInput = &(pMapInput->Inputs[Port]);

  //If port is not configured properly ahead of time, report that error
  //!!! This seems like the right policy, but may restrict otherwise valid read operations...
  if (!(pInput->SensorType == LOWSPEED_9V || pInput->SensorType == LOWSPEED)
   || !(pInput->InvalidData == FALSE))
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
  
  LSBUF * pInBuf = &(pMapLowSpeed->InBuf[Port]);

  //Normally, bytes available is a simple difference.
  Tmp = pInBuf->InPtr - pInBuf->OutPtr;

  //If InPtr is actually behind OutPtr, circular buffer has wrapped.  Account for wrappage...
  if (Tmp < 0)
    Tmp = (pInBuf->InPtr + (SIZE_OF_LSBUF - pInBuf->OutPtr));
  else if ((Tmp == 0) && 
           (pInBuf->BytesToRx == SIZE_OF_LSBUF) && 
           (pMapLowSpeed->ChannelState[Port] == LOWSPEED_IDLE))
    Tmp = SIZE_OF_LSBUF;

  return (UBYTE)(Tmp);
}

//cCmdLSWrite
//Write BufLength bytes into specified port's lowspeed buffer and kick off comm process to device
NXT_STATUS cCmdLSWrite(UBYTE Port, UBYTE BufLength, UBYTE *pBuf, UBYTE ResponseLength, UBYTE NoRestartOnRead, UBYTE bFast)
{
  if (Port >= NO_OF_LOWSPEED_COM_CHANNEL)
  {
    return (ERR_COMM_CHAN_INVALID);
  }

  if (BufLength > SIZE_OF_LSBUF || ResponseLength > SIZE_OF_LSBUF)
  {
    return (ERR_INVALID_SIZE);
  }

  INPUTSTRUCT * pInput = &(pMapInput->Inputs[Port]);
  UBYTE * pChState = &(pMapLowSpeed->ChannelState[Port]);
  LSBUF * pOutBuf = &(pMapLowSpeed->OutBuf[Port]);

  //Only start writing process if port is properly configured and c_lowspeed module is ready
  if ((pInput->SensorType == LOWSPEED_9V || pInput->SensorType == LOWSPEED)
   && (pInput->InvalidData == FALSE)
   && (LOWSPEED_IDLE == *pChState) || (LOWSPEED_ERROR == *pChState))
  {
    pOutBuf->InPtr = 0;
    pOutBuf->OutPtr = 0;

    memcpy(pOutBuf->Buf, pBuf, BufLength);
    pOutBuf->InPtr = (UBYTE)BufLength;

    pMapLowSpeed->InBuf[Port].BytesToRx = ResponseLength;

    *pChState = LOWSPEED_INIT;
    pMapLowSpeed->State |= (COM_CHANNEL_ONE_ACTIVE << Port);
    if (NoRestartOnRead)
      pMapLowSpeed->NoRestartOnRead |= (COM_CHANNEL_NO_RESTART_1 << Port);
    else
      pMapLowSpeed->NoRestartOnRead &= ~(COM_CHANNEL_NO_RESTART_1 << Port);
    if (bFast)
      pMapLowSpeed->Speed |= (COM_CHANNEL_ONE_FAST << Port);
    else
      pMapLowSpeed->Speed &= ~(COM_CHANNEL_ONE_FAST << Port);

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

  LSBUF * pInBuf = &(pMapLowSpeed->InBuf[Port]);

  //If the bytes we want to read wrap around the end, we must first read the end, then reset back to the beginning
  if (pInBuf->OutPtr + BytesToRead >= SIZE_OF_LSBUF)
  {
    BytesToRead = SIZE_OF_LSBUF - pInBuf->OutPtr;
    memcpy(pBuf, pInBuf->Buf + pInBuf->OutPtr, BytesToRead);
    pInBuf->OutPtr = 0;
    pBuf += BytesToRead;
    BytesToRead = BufLength - BytesToRead;
  }
  if (BytesToRead > 0) {
    memcpy(pBuf, pInBuf->Buf + pInBuf->OutPtr, BytesToRead);
    pInBuf->OutPtr += BytesToRead;
  }

  return (NO_ERR);
}


//
//Wrappers for OP_SYSCALL
//

NXT_STATUS cCmdWrapFileOpenReadHelper(UBYTE Cmd, UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(Cmd, ArgV[2], NULL, (ULONG *)ArgV[3]);

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

NXT_STATUS cCmdWrapFileOpenWriteHelper(UBYTE Cmd, UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  DV_INDEX DVIndex;

  //Resolve array argument
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  ArgV[2] = cCmdDVPtr(DVIndex);

  LStatus = pMapLoader->pFunc(Cmd, ArgV[2], NULL, (ULONG *)ArgV[3]);

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

//
//cCmdWrapFileOpenRead
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenRead(UBYTE * ArgV[])
{
  return cCmdWrapFileOpenReadHelper(OPENREAD, ArgV);
}

//cCmdWrapFileOpenWrite
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenWrite(UBYTE * ArgV[])
{
  return cCmdWrapFileOpenWriteHelper(OPENWRITEDATA, ArgV);
}

//cCmdWrapFileOpenAppend
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length Remaining, U32 return
NXT_STATUS cCmdWrapFileOpenAppend(UBYTE * ArgV[])
{
  return cCmdWrapFileOpenWriteHelper(OPENAPPENDDATA, ArgV);
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
  UBYTE sndVol= *(ArgV[3]);
  ArgV[1] = cCmdDVPtr(DVIndex);

  //!!! Should check filename and/or existence and return error before proceeding
  strncpy((PSZ)(pMapSound->SoundFilename), (PSZ)(ArgV[1]), FILENAME_LENGTH);

  if (*(ArgV[2]) == TRUE)
    pMapSound->Mode = SOUND_LOOP;
  else
    pMapSound->Mode = SOUND_ONCE;

  if(sndVol > 4)
    sndVol= 4;
  pMapSound->Volume = sndVol;
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
  UBYTE sndVol= *(ArgV[4]);
  pMapSound->Freq = *(UWORD*)(ArgV[1]);
  pMapSound->Duration = *(UWORD*)(ArgV[2]);
  if(sndVol > 4)
    sndVol= 4;
  pMapSound->Volume = sndVol;
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
  UBYTE Port = (*(ArgV[1]) & 0x03); // 0..3 are valid port numbers
  UBYTE * pBuf;
  UWORD BufLength;
  UBYTE ResponseLength = *(ArgV[3]);
  UBYTE NoRestartOnRead = (*(ArgV[1]) & 0x04);
  UBYTE bFast = (*(ArgV[1]) & 0x08);
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  pBuf = cCmdDVPtr(DVIndex);
  BufLength = DV_ARRAY[DVIndex].Count;

  *pReturnVal = cCmdLSWrite(Port, (UBYTE)BufLength, pBuf, ResponseLength, NoRestartOnRead, bFast);
  if (bFast && (*pReturnVal == NO_ERR))
    *pReturnVal = pMapLowSpeed->pFunc(Port);
  if (*pReturnVal >= NO_ERR)
    *pReturnVal = NO_ERR; // returning a positive value causes problems in NXC API code that expects 0 (success) or non-zero error

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



//
//cCmdWrapColorSensorRead
//ArgV[0]: (return) Error code, SBYTE
//ArgV[1]: Port, UBYTE
//ArgV[2]: SensorValue, SWORD
//ArgV[3]: RawArray, UWORD[NO_OF_COLORS]
//ArgV[4]: NormalizedArray, UWORD[NO_OF_COLORS]
//ArgV[5]: ScaledArray, SWORD[NO_OF_COLORS]
//ArgV[6]: InvalidData, UBYTE
//
NXT_STATUS cCmdWrapColorSensorRead (UBYTE * ArgV[])
{
	DV_INDEX DVIndex;
        NXT_STATUS Status = NO_ERR;
	//Resolve return val arguments
	SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
	//Resolve Port argument
	UBYTE Port = *(UBYTE*)(ArgV[1]);
	//Resolve SensorValue
	SWORD SensorValue = *(SWORD*)(ArgV[2]);
	//Resolve RawArray as array
	DVIndex = *(DV_INDEX*)(ArgV[3]);
        NXT_ASSERT(IS_DV_INDEX_SANE(DestDVIndex));
        Status= cCmdDVArrayAlloc(DVIndex, NO_OF_COLORS);
        if (IS_ERR(Status))
          return (Status);
	ArgV[3] = cCmdDVPtr (DVIndex);
	//Resolve NormalizedArray as array
	DVIndex = *(DV_INDEX*)(ArgV[4]);
        NXT_ASSERT(IS_DV_INDEX_SANE(DestDVIndex));
        Status= cCmdDVArrayAlloc(DVIndex, NO_OF_COLORS);
        if (IS_ERR(Status))
          return (Status);
	ArgV[4] = cCmdDVPtr (DVIndex);
	//Resolve ScaledArray as array
	DVIndex = *(DV_INDEX*)(ArgV[5]);
        NXT_ASSERT(IS_DV_INDEX_SANE(DestDVIndex));
        Status= cCmdDVArrayAlloc(DVIndex, NO_OF_COLORS);
	if (IS_ERR(Status))
          return (Status);
        ArgV[5] = cCmdDVPtr (DVIndex);
	//Resolve InvalidData
	UBYTE InvalidData = *(UBYTE*)(ArgV[6]);

	//call implementation with unwrapped parameters
	*pReturnVal = cCmdColorSensorRead (Port, &SensorValue, (UWORD*)ArgV[3], (UWORD*)ArgV[4], (SWORD*)ArgV[5], &InvalidData);

	*(ArgV[2]) = SensorValue;
        *(ArgV[6]) = InvalidData;

        if (IS_ERR(*pReturnVal)){
		return (*pReturnVal);
	}
	return NO_ERR;
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
//ArgV[1]: Buffer
//ArgV[2]: Count to read
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



#define MAX_IOM_BUFFER_SIZE 800
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


/////////////////////////////////////////////////////////////
// Dymanic syscall implementations
////////////////////////////////////////////////////////////

//
//cCmdWrapDatalogWrite
//ArgV[0]: (return) Error Code, SBYTE (NXT_STATUS)
//ArgV[1]: Message, CStr
//
NXT_STATUS cCmdWrapDatalogWrite(UBYTE * ArgV[])
{
#ifndef STRIPPED
  NXT_STATUS Status = NO_ERR;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  ArgV[1] = cCmdDVPtr(DVIndex);

  Status = cCmdDatalogWrite(ArgV[1], DV_ARRAY[DVIndex].Count);

  *(SBYTE *)(ArgV[0]) = Status;

  if (IS_FATAL(Status))
    return Status;
  else
    return (NO_ERR);
#else
  return (NO_ERR);
#endif
}

//
//cCmdWrapDatalogGetTimes
//ArgV[0]: SyncTime, U32
//ArgV[1]: SyncTick, U32
//
NXT_STATUS cCmdWrapDatalogGetTimes(UBYTE * ArgV[])
{
#ifndef STRIPPED
  *((ULONG *)ArgV[1]) = IOMapCmd.SyncTime;
  *((ULONG *)ArgV[2]) = IOMapCmd.SyncTick;
#endif
  return (NO_ERR);
}

//
//cCmdWrapSetSleepTimeout
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: desired timer limit in ms, ULONG
//
NXT_STATUS cCmdWrapSetSleepTimeout(UBYTE * ArgV[])
{
  ULONG value = *(ULONG*)(ArgV[1]);
  if(value==0)
  {
    pMapUi->SleepTimeout=0;
  }
  else if(value < 60000)
  {
    pMapUi->SleepTimeout=1;  //integer math would've made this zero
  }
  else
  {
    pMapUi->SleepTimeout= value / 60000;
  }
  return (NO_ERR);
}

//
//cCmdWrapCommBTOnOff
//ArgV[0]: (return) Status byte, SBYTE // JCH - this should be UWORD
//ArgV[1]: Power State, 0-1
//
NXT_STATUS cCmdWrapCommBTOnOff(UBYTE * ArgV[])
{
  UWORD retVal;
  UWORD status;
  UWORD * pReturnVal = (UWORD*)(ArgV[0]);

  UBYTE powerState = *(ArgV[1]);
  if(powerState)
    status= pMapComm->pFunc(BTON, 0, 0, 0, NULL, &retVal);
  else
    status= pMapComm->pFunc(BTOFF, 0, 0, 0, NULL, &retVal);

  *pReturnVal= (status == SUCCESS) ? retVal : status;
  return (NO_ERR);
}

//
//cCmdWrapCommBTConnection
//ArgV[0]: (return) Status byte, SBYTE // JCH - this should be UWORD
//ArgV[1]: Action, UBYTE
//ArgV[2]: name, UBYTE array CStr
//ArgV[3]: connection slot, UBYTE
//
NXT_STATUS cCmdWrapCommBTConnection(UBYTE * ArgV[])
{
  UWORD retVal;
  UWORD status;
  UWORD * pReturnVal = (UWORD*)(ArgV[0]);
  UBYTE *nmPtr;

  UBYTE action = *(ArgV[1]);
  UBYTE connection = *(ArgV[3]);
  nmPtr = cCmdDVPtr(*(DV_INDEX *)(ArgV[2]));

  if(action) // Init
    status= pMapComm->pFunc(CONNECTBYNAME, 0, connection, 0, nmPtr, &retVal);
  else // Close
    status= pMapComm->pFunc(DISCONNECT, connection, 0, 0, NULL, &retVal);

  *pReturnVal= (status == SUCCESS) ? retVal : status;
  return (NO_ERR);
}


//
//cCmdWrapReadSemData
//ArgV[0]: return data, U8
//ArgV[1]: which (0=used, 1=request), U8
//
NXT_STATUS cCmdWrapReadSemData(UBYTE * ArgV[])
{
  if(!(*((UBYTE *)ArgV[1])))
    *((UBYTE *)ArgV[0])= gUsageSemData;
  else
    *((UBYTE *)ArgV[0])= gRequestSemData;
  return (NO_ERR);
}

//
//cCmdWrapWriteSemData
//ArgV[0]: return data, U8
//ArgV[1]: which (0=used, 1=request), U8
//ArgV[2]: newValue, U8
//ArgV[3]: action (0= OR, 1= AND), U8
//
NXT_STATUS cCmdWrapWriteSemData(UBYTE * ArgV[])
{
  UBYTE curVal, newVal, which= (*((UBYTE *)ArgV[1]));
  if(!which)
    curVal= gUsageSemData;
  else
    curVal= gRequestSemData;

  newVal= *((UBYTE *)ArgV[2]);

  if(*((UBYTE *)ArgV[3]))
    curVal &= ~newVal;
  else
    curVal |= newVal;

  if(!which)
    gUsageSemData= curVal;
  else
    gRequestSemData= curVal;
  *((UBYTE *)ArgV[0])= curVal;
  return (NO_ERR);
}


//
//cCmdWrapUpdateCalibCacheInfo
//ArgV[0]: return data, U8
//ArgV[1]: nm, UBYTE array CStr
//ArgV[2]: min, U16
//ArgV[3]: max , U16
//
NXT_STATUS cCmdWrapUpdateCalibCacheInfo(UBYTE * ArgV[])
{
  UBYTE *nm= cCmdDVPtr(*(DV_INDEX *)(ArgV[1]));
  SWORD min= (*((SWORD *)ArgV[2]));
  SWORD max= (*((SWORD *)ArgV[3]));

  cCmdUpdateCalibrationCache(nm, min, max);
  *((UBYTE *)ArgV[0])= SUCCESS;
  return (NO_ERR);
}

//
//cCmdWrapComputeCalibValue
//ArgV[0]: return data, U8
//ArgV[1]: nm, UBYTE array CStr
//ArgV[2]: raw, U16 ref in out
NXT_STATUS cCmdWrapComputeCalibValue (UBYTE * ArgV[])
{
  UBYTE *nm= cCmdDVPtr(*(DV_INDEX *)(ArgV[1]));
  SWORD raw= (*((SWORD *)ArgV[2]));

  *((UBYTE *)ArgV[0])=  cCmdComputeCalibratedValue(nm, &raw);
  (*((SWORD *)ArgV[2]))= raw;
  return (NO_ERR);
}

typedef struct {
  SWORD min, max;
  UBYTE nm[FILENAME_LENGTH + 1];
} CalibCacheType;

SBYTE gCalibCacheCnt= 0;
DV_INDEX gCalibCacheArrayDVIdx= NOT_A_DS_ID;
CalibCacheType *gCalibCacheArray= NULL;

SWORD cCmdGetCalibrationIndex(UBYTE *nm) {
  SBYTE i;
  for(i= 0; i < gCalibCacheCnt; i++)
    if(!strcmp((PSZ)nm, (PSZ)gCalibCacheArray[i].nm))
      break;
  return i;
}

NXT_STATUS cCmdComputeCalibratedValue(UBYTE *nm, SWORD *pRaw) {
  SBYTE i= cCmdGetCalibrationIndex(nm);
  NXT_STATUS status= ERR_RC_ILLEGAL_VAL;
  SLONG raw= *pRaw, range;
  if(i < gCalibCacheCnt) {
    status= SUCCESS;
    raw -= gCalibCacheArray[i].min;
    range= (gCalibCacheArray[i].max - gCalibCacheArray[i].min);
  }
  else
    range= 1023;
  raw *= 100;
  raw /= range;
  if(raw < 0) raw= 0;
  else if(raw > 100) raw= 100;
  *pRaw= raw;
  return status;
}


NXT_STATUS ResizeCalibCache(ULONG elements) { // alloc dv if needed, grow if needed. dv never freed. on boot, set to NOT_A_DS_ID. use cnt for valid elements.
  NXT_STATUS Status = NO_ERR;

  if(gCalibCacheArrayDVIdx == NOT_A_DS_ID)
    Status = cCmdAllocDopeVector(&gCalibCacheArrayDVIdx, sizeof(CalibCacheType));
  if(!IS_ERR(Status) && DV_ARRAY[gCalibCacheArrayDVIdx].Count < elements) //Allocate storage for cache element
    Status = cCmdDVArrayAlloc(gCalibCacheArrayDVIdx, elements);
  if(!IS_ERR(Status))
    gCalibCacheArray= cCmdDVPtr(gCalibCacheArrayDVIdx);
  // on error, does old DVIdx still point to array, or should we null out array???
  return Status;
}

// called to update min/max on existing cache element, and to add new named element
void cCmdUpdateCalibrationCache(UBYTE *nm, SWORD min, SWORD max) {
  SWORD i= cCmdGetCalibrationIndex(nm);
  NXT_STATUS Status = NO_ERR;

  if(i == gCalibCacheCnt) { // sensor wasn't found, insert into cache
    Status= ResizeCalibCache(gCalibCacheCnt+1);
    if(!IS_ERR(Status)) {
      gCalibCacheCnt++;
      strcpy((PSZ)gCalibCacheArray[i].nm, (PSZ)nm);
    }
  }
  if(!IS_ERR(Status)) {
    gCalibCacheArray[i].min= min;
    gCalibCacheArray[i].max= max;
  }
}

void cCmdLoadCalibrationFiles(void) {
  ULONG cnt, DataSize;
  UBYTE nm[FILENAME_LENGTH + 1], nmLen;
  SWORD Handle, HandleSearch;
  gCalibCacheCnt= 0;
  gCalibCacheArrayDVIdx= NOT_A_DS_ID;
  // file I/O to load all .cal files into cached globals used by scaling syscall
  HandleSearch = pMapLoader->pFunc(FINDFIRST, "*.cal", nm, &cnt); // returns total files and nm of first one
  while (LOADER_ERR(HandleSearch) == SUCCESS) { // if we have a file, process it by closing and opening
    SWORD min= 0, max= 0, tmp;
    ULONG length;
    pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(HandleSearch), NULL, NULL);
    Handle = pMapLoader->pFunc(OPENREAD, nm, NULL, &DataSize);
    if (LOADER_ERR(Handle) == SUCCESS && DataSize == 4) {
      // access data, two bytes for min and two for max
      length= 2;
      pMapLoader->pFunc(READ,LOADER_HANDLE_P(Handle),(UBYTE*)&tmp,&length);
      if (length == 2)
        min= tmp;
      length= 2;
      pMapLoader->pFunc(READ,LOADER_HANDLE_P(Handle),(UBYTE*)&tmp,&length);
      if (length == 2)
        max= tmp;
    }
    pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(Handle), NULL, NULL);
    // update calibration cache with nm, min, and max
    nmLen= strlen((PSZ)nm) - 4; // chop off .cal extension
    nm[nmLen]= 0;
    cCmdUpdateCalibrationCache(nm, min, max);

    HandleSearch = pMapLoader->pFunc(FINDNEXT, LOADER_HANDLE_P(HandleSearch), nm, &cnt);
  }
  pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(HandleSearch), NULL, NULL);
}

//
//cCmdWrapListFiles
//ArgV[0]: return data, SBYTE
//ArgV[1]: pattern, UBYTE array CStr
//ArgV[2]: list, UBYTE array CStr array ref in out
NXT_STATUS cCmdWrapListFiles (UBYTE * ArgV[])
{
  ULONG fileSize, matchCount=0, i=0, oldCount;
  SWORD HandleSearch;
  NXT_STATUS Status = NO_ERR;
  DV_INDEX listIdx, *list;
  UBYTE *strTemp, *pattern;
  UBYTE name[FILENAME_LENGTH + 1];

  //Resolve array arguments
  pattern = cCmdDVPtr(*(DV_INDEX *)(ArgV[1]));
  listIdx = *(DV_INDEX *)(ArgV[2]);

  HandleSearch = pMapLoader->pFunc(FINDFIRST, pattern, name, &fileSize); // returns first file matching pattern

  //Count how many files we're going to have
  while (LOADER_ERR(HandleSearch) == SUCCESS)
  {
    matchCount++;
    pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(HandleSearch), NULL, NULL);
    HandleSearch = pMapLoader->pFunc(FINDNEXT, LOADER_HANDLE_P(HandleSearch), name, &fileSize);
  }

  HandleSearch = pMapLoader->pFunc(FINDFIRST, pattern, name, &fileSize); // returns first file matching pattern

  oldCount = DV_ARRAY[listIdx].Count;  // Check to see how many dope vectors are already in the array (if they passed us a non-blank array of strings)

  Status = cCmdDVArrayAlloc(listIdx, matchCount);  // Size the top-level array
  if(IS_ERR(Status))
     return Status;

  list = (DV_INDEX*)(VarsCmd.pDataspace + DV_ARRAY[listIdx].Offset); // Get a pointer into the dataspace for the array of DV_INDEXes

  while (LOADER_ERR(HandleSearch) == SUCCESS && !IS_ERR(Status))
  {
      pMapLoader->pFunc(CLOSE, LOADER_HANDLE_P(HandleSearch), NULL, NULL);   // Close the handle that we automatically opened above
      // Allocate a new dope vector if one doesn't already exist
      if(i >= oldCount)
        Status = cCmdAllocDopeVector(&(list[i]), sizeof(char));

      // Allocate the string buffer for output array[i]
      if(!IS_ERR(Status))
        Status = cCmdDVArrayAlloc(list[i], strlen((PSZ)name) + 1);

      if(!IS_ERR(Status))
      {
        strTemp = VarsCmd.pDataspace + DV_ARRAY[list[i]].Offset; // Get a pointer into the dataspace for this string
        strcpy((PSZ)strTemp, (PSZ)name);
      }
      i++;

      HandleSearch = pMapLoader->pFunc(FINDNEXT, LOADER_HANDLE_P(HandleSearch), name, &fileSize);
  }

  *(SBYTE *)(ArgV[0]) = Status;

  return Status;
}

//
//cCmdWrapCommExecuteFunction
//ArgV[0]: (return) Result word, UWORD
//ArgV[1]: UBYTE Cmd
//ArgV[2]: UBYTE Param1
//ArgV[3]: UBYTE Param2
//ArgV[4]: UBYTE Param3
//ArgV[5]: Name, UBYTE array
//ArgV[6]: UWORD RetVal
//
NXT_STATUS cCmdWrapCommExecuteFunction(UBYTE * ArgV[])
{
  // resolve Name
  ArgV[5] = cCmdDVPtr(*(DV_INDEX *)(ArgV[5]));
  
  *(UWORD*)(ArgV[0]) = 
     pMapComm->pFunc(*(UBYTE*)(ArgV[1]), 
                     *(UBYTE*)(ArgV[2]),
                     *(UBYTE*)(ArgV[3]),
                     *(UBYTE*)(ArgV[4]),
                     (UBYTE*)(ArgV[5]),
                     (UWORD*)(ArgV[6]));
  return (NO_ERR);
}

//
//cCmdWrapLoaderExecuteFunction
//ArgV[0]: (return) Result word, UWORD
//ArgV[1]: UBYTE Cmd
//ArgV[2]: FileName, UBYTE array
//ArgV[3]: Buffer, UBYTE array
//ArgV[4]: ULONG pLength
//
NXT_STATUS cCmdWrapLoaderExecuteFunction(UBYTE * ArgV[])
{
  // resolve FileName
  ArgV[2] = cCmdDVPtr(*(DV_INDEX *)(ArgV[2]));
  // resolve Buffer
  ArgV[3] = cCmdDVPtr(*(DV_INDEX *)(ArgV[3]));
  
  *(UWORD*)(ArgV[0]) = 
     pMapLoader->pFunc(*(UBYTE*)(ArgV[1]), 
                     (UBYTE*)(ArgV[2]),
                     (UBYTE*)(ArgV[3]),
                     (ULONG*)(ArgV[4]));
  return (NO_ERR);
}

//
//cCmdWrapIOMapReadByID
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: ModuleID, ULONG
//ArgV[2]: Offset, UWORD
//ArgV[3]: Count, UWORD
//ArgV[4]: Buffer, UBYTE array
//
NXT_STATUS cCmdWrapIOMapReadByID(UBYTE * ArgV[])
{
  UWORD LStatus;
  NXT_STATUS Status;

  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UWORD Offset = *(UWORD*)(ArgV[2]);
  //Our copy of 'Count' must be a ULONG to match the loader interface
  ULONG Count = *(UWORD*)(ArgV[3]);
  ULONG ModuleID = *(ULONG*)ArgV[1];

  DV_INDEX DVIndex;

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

  //Module was found, transfer Offset into first two bytes of DataBuffer and attempt to read
  *(UWORD*)(DataBuffer) = Offset;
  LStatus = pMapLoader->pFunc(IOMAPREAD, (UBYTE *)&ModuleID, DataBuffer, &Count);

  if (LOADER_ERR(LStatus) == SUCCESS)
  {
    //No error from IOMAPREAD, so copy the data into VM's dataspace
    //Size destination array
    DVIndex = *(DV_INDEX *)(ArgV[4]);
    Status = cCmdDVArrayAlloc(DVIndex, (UWORD)Count);
    if (IS_ERR(Status))
    {
      //Alloc failed, so return
      return (Status);
    }

    //Alloc succeeded, so resolve and copy away
    ArgV[4] = cCmdDVPtr(DVIndex);
    memcpy(ArgV[4], &(DataBuffer[2]), Count);
  }

  *pReturnVal = LOADER_ERR_BYTE(LStatus);

  return (NO_ERR);
}

//
//cCmdWrapIOMapWriteByID
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: ModuleID, ULONG
//ArgV[2]: Offset, UWORD
//ArgV[3]: Buffer, UBYTE array
//
NXT_STATUS cCmdWrapIOMapWriteByID(UBYTE * ArgV[])
{
  UWORD LStatus;

  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UWORD Offset = *(UWORD*)(ArgV[2]);
  ULONG ModuleID = *(ULONG*)ArgV[1];

  //Our copy of 'Count' must be a ULONG to match the loader interface
  ULONG Count;
  DV_INDEX DVIndex;

  //Buffer to store data and offset in for IOMAPREAD call
  //!!! Constant size means only limited reads and writes
  UBYTE DataBuffer[MAX_IOM_BUFFER_SIZE + 2];

  //Resolve buffer
  DVIndex = *(DV_INDEX *)(ArgV[3]);
  ArgV[3] = cCmdDVPtr(DVIndex);
  Count = DV_ARRAY[DVIndex].Count;

  if (Count > MAX_IOM_BUFFER_SIZE)
  {
    //Request to read too much data at once; return error and give up
    *pReturnVal = ERR_INVALID_SIZE;
    return (NO_ERR);
  }

  //Module was found, transfer Offset into first two bytes of DataBuffer, copy data into rest of buffer, then write
  *(UWORD*)(DataBuffer) = Offset;
  memcpy(&(DataBuffer[2]), ArgV[3], Count);
  LStatus = pMapLoader->pFunc(IOMAPWRITE, (UBYTE *)&ModuleID, DataBuffer, &Count);

  *pReturnVal = LOADER_ERR_BYTE(LStatus);

  return (NO_ERR);
}

/*
NXT_STATUS cCmdWrapFileFindHelper(UBYTE First, UBYTE * ArgV[])
{
  LOADER_STATUS LStatus;
  NXT_STATUS Status;
  DV_INDEX DVIndex;
  UBYTE LoaderCmd = FINDNEXT;

  UBYTE FileMask[FILENAME_LENGTH+1];

  //Resolve array arguments
  // input mask/output filename
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  if (First) {
    LoaderCmd = FINDFIRST;
    ArgV[1] = FileMask;
    memcpy(FileMask, cCmdDVPtr(DVIndex), DV_ARRAY[DVIndex].Count);
  }
  //Size Buffer to Length
  //Add room for null terminator to length
  Status = cCmdDVArrayAlloc(DVIndex, (UWORD)(FILENAME_LENGTH + 1));
  if (IS_ERR(Status))
    return Status;
  ArgV[2] = cCmdDVPtr(DVIndex);
  
  LStatus = pMapLoader->pFunc(LoaderCmd, ArgV[1], ArgV[2], (ULONG *)ArgV[3]);
  
  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return (NO_ERR);
}
*/
//cCmdWrapFileFindFirst
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 out
//ArgV[2]: Filename, CStr in/out
//ArgV[3]: Length, U32 out
NXT_STATUS cCmdWrapFileFindFirst(UBYTE * ArgV[])
{
//  return cCmdWrapFileFindHelper(TRUE, ArgV);
  LOADER_STATUS LStatus;
  NXT_STATUS Status;
  DV_INDEX DVIndex;

  UBYTE FileMask[FILENAME_LENGTH+1];

  //Resolve array arguments
  // input mask/output filename
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  memcpy(FileMask, cCmdDVPtr(DVIndex), DV_ARRAY[DVIndex].Count);
  //Size Buffer to Length
  //Add room for null terminator to length
  Status = cCmdDVArrayAlloc(DVIndex, (UWORD)(FILENAME_LENGTH + 1));
  if (IS_ERR(Status))
    return Status;
  ArgV[2] = cCmdDVPtr(DVIndex);
  
  LStatus = pMapLoader->pFunc(FINDFIRST, FileMask, ArgV[2], (ULONG *)ArgV[3]);
  
  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return (NO_ERR);
}

//cCmdWrapFileFindNext
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 in/out
//ArgV[2]: Filename, CStr out
//ArgV[3]: Length, U32 out
NXT_STATUS cCmdWrapFileFindNext(UBYTE * ArgV[])
{
//  return cCmdWrapFileFindHelper(FALSE, ArgV);
  LOADER_STATUS LStatus;
  NXT_STATUS Status;
  DV_INDEX DVIndex;

  //Resolve array arguments
  // output filename
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  //Size Buffer to Length
  //Add room for null terminator to length
  Status = cCmdDVArrayAlloc(DVIndex, (UWORD)(FILENAME_LENGTH + 1));
  if (IS_ERR(Status))
    return Status;
  ArgV[2] = cCmdDVPtr(DVIndex);
  
  LStatus = pMapLoader->pFunc(FINDNEXT, ArgV[1], ArgV[2], (ULONG *)ArgV[3]);
  
  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);

  return (NO_ERR);
}

//cCmdWrapFileOpenReadLinear
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenReadLinear(UBYTE * ArgV[])
{
  return cCmdWrapFileOpenReadHelper(OPENREADLINEAR, ArgV);
}

//cCmdWrapFileOpenWriteLinear
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenWriteLinear(UBYTE * ArgV[])
{
  return cCmdWrapFileOpenWriteHelper(OPENWRITELINEAR, ArgV);
}

//cCmdWrapFileOpenWriteNonLinear
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 return
//ArgV[2]: Filename, CStr
//ArgV[3]: Length, U32 return
NXT_STATUS cCmdWrapFileOpenWriteNonLinear(UBYTE * ArgV[])
{
  return cCmdWrapFileOpenWriteHelper(OPENWRITE, ArgV);
}

//
//cCmdWrapCommHSControl
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Command, UBYTE (init, uart, or exit)
//ArgV[2]: BaudRate, UBYTE
//ArgV[3]: Mode, UWORD
NXT_STATUS cCmdWrapCommHSControl(UBYTE * ArgV[])
{
  pMapComm->HsInBuf.InPtr = 0;
  pMapComm->HsInBuf.OutPtr = 0;
  pMapComm->HsOutBuf.InPtr = 0;
  pMapComm->HsOutBuf.OutPtr = 0;
  switch (*(ArgV[1])) 
  {
    case HS_CTRL_INIT:
    {
      // hi-speed enable/init
      pMapComm->HsState = HS_ENABLE;
      pMapComm->HsFlags = HS_UPDATE;
    }
    break;

    case HS_CTRL_UART:
    {
      // hi-speed setup uart
      pMapComm->HsSpeed = *(ArgV[2]);
      pMapComm->HsMode  = *(UWORD*)(ArgV[3]);
      pMapComm->HsState = HS_INITIALISE;
      pMapComm->HsFlags = HS_UPDATE;
    }
    break;
    
    case HS_CTRL_EXIT:
    {
      // hi-speed exit
      pMapComm->HsState = HS_DISABLE;
      pMapComm->HsFlags = HS_UPDATE;
    }
    break;
  }
  
  *(ArgV[0]) = pMapComm->HsState;
  
  return (NO_ERR);
}

//cCmdHSCalcBytesReady
//Calculate true number of bytes available in the inbound HS buffer
UBYTE cCmdHSCalcBytesReady()
{
  SWORD Tmp = pMapComm->HsInBuf.InPtr - pMapComm->HsInBuf.OutPtr;
  if (Tmp < 0)
    Tmp = (pMapComm->HsInBuf.InPtr + (SIZE_OF_HSBUF - pMapComm->HsInBuf.OutPtr));
  return (UBYTE)(Tmp);
}

//cCmdWrapCommHSCheckStatus
//ArgV[0]: SendingData, UBYTE out
//ArgV[1]: DataAvailable, UBYTE out
NXT_STATUS cCmdWrapCommHSCheckStatus(UBYTE * ArgV[])
{
  *(ArgV[0]) = (pMapComm->HsState > HS_BYTES_REMAINING) ? 
                 (pMapComm->HsState - HS_BYTES_REMAINING) : 
                 0;
  *(ArgV[1]) = cCmdHSCalcBytesReady();

  return (NO_ERR);
}

//cCmdWrapCommHSWrite
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Buffer
//ArgV[2]: BufferLength -- not used for Write
NXT_STATUS cCmdWrapCommHSWrite(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UBYTE * pBuf;
  UWORD BufLength;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[1]);
  pBuf = cCmdDVPtr(DVIndex);
  BufLength = DV_ARRAY[DVIndex].Count;
  
  if (BufLength > SIZE_OF_HSBUF)
  {
    *pReturnVal = ERR_INVALID_SIZE;
    return (NO_ERR);
  }
  
  // set inptr & outptr
  pMapComm->HsOutBuf.OutPtr = 0;
  pMapComm->HsOutBuf.InPtr = BufLength;
  memcpy(pMapComm->HsOutBuf.Buf, pBuf, BufLength);

  // send the data
  pMapComm->HsState = HS_SEND_DATA;
  pMapComm->HsFlags = HS_UPDATE;
  
  *pReturnVal = pMapComm->HsState;
  
  return (NO_ERR);
}

//cCmdHSRead
//Read BufLength bytes from the hispeed buffer
NXT_STATUS cCmdHSRead(UBYTE BufLength, UBYTE * pBuf)
{
  UBYTE BytesReady, BytesToRead;

  if (BufLength > SIZE_OF_HSBUF)
  {
    return (ERR_INVALID_SIZE);
  }

  BytesReady = cCmdHSCalcBytesReady();

  if (BufLength > BytesReady)
  {
    return (ERR_COMM_CHAN_NOT_READY);
  }

  BytesToRead = BufLength;

  HSBUF * pInBuf = &(pMapComm->HsInBuf);

  //If the bytes we want to read wrap around the end, we must first read the end, then reset back to the beginning
  if (pInBuf->OutPtr + BytesToRead >= SIZE_OF_HSBUF)
  {
    BytesToRead = SIZE_OF_HSBUF - pInBuf->OutPtr;
    memcpy(pBuf, pInBuf->Buf + pInBuf->OutPtr, BytesToRead);
    pInBuf->OutPtr = 0;
    pBuf += BytesToRead;
    BytesToRead = BufLength - BytesToRead;
  }
  if (BytesToRead > 0) {
    memcpy(pBuf, pInBuf->Buf + pInBuf->OutPtr, BytesToRead);
    pInBuf->OutPtr += BytesToRead;
  }

  return (NO_ERR);
}

//cCmdWrapCommHSRead
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Buffer, out
//ArgV[2]: BufferLength, UBYTE, specifies size of buffer requested
NXT_STATUS cCmdWrapCommHSRead(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  DV_INDEX DVIndex = *(DV_INDEX *)(ArgV[1]);
  UBYTE BufLength = *(ArgV[2]);
  UBYTE BytesToRead;
  NXT_STATUS Status;
  UBYTE * pBuf;

  BytesToRead = cCmdHSCalcBytesReady();

  if (BytesToRead > 0)
  {
    //Limit buffer to available data
    if (BufLength > BytesToRead)
      BufLength = BytesToRead;

    Status = cCmdDVArrayAlloc(DVIndex, BufLength);
    if (IS_ERR(Status))
      return (Status);

    pBuf = cCmdDVPtr(DVIndex);
    *pReturnVal = cCmdHSRead(BufLength, pBuf);
  }
  else
  {
    Status = cCmdDVArrayAlloc(DVIndex, 0);
    if (IS_ERR(Status))
      return (Status);
  }

  return (NO_ERR);
}

//cCmdWrapCommLSWriteEx
//ArgV[0]: (return) Status code, SBYTE
//ArgV[1]: Port specifier, UBYTE
//ArgV[2]: Buffer to send, UBYTE array, only SIZE_OF_LSBUF bytes will be used
//ArgV[3]: ResponseLength, UBYTE, specifies expected bytes back from slave device
//ArgV[4]: Options, UBYTE, specifies whether or not to restart before the read and whether to use fast mode or not
//
NXT_STATUS cCmdWrapCommLSWriteEx(UBYTE * ArgV[])
{
/*
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UBYTE Port = *(ArgV[1]);
  UBYTE * pBuf;
  UWORD BufLength;
  UBYTE ResponseLength = *(ArgV[3]);
  UBYTE NoRestartOnRead = *(ArgV[4]) & 0x01;
  UBYTE bFast = *(ArgV[4]) & 0x02;
  DV_INDEX DVIndex;

  //Resolve array arguments
  DVIndex = *(DV_INDEX *)(ArgV[2]);
  pBuf = cCmdDVPtr(DVIndex);
  BufLength = DV_ARRAY[DVIndex].Count;

  *pReturnVal = cCmdLSWrite(Port, (UBYTE)BufLength, pBuf, ResponseLength, NoRestartOnRead, bFast);
*/
  return (NO_ERR);
}

//cCmdWrapFileSeek
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 in/out
//ArgV[2]: Origin, U8 in
//ArgV[3]: Length, S32 in
NXT_STATUS cCmdWrapFileSeek(UBYTE * ArgV[])
{
  UBYTE Origin = *((UBYTE *)ArgV[2]);
  LOADER_STATUS LStatus = pMapLoader->pFunc(Origin+SEEKFROMSTART, ArgV[1], NULL, (ULONG *)ArgV[3]);
  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);
  return (NO_ERR);
}

//cCmdWrapFileResize
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 in/out
//ArgV[2]: NewSize, U16 in
NXT_STATUS cCmdWrapFileResize(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus = pMapLoader->pFunc(RESIZEDATAFILE, ArgV[1], NULL, (ULONG *)ArgV[2]);
  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);
  return (NO_ERR);
}

//cCmdWrapMemoryManager
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Compact?, UBYTE (true or false)
//ArgV[2]: PoolSize, UWORD
//ArgV[3]: DataspaceSize, UWORD
NXT_STATUS cCmdWrapMemoryManager(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  *pReturnVal = NO_ERR;
  if (*(ArgV[1])) {
    *pReturnVal = cCmdDSCompact();  
  }
  *(UWORD*)(ArgV[2]) = (UWORD)VarsCmd.PoolSize;
  *(UWORD*)(ArgV[3]) = VarsCmd.DataspaceSize;
  
  return (NO_ERR);
}

//cCmdWrapReadLastResponse
//ArgV[0]: (return) Status byte, SBYTE
//ArgV[1]: Clear?, UBYTE (true or false)
//ArgV[2]: Length, UBYTE out
//ArgV[3]: Command, UBYTE out
//ArgV[4]: Buffer, out
NXT_STATUS cCmdWrapReadLastResponse(UBYTE * ArgV[])
{
  SBYTE * pReturnVal = (SBYTE*)(ArgV[0]);
  UWORD bufLen = 0;
  if (VarsCmd.LastResponseLength > 0)
    bufLen = VarsCmd.LastResponseLength-2;

  //Resolve array arguments
  // output buffer
  DV_INDEX DVIndex = *(DV_INDEX *)(ArgV[4]);
  //Size Buffer to Length
  NXT_STATUS Status = cCmdDVArrayAlloc(DVIndex, bufLen);
  if (IS_ERR(Status))
    return Status;
  UBYTE* pBuf = cCmdDVPtr(DVIndex);
  ArgV[4] = pBuf;
  *(ArgV[2]) = bufLen; // Length
  *pReturnVal = NO_ERR;
  
  if (bufLen > 0)
  {
    memset(pBuf, 0, bufLen);
    memcpy(pBuf, (PSZ)&(VarsCmd.LastResponseBuffer[2]), bufLen-1);
    *pReturnVal = VarsCmd.LastResponseBuffer[1];
    *(ArgV[3]) = VarsCmd.LastResponseBuffer[0];
  }
  // clear?
  if (*(ArgV[1])) {
    VarsCmd.LastResponseLength = 0;
    memset(VarsCmd.LastResponseBuffer, 0, 64);
  }
  
  return (NO_ERR);
}

//cCmdWrapFileTell
//ArgV[0]: (Function return) Loader status, U16 return
//ArgV[1]: File Handle, U8 in/out
//ArgV[2]: File Position, U32 out
NXT_STATUS cCmdWrapFileTell(UBYTE * ArgV[])
{
  LOADER_STATUS LStatus = pMapLoader->pFunc(FILEPOSITION, ArgV[1], NULL, (ULONG *)ArgV[2]);
  //Status code in high byte of LStatus
  *((UWORD *)ArgV[0]) = LOADER_ERR(LStatus);
  //File handle in low byte of LStatus
  *(ArgV[1]) = LOADER_HANDLE(LStatus);
  return (NO_ERR);
}

//
//cCmdWrapRandomEx
//ArgV[0]: Seed, SLONG (in/out)
//ArgV[1]: Reseed?, UBYTE (true or false) (in)
static SLONG __random_seed = 1;
static SLONG __old_random_seed = 1;

NXT_STATUS cCmdWrapRandomEx(UBYTE * ArgV[])
{
  SLONG * pSeed = (SLONG*)(ArgV[0]);
  if (*(ArgV[1]))
  {
    // reseed
    if (*pSeed == 0) {
      *pSeed = (SLONG)dTimerRead();
      if (*pSeed < 0)
        *pSeed = 1;
    }
    else if (*pSeed < 0)
      *pSeed = __old_random_seed;
    __random_seed = *pSeed;
    __old_random_seed = __random_seed;
  }
  else
  {
    /*
     MINSTD  
     a = 16807 (with q = 127773 and r = 2836) or 
     better randomness 
     a = 48271 (with q = 44488 and r = 3399) or 
     a = 69621 (with q = 30845 and r = 23902)
     */
#define a 48271
#define m 2147483647
#define q (m / a)
#define r (m % a)
    SLONG test = a * (__random_seed % q) - r * (__random_seed / q);
    if (test > 0)
      __random_seed = test;
    else
      __random_seed = test + m;
  }
  *pSeed = __random_seed;

  return NO_ERR;
}


//
//cCmdWrapInputPinFunction
//ArgV[0]: (return) Result word, UWORD
//ArgV[1]: UBYTE Cmd
//ArgV[2]: UBYTE Port
//ArgV[3]: UBYTE Pin
//ArgV[4]: UBYTE pData
//
NXT_STATUS cCmdWrapInputPinFunction(UBYTE * ArgV[])
{
  *(UWORD*)(ArgV[0]) = 
     pMapInput->pFunc(*(UBYTE*)(ArgV[1]), 
                      *(UBYTE*)(ArgV[2]),
                      *(UBYTE*)(ArgV[3]),
                       (UBYTE*)(ArgV[4])
                      );
  return (NO_ERR);
}


NXT_STATUS cCmdWrapUndefinedSysCall(UBYTE * ArgV[])
{
  return (NO_ERR);
}

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
#error   // CodeStart is now absolute, but not sure how to fix
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
#include "c_cmd_alternate.c"

#endif //ENABLE_VM
