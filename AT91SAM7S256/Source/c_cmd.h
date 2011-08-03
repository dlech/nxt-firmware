//
// Date init       14.12.2004
//
// Revision date   $Date: 10-07-08 13:22 $
//
// Filename        $Workfile:: c_cmd.h                                       $
//
// Version         $Revision: 8 $
//
// Archive         $Archive:: /LMS2006/Sys01/Main_V02/Firmware/Source/c_cmd. $
//
// Platform        C
//

//
// File Description:
// This file contains definitions and prototypes for the VM which runs bytecode
// programs compatible with LEGO MINDSTORMS NXT Software 1.0.
//

#ifndef   C_CMD
#define   C_CMD

//!!! MAX_HANDLES also defined in m_sched.h
#ifndef MAX_HANDLES
#define MAX_HANDLES 16
#endif

#include "c_cmd_bytecodes.h"
#define SYSCALL_COUNT 100

extern    const HEADER cCmd;

//
//Standard interface to other modules
//
void      cCmdInit(void* pHeader);
void      cCmdCtrl(void);
void      cCmdExit(void);

//
//ARM_NXT vs SIM_NXT
//These definitions are set up to allow compiling this code for use in a simulated (non-ARM7) environment.
//If your toolchain doesn't automatically use the __ICCARM__ token, define it to ensure normal compilation.
//
#ifdef __ICCARM__
#define ARM_NXT
#else
#define SIM_NXT
#endif

//
//ENABLE_VM toggles compilation the main body of VM code.
//Define it as 0 to compile alternate implementation for testing (see bottom of c_cmd.c)
//
#define ENABLE_VM 1
#undef ARM_DEBUG


//
//WRITE_IOMAP_OFFSETS enables saving a file containing accurate iomap offsets.
//
#define WRITE_IOMAP_OFFSETS 0

#if WRITE_IOMAP_OFFSETS
void cCmdWriteIOMapOffsetsFile(); 
#endif

//
//VM_BENCHMARK enables extra instrumentation code to measure VM performance.
//When enabled, a file named "benchmark.txt" is produced every time a program completes.
//
#define VM_BENCHMARK (ENABLE_VM && 0) //<-- Toggle to turn on benchmark calculations

#if VM_BENCHMARK
//Prototype for benchmark recording function
void cCmdWriteBenchmarkFile();
#endif

//
//Run-time assert macros
//Use these to test for unexpected conditions
//If expr evaluates as false while running under a debugger,
// a software breakpoint exception is thrown.
//NXT_BREAK is just a shortcut for unconditional break.
//
//Assert definitions behind WIN_DEBUG only make sense when compiling SIM_NXT
// under an x86 Windows debugger.
//
#if defined WIN_DEBUG
//"int 3" is a break exception on x86
#define NXT_ASSERT(expr) if (expr) {} else { __asm {int 3} }
#define NXT_BREAK NXT_ASSERT(0)
//
//Assert definitions behind ARM_DEBUG aren't quite as handy as WIN_DEBUG,
// but they do record the code line causing the last assert failure.
//
#elif defined(ARM_DEBUG)
#define NXT_ASSERT(expr) if (expr) {}\
                         else\
                         {\
                           VarsCmd.AssertFlag = TRUE;\
                           VarsCmd.AssertLine = __LINE__;\
                         }
#define NXT_BREAK NXT_ASSERT(0);
#else
//Not debugging, so #defined as nothing
//!!! Note that these definitions means all usages of NXT_ASSERT and NXT_BREAK
// get stripped out of an unmodified ARM7 build.
//Unless ARM_DEBUG is enabled, treat them as documentation of expected values.
#define NXT_ASSERT(expr)
#define NXT_BREAK
#endif

//
//Status byte used to return requests for further action or errors
//Valid codes #defined in c_cmd.iom
//!!!JLOFTUS Replace with NXT_STATUS?  Same for ASSERTS? Others? Risk factors?
//
typedef SBYTE NXT_STATUS;

#if ENABLE_VM

//Intial values for clump records are packed into 4 bytes in the file format.
#define VM_FILE_CLUMP_REC_SIZE 4

//
// Definitions for dataspace management, IO Map (IOM) access, and bytecode instruction structure
//

//Type codes for use in the dataspace table-of-contents (DSTOC)
typedef UBYTE TYPE_CODE;

enum
{
  //VOID type for unused DS elements; never valid to address them from bytecode
  TC_VOID,

  //Simple scalar integers, equivalent to matching basic types from stdconst.h
  TC_UBYTE,
  TC_SBYTE,
  TC_UWORD,
  TC_SWORD,
  TC_ULONG,
  TC_SLONG, TC_LAST_INT_SCALAR= TC_SLONG,

  //Aggregate types containing one or more scalar
  TC_ARRAY,
  TC_CLUSTER,

  //Mutex tracks current holder and any waiting clumps
  TC_MUTEX,
  TC_FLOAT, TC_LAST_VALID= TC_FLOAT
};

//Sizes (in bytes) of each scalar type
#define SIZE_UBYTE 1
#define SIZE_SBYTE 1
#define SIZE_UWORD 2
#define SIZE_SWORD 2
#define SIZE_ULONG 4
#define SIZE_SLONG 4
#define SIZE_FLOAT 4

//MUTEX record is a struct containing 3 8-bit CLUMP_IDs, packed into 32-bit word
//See MUTEX_Q typedef
#define SIZE_MUTEX 4

//Module IDs for IO map addressing
enum
{
  MOD_INPUT,
  MOD_OUTPUT
};

//Field IDs for input IOM
enum
{
  IO_IN_TYPE,
  IO_IN_MODE,
  IO_IN_ADRAW,
  IO_IN_NORMRAW,
  IO_IN_SCALEDVAL,
  IO_IN_INVALID_DATA
};

//FPP = Fields Per Port
#define IO_IN_FPP 6
#define IO_IN_FIELD_COUNT (IO_IN_FPP * NO_OF_INPUTS)

//Field IDs for input IOM
enum
{
  IO_OUT_FLAGS,
  IO_OUT_MODE,
  IO_OUT_SPEED, //AKA "Power"
  IO_OUT_ACTUAL_SPEED,
  IO_OUT_TACH_COUNT,
  IO_OUT_TACH_LIMIT,
  IO_OUT_RUN_STATE,
  IO_OUT_TURN_RATIO,
  IO_OUT_REG_MODE,
  IO_OUT_OVERLOAD,
  IO_OUT_REG_P_VAL,
  IO_OUT_REG_I_VAL,
  IO_OUT_REG_D_VAL,
  IO_OUT_BLOCK_TACH_COUNT,
  IO_OUT_ROTATION_COUNT,
  IO_OUT_OPTIONS,
  IO_OUT_MAX_SPEED,
  IO_OUT_MAX_ACCELERATION,
};

#define IO_OUT_FPP 18
#define IO_OUT_FIELD_COUNT (IO_OUT_FPP * NO_OF_OUTPUTS)

//
//DS_TOC_ENTRY is a record in the dataspace table of contents
//The TypeCode describes the data which is stored at Dataspace[DSOffset]
//
typedef struct
{
  TYPE_CODE TypeCode;
  UBYTE Flags;
  SWORD DSOffset;
} DS_TOC_ENTRY;

//DS_TOC_ENTRY Flags
//!!! Yes, there's only one flag defined for an 8-bit field.
//ARM7 alignment rules means those bits would otherwise just be padding, anyway.
#define DS_DEFAULT_DEFAULT  1 //This entry has no default value in file; fill with zero at activation time

//DS_ELEMENT_ID (AKA "DS item ID") indexes DataspaceTOC
typedef UWORD DS_ELEMENT_ID;

//Special flag value used for opcode-specific default behavior when real dataspace argument is not provided
#define NOT_A_DS_ID 0xFFFF

//Macro to bump DS_ELEMENT_IDs +1 with a cast (mostly to quash annoying warnings)
#define INC_ID(X) ((DS_ELEMENT_ID)(X + 1))

//DATA_ARG may contain a DS_ELEMENT_ID or encoded IO map address
typedef UWORD DATA_ARG;

//CODE_WORD is a single indexable element of the codespace
typedef SWORD CODE_WORD;

//CODE_INDEX indexes codespaces for opcodes and args
//!!! UWORD CODE_INDEX currently limits programs to 128KB code
// Yes, this is "plenty", but noted here to make sure we think about it
// when considering code size changes
typedef UWORD CODE_INDEX;

//Typedef and define to hold and check for valid file handles
typedef UBYTE FILE_HANDLE;

#define NOT_A_HANDLE 0xFF

//
// Dynamic Memory Manager
//

typedef UWORD DV_INDEX; //Dope Vector Index: Index into the DopeVectorArray

//DOPE_VECTOR struct: One instance exists in the DopeVectorArray for every array in the dataspace.
typedef struct
{
  UWORD Offset;
  UWORD ElemSize;
  UWORD Count;
  DV_INDEX BackLink; // points to previous DV
  DV_INDEX Link; // points to next DV
} DOPE_VECTOR;

//
//MEM_MGR struct
//Head and Tail keep track of the main linked-list of dope vectors,
// which must be maintained in ascending order according to Offset
//FreeHead is the head DV of the list of allocated but unused DVs
//pDopeVectorArray is initialized at activation-time to point to the master DVA
//
typedef struct
{
  DV_INDEX Head;
  DV_INDEX Tail;
  DV_INDEX FreeHead;
  DOPE_VECTOR * pDopeVectorArray;
} MEM_MGR;

//Macro to shorten common DVA access code
#define DV_ARRAY VarsCmd.MemMgr.pDopeVectorArray
//# of nodes to alloc when the Dope Vector Array is full
#define DV_ARRAY_GROWTH_COUNT 25
//Flag value for invalid Offset fields in DVs
#define NOT_AN_OFFSET 0xFFFF
//Check for legal index into DVA
#define IS_DV_INDEX_SANE(X) (((X) > 0) && ((X) < DV_ARRAY[0].Count))

//
// Message Queuing
//

//
//There are 10 incoming and 10 outgoing message queues, each 5 messages deep
//A "message" is defined as a null-terminated string under MAX_MESSAGE_SIZE
//
#define MESSAGES_PER_QUEUE  5
#define MESSAGE_QUEUE_COUNT 20
#define INCOMING_QUEUE_COUNT ((MESSAGE_QUEUE_COUNT)/2)
#define NOT_A_QUEUE 0xFF

//
//MAX_MESSAGE_SIZE including null-terminator
//!!! Capped at 59 unless USB protocol assumptions are changed!
//
#define MAX_MESSAGE_SIZE    59

//A MESSAGE is a dynamically sized string, so we use a DV_INDEX to get to its information
typedef DV_INDEX MESSAGE;

//
//MESSAGE_QUEUE keeps track of last messages read and written (acts as a circular buffer)
//
typedef struct
{
  UWORD ReadIndex;
  UWORD WriteIndex;
  MESSAGE Messages[MESSAGES_PER_QUEUE];
} MESSAGE_QUEUE;

//Handy macros for accessing MESSAGE_QUEUEs
#define GET_WRITE_MSG(QueueID) (VarsCmd.MessageQueues[(QueueID)].Messages[VarsCmd.MessageQueues[(QueueID)].WriteIndex])
#define GET_READ_MSG(QueueID) (VarsCmd.MessageQueues[(QueueID)].Messages[VarsCmd.MessageQueues[(QueueID)].ReadIndex])
#define SET_WRITE_MSG(QueueID, DVIndex) (VarsCmd.MessageQueues[(QueueID)].Messages[VarsCmd.MessageQueues[(QueueID)].WriteIndex] = (DVIndex))
#define SET_READ_MSG(QueueID, DVIndex) (VarsCmd.MessageQueues[(QueueID)].Messages[VarsCmd.MessageQueues[(QueueID)].ReadIndex] = (DVIndex))

#ifndef STRIPPED
//
// Datalog Queuing
//
// The datalog queue is loosely modeled around the message queue except that there is only one queue, not an array of them.
//

// A datalog has one less byte of 'header' info so different max size
#define MAX_DATALOG_SIZE    60

// The number of datalog messages to buffer
#define DATALOG_QUEUE_DEPTH  30

// A DATALOG_MESSAGE is a dynamically sized string, so we use a DV_INDEX to get to its information
typedef DV_INDEX DATALOG_MESSAGE;

//
// DATALOG_QUEUE keeps track of last messages read and written (acts as a circular buffer)
typedef struct
{
  UWORD ReadIndex;
  UWORD WriteIndex;
  DATALOG_MESSAGE Datalogs[DATALOG_QUEUE_DEPTH];
} DATALOG_QUEUE;

//Handy macros for accessing the DATALOG_QUEUE
#define GET_WRITE_DTLG() (VarsCmd.DatalogBuffer.Datalogs[VarsCmd.DatalogBuffer.WriteIndex])
#define GET_READ_DTLG() (VarsCmd.DatalogBuffer.Datalogs[VarsCmd.DatalogBuffer.ReadIndex])
#define SET_WRITE_DTLG(DVIndex) (VarsCmd.DatalogBuffer.Datalogs[VarsCmd.DatalogBuffer.WriteIndex] = (DVIndex))
#define SET_READ_DTLG(DVIndex) (VarsCmd.DatalogBuffer.Datalogs[VarsCmd.DatalogBuffer.ReadIndex] = (DVIndex))

#endif

//
//Definitions related to dataflow scheduling
//

//CLUMP_IDs are used to index list at pAllClumps
typedef UBYTE CLUMP_ID;

//
//The last value in CLUMP_ID's range is reserved as NOT_A_CLUMP
//This is useful as a queue terminator and general placeholder
//
#define NOT_A_CLUMP 0xFF
#define MAX_CLUMPS  255
#define INSTR_MAX_COUNT 20

//CLUMP_Q struct for tracking head and tail of a queue of clumps
typedef struct
{
  CLUMP_ID Head;
  CLUMP_ID Tail;
} CLUMP_Q;

//
//MUTEX_Q is a struct to be stashed in the dataspace to track state of a mutex
//If mutex is free, Owner field is NOT_A_CLUMP and WaitQ is empty.
//The mutex is acquired by stashing a new owner's ID.
//If others attempt to acquire, they will be put on the WaitQ
//
typedef struct
{
  CLUMP_ID Owner;
  CLUMP_Q WaitQ;
} MUTEX_Q;


// Clump Breakpoints
//
typedef struct
{ 
  CODE_INDEX Location;
  UBYTE Enabled;
} CLUMP_BREAK_REC;

#define MAX_BREAKPOINTS 4

//
// Clump Record, run-time book-keeping for each clump
//
// CodeStart: Start of this clump's bytecodes, absolute address
// CodeEnd: End of this clump's bytecodes, absolute address
// PC: "program counter" -- current offset into codespace relative to CodeStart
// InitFireCount: Initial count of upstream dependencies
// CurrFireCount: Run-time count of unsatisfied dependencies
// Link: ID of next clump in the queue.  NOT_A_CLUMP denotes end or bad link.
//
// Priority: number of instructions to run per pass on this clump
// clumpScalarDispatchHints: this clump only uses scalar data args, can be interpretted with faster dispatch tables
//
// pDependents: pointer to list of downstream dependents' ClumpIDs
// awakenTime: If a clump is on rest queue for sleep, this is the time at which it will return to runQueue
// DependentCount: Count of downstream dependents
//
typedef struct
{
  CODE_WORD*  CodeStart;
  CODE_WORD*  CodeEnd;
  CODE_WORD*  PC;
  UBYTE     InitFireCount;
  UBYTE     CurrFireCount; //AKA ShortCount
  CLUMP_ID  Link;

  UBYTE     Priority; // deleted in 1.28
  UBYTE     clumpScalarDispatchHints;

  CLUMP_ID* pDependents;
  ULONG     awakenTime;
  UBYTE     DependentCount;
  CLUMP_ID  CalledClump;
  CLUMP_BREAK_REC Breakpoints[MAX_BREAKPOINTS];
} CLUMP_REC;

//
//Definitions for memory pool management
//

//First valid pointer into the memory pool
#define POOL_START ((UBYTE*)(VarsCmd.Pool))

//Sentinel points one byte *past* the pool -- i.e. first bad pool pointer
#define POOL_SENTINEL ((UBYTE*)(VarsCmd.Pool + VarsCmd.PoolSize))

//Alignment mod for Pool and all sub-fields of the Pool
#define POOL_ALIGN SIZE_SLONG

#define ALIGN_TO_MOD(val,mod) if ((val) % (mod) != 0) { (val) += (mod) - ((val) % (mod)); } else {}

//
//Internal states of the VM
//VM_IDLE: Just sitting around.  Request to run program will lead to ONE of the VM_RUN* states.
//VM_RUN_FREE: Attempt to run as many instructions as possible within our timeslice
//VM_RUN_SINGLE: Run exactly one instruction per timeslice
//VM_RUN_PAUSE: Program still "active", but someone has asked us to pause
//VM_RESET2: Final clean up and return to IDLE
//VM_RESET1: Initialize state variables and some I/O devices -- executed when programs end
//
typedef enum
{
  VM_IDLE,
  VM_RUN_FREE,
  VM_RUN_SINGLE,
  VM_RUN_PAUSE,
  VM_RESET1,
  VM_RESET2,
} VM_STATE;

//
// VARSCMD: Private state data for active program and VM system
//
//pCodespace: pointer for flat codespace (stored in flash, includes all clumps)
//CodespaceCount: count of code words
//
//pAllClumps: Pointer to list of CLUMP_RECs
//AllClumpsCount: Count of CLUMP_RECs in list
//
//RunQ: Head and tail of run queue (elements in-place in AllClumps list)
//
//pDataspaceTOC: Pointer to DSTOC entries (stored in flash)
//DataspaceCount: Count of entries in DSTOC
//pDataspace: Base pointer of actual dataspace
//DataspaceSize: Size, in bytes, of dataspace
//DSStaticSize: Size, in bytes, of static portion of the dataspace (used as an offset to the dynamic dataspace)
//
//VMState: Internal state of VM's loader/scheduler (cCmdCtrl())
//
//MemMgr: Contains data to manage dynamic arrays
//
//PoolSize: Current size of main memory pool, in bytes.
//Pool: Static pool of bytes for stashing all program run-time data
//
//ActiveProgHandle: Handle of the program that is currently running
//ActiveProgName: Stashed name of currently running program, if any
//
//FileHandleTable: Table of file names opened by program while running.
// First byte of each record is 'r' or 'w' (read or write).
//
//MessageQueues: Message buffer tracking data
//
//CommStat, CommStatReset, CommCurrConnection, DirtyComm: Helper data for interfacing to c_comm module
//
//DirtyDisplay: Boolean reminding us to re-initialize the display if program used it
//
//StartTick: MS tick stashed when program started.  Used for relative time measurements.
//
//Further notes on the memory pool:
// The main memory pool is used for all clump records, dataspace tracking data,
// and the dataspace itself.  In other words, pAllClumps and
// pDataspace must all point to memory within the pool.  Watch for NXT_ASSERTs
// to enforce safe indexing into the pool.
//
typedef struct
{
  CODE_WORD*    pCodespace;
  CLUMP_REC*    pAllClumps;
  DS_TOC_ENTRY* pDataspaceTOC;
  UBYTE*        pDataspace;
  UBYTE*        Pool;

  ULONG     PoolSize;
  UWORD     CodespaceCount;
  CLUMP_ID  AllClumpsCount;
  UWORD     DataspaceCount;
  UWORD     DataspaceSize;
  UWORD     DSStaticSize;

  VM_STATE VMState;

  MEM_MGR   MemMgr;

  CLUMP_Q    RunQ;
  CLUMP_Q    RestQ;

  UBYTE ActiveProgHandle;
  UBYTE ActiveProgName[FILENAME_LENGTH + 1];

  UBYTE FileHandleTable[MAX_HANDLES][FILENAME_LENGTH + 2];

  MESSAGE_QUEUE MessageQueues[MESSAGE_QUEUE_COUNT];

  SWORD CommStat;
  SWORD CommStatReset;
  UBYTE CommCurrConnection;

  UBYTE DirtyComm;
  UBYTE DirtyDisplay;

  ULONG StartTick;

#ifndef STRIPPED
  DATALOG_QUEUE DatalogBuffer;
#endif

  UBYTE Debugging;
  UBYTE PauseClump;
  CODE_INDEX PausePC;
  
  // add a buffer for storing the last response raw content (64 bytes)
  UBYTE LastResponseBuffer[64];
  UBYTE LastResponseLength;
  
#if VM_BENCHMARK
  ULONG InstrCount;
  ULONG Average;
  ULONG OverTimeCount;
  ULONG MaxOverTimeLength;
  ULONG CmdCtrlCount;
  ULONG CompactionCount;
  ULONG LastCompactionTick;
  ULONG MaxCompactionTime;
  ULONG OpcodeBenchmarks[OPCODE_COUNT][4];
  ULONG SyscallBenchmarks[SYSCALL_COUNT][4];
  UBYTE Buffer[256];
#endif

#if defined ARM_DEBUG
  UBYTE AssertFlag;
  ULONG AssertLine;
#endif
} VARSCMD;

//
//Activation
//

//Activate new program by filename (open file and inflate run-time data)
NXT_STATUS cCmdActivateProgram(UBYTE * pFileName);

//Deactivate currently active program (re-init run-time data and close file)
void cCmdDeactivateProgram();

//Reset various device state variables
void cCmdResetDevices(void);

//Parse activation record file header information
typedef struct
{
  UWORD DSTOC;
  UWORD DSDefaults;
  UWORD DSDefaultsSize;
  UWORD DynamicDefaults;
  UWORD DynamicDefaultsSize;
  UWORD Clumps;
  UWORD Codespace;
} PROG_FILE_OFFSETS;

NXT_STATUS cCmdReadFileHeader(UBYTE* pData, ULONG DataSize,
            PROG_FILE_OFFSETS* pFileOffsets);

NXT_STATUS cCmdInflateDSDefaults(UBYTE* pDSDefaults, UWORD *pDefaultsOffset, DS_ELEMENT_ID DSElementID);


//
//Clump management
//

//Clump queuing
void cCmdEnQClump(CLUMP_Q * Queue, CLUMP_ID NewClump);
void cCmdDeQClump(CLUMP_Q * Queue, CLUMP_ID Clump);
void cCmdRotateQ();
UBYTE cCmdIsClumpOnQ(CLUMP_Q * Queue, CLUMP_ID Clump);
UBYTE cCmdIsQSane(CLUMP_Q * Queue);

// Rest queue functions
NXT_STATUS cCmdSleepClump(ULONG time);
UBYTE cCmdCheckRestQ(ULONG currTime);

//Mutex queuing
NXT_STATUS cCmdAcquireMutex(MUTEX_Q * Mutex);
NXT_STATUS cCmdReleaseMutex(MUTEX_Q * Mutex);

//Conditionally schedule dependents of given clump (Begin and End specify subset of list)
NXT_STATUS cCmdSchedDependents(CLUMP_ID Clump, SWORD Begin, SWORD End);

//Conditionally schedule TargetClump
NXT_STATUS cCmdSchedDependent(CLUMP_ID Clump, CLUMP_ID TargetClump);

//Test if ClumpID is sane at run-time (valid for indexing AllClumps)
UBYTE cCmdIsClumpIDSane(CLUMP_ID Clump);

//
//Code stream management
//

//Instruction masking macros -- get the interesting bits out of an encoded instruction word
#define COMP_CODE(pInstr)   ((UBYTE)((((pInstr)[0]) & 0x0700) >> 8))
#define INSTR_SIZE(wd)      ((wd) >> 12) & 0x0F;

#define IS_SHORT_OP(pInstr)   ((UBYTE)((((pInstr)[0]) & 0x0800) >> 8) == 8)
#define SHORT_OP_CODE(pInstr) COMP_CODE(pInstr)
#define SHORT_ARG(pInstr)     ((SBYTE) (((pInstr)[0]) & 0x00FF))
//ShortOpMap defined in c_cmd_bytecodes.h
#define OP_CODE(pInstr)       (UBYTE) (((pInstr)[0]) & 0x00FF)

//
//Memory pool management
//

//Initialize entire memory pool with default value
void cCmdInitPool(void);

//Resize dataspace array specified by DSElementID and Offset.
NXT_STATUS cCmdDSArrayAlloc(DS_ELEMENT_ID DSElementID, UWORD Offset, UWORD NewCount);
//Resize dataspace array specified by DVIndex.  In most cases, call higher-level cCmdDSArrayAlloc instead.
NXT_STATUS cCmdDVArrayAlloc(DV_INDEX DVIndex, UWORD NewCount);

NXT_STATUS cCmdAllocSubArrayDopeVectors(DS_ELEMENT_ID DSElementID, UWORD Offset);
NXT_STATUS cCmdFreeSubArrayDopeVectors(DS_ELEMENT_ID DSElementID, UWORD Offset);
NXT_STATUS cCmdAllocDopeVector(DV_INDEX *pIndex, UWORD ElemSize);
NXT_STATUS cCmdFreeDopeVector(DV_INDEX DVIndex);
NXT_STATUS cCmdGrowDopeVectorArray(UWORD NewCount);

UWORD cCmdCalcArrayElemSize(DS_ELEMENT_ID DSElementID);

NXT_STATUS cCmdMemMgrMoveToTail(DV_INDEX DVIndex);
NXT_STATUS cCmdMemMgrInsertAtTail(DV_INDEX DVIndex);

//Utility function to check sanity of MemMgr data structure.  Boolean result.
UBYTE cCmdVerifyMemMgr();

NXT_STATUS cCmdDSCompact(void);

//
// Message Queue management
//

NXT_STATUS cCmdMessageWrite(UWORD QueueID, UBYTE * pData, UWORD Length);
NXT_STATUS cCmdMessageRead(UWORD QueueID, UBYTE * pData, UWORD Length, UBYTE Remove);
NXT_STATUS cCmdMessageGetSize(UWORD QueueID, UWORD * Size);

//
// Datalog Queue management
//

NXT_STATUS cCmdDatalogWrite(UBYTE * pData, UWORD Length);
NXT_STATUS cCmdDatalogRead(UBYTE * pData, UWORD Length, UBYTE Remove);
NXT_STATUS cCmdDatalogGetSize(UWORD * Size);

//
// Color Sensor
//

NXT_STATUS cCmdColorSensorRead (UBYTE Port, SWORD* SensorValue, UWORD* RawArray, UWORD* NormalizedArray,
								SWORD* ScaledArray, UBYTE* InvalidData);

//
//Dataspace management
//

#define IS_AGGREGATE_TYPE(TypeCode) ((TypeCode == TC_ARRAY) || (TypeCode == TC_CLUSTER))
// use carefully, only where tc will be a scalar int
#define QUICK_UNSIGNED_TEST(TypeCode) ((TypeCode) & 0x1)
#define IS_SIGNED_TYPE(TypeCode) (((TypeCode) == TC_SBYTE) || ((TypeCode) == TC_SWORD) || ((TypeCode) == TC_SLONG))
//!!!BDUGGAN add TC_FLOAT?

//Test if DS_ELEMENT_ID is sane at run-time (valid for indexing DS TOC)
UBYTE cCmdIsDSElementIDSane(DS_ELEMENT_ID Index);

DS_ELEMENT_ID cCmdGetDataspaceCount(void);

//Pointer accessors to resolve actual data locations in RAM
void* cCmdDSPtr(DS_ELEMENT_ID DSElementID, UWORD Offset);
void* cCmdDVPtr(DV_INDEX DVIndex);

//Helper to walk the DSTOC to the next entry at the same aggregate nesting level as CurrID
DS_ELEMENT_ID cCmdNextDSElement(DS_ELEMENT_ID CurrID);

//Recursively compare two complete data type descriptors
UBYTE cCmdCompareDSType(DS_ELEMENT_ID DSElementID1, DS_ELEMENT_ID DSElementID2);

//Functions for managing data flattened to byte arrays
UWORD cCmdCalcFlattenedSize(DS_ELEMENT_ID DSElementID, UWORD Offset);
NXT_STATUS cCmdFlattenToByteArray(UBYTE * pByteArray, UWORD * pByteOffset, DS_ELEMENT_ID DSElementID, UWORD Offset);
NXT_STATUS cCmdUnflattenFromByteArray(UBYTE * pByteArray, UWORD * pByteOffset, DS_ELEMENT_ID DSElementID, UWORD Offset);

//Comparison evaluation.  Comparison codes defined in c_cmd_bytecodes.h.
//cCmdCompare operates on scalars passed as ULONGs -- type-specific comparisons done inside function.
UBYTE cCmdCompare(UBYTE CompCode, ULONG Val1, ULONG Val2, TYPE_CODE TypeCode1, TYPE_CODE TypeCode2);
UBYTE cCmdCompareFlt(UBYTE CompCode, float Val1, float Val2, TYPE_CODE TypeCode1, TYPE_CODE TypeCode2);
//cCmdCompareAggregates does polymorphic comparisons (with recursive helper function).
NXT_STATUS cCmdCompareAggregates(UBYTE CompCode, UBYTE *ReturnBool, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3);
NXT_STATUS cCmdRecursiveCompareAggregates(UBYTE CompCode, UBYTE *ReturnBool, UBYTE *Finished, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3);

//Cluster functions
UWORD cCmdClusterCount(DS_ELEMENT_ID DSElementID);

//Array functions
#define ARRAY_ELEM_OFFSET(DVIndex, Index) ((UWORD)(DV_ARRAY[(DVIndex)].Offset + DV_ARRAY[(DVIndex)].ElemSize * (Index)))
UWORD cCmdGetDVIndex(DS_ELEMENT_ID DSElementID, UWORD Offset);
UWORD cCmdArrayCount(DS_ELEMENT_ID DSElementID, UWORD Offset);
TYPE_CODE cCmdArrayType(DS_ELEMENT_ID DSElementID);

//!!! DATA_ARG masks are for internal use only! (Bytecode programs should never contain them)
//    See cCmdResolveDataArg() calls in the interpreter code for OP_GETOUT, OP_SETIN, and OP_GETIN.
#define DATA_ARG_ADDR_MASK 0x3FFF
#define DATA_ARG_IMM_MASK 0x7FFF

//General data accessors (DS and IO Map)
void * cCmdResolveDataArg(DATA_ARG DataArg, UWORD Offset, TYPE_CODE * TypeCode);
void * cCmdResolveIODataArg(DATA_ARG DataArg, ULONG Offset, TYPE_CODE * TypeCode);
ULONG cCmdGetVal(void * pVal, TYPE_CODE TypeCode);
void cCmdSetVal(void * pVal, TYPE_CODE TypeCode, ULONG NewVal);

// Calibration routines
void cCmdLoadCalibrationFiles(void);
NXT_STATUS cCmdComputeCalibratedValue(UBYTE *nm, SWORD *raw);
void cCmdUpdateCalibrationCache(UBYTE *nm, SWORD min, SWORD max);

//
//Interpreter functions
//

//Clump-based "master" interpreter
NXT_STATUS cCmdInterpFromClump();

//Function pointer typedef for sub-interpreters
typedef NXT_STATUS (*pInterp)(CODE_WORD * const);
typedef NXT_STATUS (*pInterpShort)(CODE_WORD * const);

//Sub-interpreter dispatch functions
NXT_STATUS cCmdInterpNoArg(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpUnop1(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpUnop2(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpScalarUnop2(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpBinop(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpScalarBinop(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpOther(CODE_WORD * const pCode);

NXT_STATUS cCmdInterpShortError(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpShortSubCall(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpShortMove(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpShortAcquire(CODE_WORD * const pCode);
NXT_STATUS cCmdInterpShortRelease(CODE_WORD * const pCode);

NXT_STATUS cCmdMove(DATA_ARG Arg1, DATA_ARG Arg2);

//Polymorphic interpreter functions
NXT_STATUS cCmdInterpPolyUnop2(CODE_WORD const Code, DATA_ARG Arg1, UWORD Offset1, DATA_ARG Arg2, UWORD Offset2);
ULONG cCmdUnop2(CODE_WORD const Code, ULONG Operand, TYPE_CODE TypeCode);
float cCmdUnop2Flt(CODE_WORD const Code, float Operand, TYPE_CODE TypeCode);

NXT_STATUS cCmdInterpPolyBinop(CODE_WORD const Code, DATA_ARG Arg1, UWORD Offset1, DATA_ARG Arg2, UWORD Offset2, DATA_ARG Arg3, UWORD Offset3);
ULONG cCmdBinop(CODE_WORD const Code, ULONG LeftOp, ULONG RightOp, TYPE_CODE LeftType, TYPE_CODE RightType);
float cCmdBinopFlt(CODE_WORD const Code, float LeftOp, float RightOp, TYPE_CODE LeftType, TYPE_CODE RightType);
void cCmdSetValFlt(void * pVal, TYPE_CODE TypeCode, float NewVal);
float cCmdGetValFlt(void * pVal, TYPE_CODE TypeCode);
//
//Support functions for lowspeed (I2C devices, i.e. ultrasonic sensor) communications
//

NXT_STATUS cCmdLSCheckStatus(UBYTE Port);
UBYTE cCmdLSCalcBytesReady(UBYTE Port);
NXT_STATUS cCmdLSWrite(UBYTE Port, UBYTE BufLength, UBYTE *pBuf, UBYTE ResponseLength, UBYTE NoRestartOnRead, UBYTE bFast);
NXT_STATUS cCmdLSRead(UBYTE Port, UBYTE BufLength, UBYTE * pBuf);

//
//Support for OP_SYSCALL
//

//
//Each cCmdWrap<SysCallName> funtion below implements one system call.
//The OP_SYSCALL interpreter wrangles the argument vector, ArgV,
// then calls the appropriate wrapper function according to the SysCallID.
//Wrapper functions write directly back into the dataspace via ArgV.
//
#define MAX_CALL_ARGS 16

typedef NXT_STATUS (*pSysCall)(UBYTE * ArgV[]);

NXT_STATUS cCmdWrapFileOpenRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileOpenWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileOpenAppend(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileClose(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileResolveHandle (UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileRename (UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileDelete (UBYTE * ArgV[]);
NXT_STATUS cCmdWrapSoundPlayFile(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapSoundPlayTone(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapSoundGetState(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapSoundSetState(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawText(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawPoint(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawLine(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawCircle(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawRect(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawPicture(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapSetScreenMode(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapReadButton(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommLSWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommLSRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommLSCheckStatus(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapRandomNumber(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapGetStartTick(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapMessageWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapMessageRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDatalogWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommBTCheckStatus(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommBTWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommBTRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapKeepAlive(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapIOMapRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapIOMapWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapColorSensorRead (UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDatalogGetTimes(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapSetSleepTimeout(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapListFiles(UBYTE * ArgV[]);

// Handlers for dynamically added syscalls
NXT_STATUS cCmdWrapCommBTOnOff(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommBTConnection(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapReadSemData(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapWriteSemData(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapUpdateCalibCacheInfo(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapComputeCalibValue(UBYTE * ArgV[]);

NXT_STATUS cCmdWrapInputPinFunction(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapIOMapReadByID(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapIOMapWriteByID(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDisplayExecuteFunction(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommExecuteFunction(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapLoaderExecuteFunction(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileFindFirst(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileFindNext(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileOpenWriteLinear(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileOpenWriteNonLinear(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileOpenReadLinear(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommHSControl(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommHSCheckStatus(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommHSWrite(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommHSRead(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapCommLSWriteEx(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileSeek(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileResize(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawPictureArray(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawPolygon(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawEllipse(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapDrawFont(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapMemoryManager(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapReadLastResponse(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapFileTell(UBYTE * ArgV[]);
NXT_STATUS cCmdWrapRandomEx(UBYTE * ArgV[]);

NXT_STATUS cCmdWrapUndefinedSysCall(UBYTE * ArgV[]);

//Handler for remote control protocol packets -- called from comm module via IO map function pointer
UWORD cCmdHandleRemoteCommands(UBYTE * pInBuf, UBYTE * pOutBuf, UBYTE * pLen);

#ifdef SIM_NXT
//
// Helper functions to provide simulator library access to VM internals
//
SWORD cCmdGetCodeWord(CLUMP_ID Clump, CODE_INDEX Index);
UBYTE * cCmdGetDataspace(UWORD *DataspaceSize);
DOPE_VECTOR * cCmdGetDopeVectorPtr(void);
ULONG cCmdGetPoolSize(void);
MEM_MGR cCmdGetMemMgr(void);
#endif

#else //!ENABLE_VM

//Placeholder VARSCMD for alternate implementation (see bottom of c_cmd.c for usage notes)
typedef struct
{
  UBYTE         Tmp;
} VARSCMD;

#endif //ENABLE_VM

#endif //C_CMD
