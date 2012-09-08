/** @file debug_internals.h
 *  @brief Shared C/ASM header file for debugger internal constants
 *
 */

/* Copyright (C) 2007-2011 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * See AUTHORS for a full list of the developers.
 *
 * See COPYING for redistribution license
 *
 */

#ifndef __DEBUG_INTERNALS_H__
#define __DEBUG_INTERNALS_H__

#include "_c_arm_macros.h"


/** @addtogroup debugger */
/*@{*/


/* Declarations go here. */
/** @name Debug Message Constants.
 *
 * Debug Message Values
 */
/*@{*/

/*
 * USB Buffer Sizes:    Ctrl    Intr    Iso     Bulk
 * Full Speed Device    64      64      1023    64
 * High Speed Device    64      1024    1024    512
 */

#define USB_BUFSIZE     64                                      /* USB Buffer size for AT91SAM7S */

#define NXT_MSG_TELEGRAMTYPE_OFFSET     0                       /* NXT Direct Command/Response Header */
#define NXT_MSG_SEGNUM_OFFSET           1
#define NXT_MSG_TELEGRAMSIZE_OFFSET     2

#define NXT_GDBMSG_TELEGRAMTYPE            0x8d                 /* GDB debugger specific, no Response required */

#define NXT_GDBMSG_START                3                       /* Offset into USB Telegram buffer */

#define MSG_NUMSEGMENTS  		3                       /* For packet transfers */
#define MSG_SEGMENTSIZE (USB_BUFSIZE - NXT_GDBMSG_START)        /* 61 bytes per segment */
#define MSGBUF_SIZE     (MSG_SEGMENTSIZE*MSG_NUMSEGMENTS)       /* Debug Message Buffer Size, 61 x 3 = 183 chars = ~80 bytes of actual data */
#define MSGBUF_CHKSUMOFFSET             3                       /* to be subtracted from message length */
#define MSGBUF_IN_OVERHEADLEN		5			/* For calculating max message data length (exclude ASCIIZ char) */
#define MSGBUF_OUT_OVERHEADLEN		5			/* For calculating max message data length (exclude ASCIIZ char) */

#define MSGBUF_CTRLC     0x03                                   /* For Out of Band Signaling: not implemented yet */
#define MSGBUF_STARTCHAR '$'
#define MSGBUF_ACKCHAR   '+'
#define MSGBUF_NAKCHAR   '-'
#define MSGBUF_ERRCHAR   'E'
#define MSGBUF_SIGCHAR   'S'
#define MSGBUF_SETCHAR   '='
#define MSGBUF_CHKSUMCHAR '#'
#define MSGBUF_SEPCHAR   ','
#define MSGBUF_ARGCHAR   ':'
#define MSGBUF_MSGERROR  -1
/*@}*/

/** @name Debug Command Lookup Constants.
 *
 * Debug Command Lookup
 */
/*@{*/

#define CMDINDEX_OUTOFRANGE -1
/*@}*/

/** @name Debug Register Command Constants.
 *
 * Debug Register Command
 */
/*@{*/
#define CMD_REG_NUMREGS             17
#define CMD_REG_GETONE_MINPARAMLEN  1
#define CMD_REG_GETONE_MAXPARAMLEN  2
#define CMD_REG_GETALL_PARAMLEN     0
#define CMD_REG_REGPARAMLEN         8   /* 32-bit ASCII Hex Value */
#define CMD_REG_SETONE_MINPARAMLEN     (2 + CMD_REG_REGPARAMLEN)
#define CMD_REG_SETONE_MAXPARAMLEN     (3 + CMD_REG_REGPARAMLEN)
#define CMD_REG_SETALL_PARAMLEN     (CMD_REG_NUMREGS*CMD_REG_REGPARAMLEN)
#define CMD_KILL_PARAMLEN     		0
#define CMD_DETACH_PARAMLEN    		0

/*@}*/

/** @name Debug Memory Command Constants.
 *
 * Debug Memory Command
 * FIXME: These limits are not enforced by the GDB client, it truncates addresses and lengths to remove leading '0's
 *        The PARAMLEN constants would probably be removed
 */
/*@{*/
#define CMD_NUMITEMS_PARAMLEN		4	/* 16-bit ASCII Hex Value */
#define CMD_MEM_READ_PARAMLEN		(CMD_REG_REGPARAMLEN + CMD_NUMITEMS_PARAMLEN + 1)	/* Address length is equivalent to reg param len */
#define CMD_MEM_WRITE_MINPARAMLEN	(CMD_REG_REGPARAMLEN + CMD_NUMITEMS_PARAMLEN + 2)	/* Address length is equivalent to reg param len */
#define CMD_MEM_SEPCHAR_OFFSET		CMD_REG_REGPARAMLEN	/* Address length is equivalent to reg param len */
#define CMD_MEM_MAXOUTBUFLEN		(MSGBUF_SIZE - MSGBUF_OUT_OVERHEADLEN)
#define CMD_MEM_MAXREADBYTES		(CMD_MEM_MAXOUTBUFLEN/2)
#define CMD_MEM_MAXINBUFLEN			(MSGBUF_SIZE - MSGBUF_IN_OVERHEADLEN)
#define CMD_MEM_MAXWRITEBYTES		((CMD_MEM_MAXINBUFLEN - CMD_MEM_WRITE_MINPARAMLEN)/2)
/*@}*/

/** @name Debug Continue and Step Command Constants.
 *
 * Debug Continue and Step Command
 */
/*@{*/
#define CMD_CONTINUE_MINPARAMLEN    0
#define CMD_STEP_MINPARAMLEN        0
/*@}*/

/** @name Debug Query Command Constants.
 *
 * Debug Query Command
 */
/*@{*/
#define CMD_QUERY_MINPARAMLEN    0
#define CMD_QUERY_CURRTID_PARAMLEN    1
#define CMD_QUERY_FTINFO_PARAMLEN 11
#define CMD_QUERY_STINFO_PARAMLEN 11
#define CMD_QUERY_CURRTID_CHAR  'C'
#define CMD_QUERY_FTINFO_CHAR   'f'
#define CMD_QUERY_STINFO_CHAR   's'
/*@}*/


/** @name Debug Breakpoint Command Constants.
 *
 * Debug Breakpoint Command
 */
/*@{*/

#define CMD_BKPT_INSERT_MINPARAMLEN     5
#define CMD_BKPT_REMOVE_MINPARAMLEN     5


#define CMD_BKPT_TYPE_BREAK_MEMORY  0
#define CMD_BKPT_TYPE_BREAK_HARD    1   /* Not supported */
#define CMD_BKPT_TYPE_WATCH_WRITE   2   /* Not supported (yet) */
#define CMD_BKPT_TYPE_WATCH_READ    3   /* Not supported (yet) */
#define CMD_BKPT_TYPE_WATCH_ACCESS  4   /* Not supported (yet) */

#define CMD_BKPT_KIND_THUMB         2
#define CMD_BKPT_KIND_THUMB2        3   /* Not supported */
#define CMD_BKPT_KIND_ARM           4

#define CMD_BKPT_NOTFOUND           -1

/*@}*/

/** @name Debug Stack Constants.
 *
 * Debug Stack Manipulation Values
 */
/*@{*/
#define DBGSTACK_NEXTINSTR_INDEX 0                /* Next Instruction Address is at index 0 from bottom of Debug Stack */
#define DBGSTACK_USERCPSR_INDEX 1                /* User CPSR (SPSR_UNDEF) is at index 1 from bottom of Debug Stack */
#define DBGSTACK_USERREG_INDEX  2                /* R0 starts at index 2 from bottom of Debug Stack */
#define DBGSTACK_USERSP_INDEX   (DBGSTACK_USERREG_INDEX + REG_SP)     /* SP is R13 */
#define DBGSTACK_USERLR_INDEX   (DBGSTACK_USERREG_INDEX + REG_LR)     /* LR is R14 */
#define DBGSTACK_USERPC_INDEX   (DBGSTACK_USERREG_INDEX + REG_PC)     /* PC is R15 */
/*@}*/


/** @name Exception Handler Vector Definitions.
 *
 * Exception Handler Vectors.
 */
/*@{*/

#define RESET_VECTOR	0x00000000
#define UNDEF_VECTOR	0x00000004
#define SVC_VECTOR	0x00000008
#define PABRT_VECTOR	0x0000000C
#define DABRT_VECTOR	0x00000010
#define RESERVED_VECTOR 0x00000014
#define IRQ_VECTOR	0x00000018
#define FIQ_VECTOR	0x0000001C


/*@}*/


/** @name Bitmask Definitions.
 *
 * Various Bitmasks used for data manipulation.
 */
/*@{*/
#define BKPT_STATE_THUMB_FLAG	0x01             /* Flag Thumb Breakpoint */
#define ASCII_LOWER2UPPER_MASK	0x20             /* ASCII Conversion bitmask */
#define NIBBLE0	0x0000000F                       /* Nibble 0 word(3:0) */
#define NIBBLE1	0x000000F0                       /* Nibble 1 word(7:4) */
#define NIBBLE2	0x00000F00                       /* Nibble 2 word(11:8) */
#define NIBBLE3	0x0000F000                       /* Nibble 3 word(15:12) */
#define NIBBLE4	0x000F0000                       /* Nibble 4 word(19:16) */
#define NIBBLE5	0x00F00000                       /* Nibble 5 word(23:20) */
#define NIBBLE6	0x0F000000                       /* Nibble 6 word(27:24) */
#define NIBBLE7	0xF0000000                       /* Nibble 7 word(31:28) */
#define BYTE0	0x000000FF                       /* Byte 0 word(7:0) */
#define BYTE1	0x0000FF00                       /* Byte 1 word(15:8) */
#define BYTE2	0x00FF0000                       /* Byte 2 word(23:16) */
#define BYTE3	0xFF000000                       /* Byte 3 word(31:24) */
#define HLFWRD0	0x0000FFFF                       /* Halfword 0 word(15:0) */
#define HLFWRD1	0xFFFF0000                       /* Halfword 0 word(31:16) */
/*@}*/

/** @name CPSR Bit Definitions.
 *
 * Various Bit definitions for accessing the CPSR register.
 */
/*@{*/
#define CPSR_THUMB      0x00000020
#define CPSR_FIQ        0x00000040
#define CPSR_IRQ        0x00000080
#define CPSR_MODE       0x0000001F
#define CPSR_COND	0xF0000000

/* ARM Exception Modes */
#define MODE_USR 0x10                   /* User mode */
#define MODE_FIQ 0x11                   /* FIQ mode */
#define MODE_IRQ 0x12                   /* IRQ mode */
#define MODE_SVC 0x13                   /* Supervisor mode */
#define MODE_ABT 0x17                   /* Abort mode */
#define MODE_UND 0x1B                   /* Undefined mode */
#define MODE_SYS 0x1F                   /* System mode */

/* Condition Flags
 * b31 b30 b29 b28
 *  N   Z   C   V
 */
#define CPSR_NFLAG		0x80000000
#define CPSR_ZFLAG		0x40000000
#define CPSR_CFLAG		0x20000000
#define CPSR_VFLAG		0x10000000


/*
 * ARM Opcode Masks (for Parser)
 */
#define ARM_DATA_INSTR_MASK     0x0FBF0000
#define ARM_DATA_INSTR_MSRMRS   0x010F0000
#define ARM_DATA_INSTR_NORMAL   0x01E00000
#define ARM_DATA_INSTR_IMMREG   0x02000000

#define ARM_LDR_INSTR_REGIMM    0x02000000
#define ARM_LDR_INSTR_PREPOST   0x01000000
#define ARM_LDR_INSTR_UPDOWN    0x00800000

#define ARM_LDM_INSTR_PREPOST   0x01000000
#define ARM_LDM_INSTR_UPDOWN    0x00800000

#define ARM_BLX_INSTR_MASK      0xFE000000
#define ARM_BLX_INSTR_BLX       0xFA000000
#define ARM_BLX_INSTR_HBIT      0x01000000

#define ARM_SWI_INSTR_MASK      0x0F000000
#define ARM_SWI_INSTR_VAL       0x0F000000


/*
 * Thumb Opcode Masks (for Parser)
 */
#define THUMB_BLX_INSTR_REG_RNMASK     0x0078

#define THUMB_BCOND_SWI_INSTR_CONDMASK 0x0F00
#define THUMB_BCOND_SWI_COND_UNUSED    0x0E00
#define THUMB_BCOND_SWI_INSTR_SWI      0x0F00

#define THUMB_BLX_INSTR_IMM_HBIT       0x0800
#define THUMB_BLX_INSTR_IMM_MASK       0xF000
#define THUMB_BLX_INSTR_IMM_BL         0xF000
#define THUMB_BLX_INSTR_IMM_BLX        0xE000

/*@}*/

/** Debugger State Enums
 *
 * Debugger State.
 * The enums must be consecutive, starting from 0
 */
ENUM_BEGIN
ENUM_VALASSIGN(DBG_RESET, 0)  /**< Initial State. */
ENUM_VAL(DBG_INIT)            /**< Debugger Initialized. */
ENUM_VAL(DBG_CONFIGURED)        /**< Debugger has been configured by GDB Server */
ENUM_END(dbg_state_t)

/** Breakpoint Type Enums
 *
 * Breakpoint Type.
 * The enums must be consecutive, starting from 0
 */
ENUM_BEGIN
ENUM_VALASSIGN(DBG_AUTO_BKPT,0)         /**< RESERVED: Auto Breakpoint (Instruction resume after breakpoint). */
ENUM_VAL(DBG_MANUAL_BKPT_ARM)           /**< Manual ARM Breakpoint. */
ENUM_VAL(DBG_NORMAL_BKPT_ARM)           /**< Normal ARM Breakpoint (Single Step, Normal). */
ENUM_VAL(DBG_MANUAL_BKPT_THUMB)         /**< Manual Thumb Breakpoint. */
ENUM_VAL(DBG_NORMAL_BKPT_THUMB)         /**< Normal Thumb Breakpoint (Single Step, Normal). */
ENUM_VAL(DBG_ABORT_PREFETCH)            /**< Prefetch Abort. */
ENUM_VAL(DBG_ABORT_DATA)                /**< Data Abort. */
ENUM_END(bkpt_type_t)

/** Debugger Message Signal Enums
 *
 * Debugger Signal Message Enums.
 * The enums must be consecutive, starting from 0
 */
/* Need to sync with the Signal enums in ecos-common-hal_stub.c */
ENUM_BEGIN
ENUM_VALASSIGN(MSG_SIG_DEFAULT, 0)    /**< Default Signal Response. */
ENUM_VAL(MSG_SIG_HUP)                 /**< Hangup Signal Response. */
ENUM_VAL(MSG_SIG_INT)                 /**< Interrupt Signal Response. */
ENUM_VAL(MSG_SIG_QUIT)                /**< Quit Signal Response. */
ENUM_VAL(MSG_SIG_ILL)                 /**< Illegal Instruction Signal Response (not reset when caught). */
ENUM_VAL(MSG_SIG_TRAP)                /**< Trace Trap Signal Response (not reset when caught). */
ENUM_VAL(MSG_SIG_ABRT)                /**< Abort Signal Response (replace SIGIOT). */
ENUM_VAL(MSG_SIG_EMT)                 /**< EMT Instruciton Signal Response. */
ENUM_VAL(MSG_SIG_FPE)                 /**< Floating Point Exception Signal Response. */
ENUM_VAL(MSG_SIG_KILL)                /**< Kill Signal Response (cannot be caught or ignored). */
ENUM_VAL(MSG_SIG_BUS)                 /**< Bus Error Signal Response. */
ENUM_VAL(MSG_SIG_SEGV)                /**< Segmentation Violation Signal Response. */
ENUM_VAL(MSG_SIG_SYS)                 /**< Bad Argument to System Call Signal Response. */
ENUM_VAL(MSG_SIG_PIPE)                /**< Write on a Pipe with No Reader Signal Response. */
ENUM_VAL(MSG_SIG_ALRM)                /**< Alarm Clock Signal Response. */
ENUM_VAL(MSG_SIG_TERM)                /**< Software Termination Signal from Kill Signal Response. */
ENUM_END(dbg_msg_signo)

/** Debugger Message Error Enums
 *
 * Debugger Error Message Enums.
 * The enums must be consecutive, starting from 1
 */
/* FIXME: Need to validate against the ecos-generic-stub.c Error enums */
ENUM_BEGIN
ENUM_VALASSIGN(MSG_ERRIMPL, 0)    /**< Stub (not implemented) Error. */
ENUM_VAL(MSG_ERRINLENGTH)      	  /**< Message Write Length Error. */
ENUM_VAL(MSG_ERROUTLENGTH)        /**< Message Read Length Error. */
ENUM_VAL(MSG_ERRFORMAT)           /**< Message Format Error. */
ENUM_VAL(MSG_UNKNOWNCMD)          /**< Unrecognized Command Error. */
ENUM_VAL(MSG_UNKNOWNPARAM)        /**< Unrecognized Parameter Error. */
ENUM_VAL(MSG_UNKNOWNBRKPT)        /**< Unrecognized Breakpoint Error. */
ENUM_END(dbg_msg_errno)

/** Register Enums
 *
 * Register Enums.
 * Refer to eCOS's arm_stub.h for enum values
 */
ENUM_BEGIN
ENUM_VALASSIGN(REG_R0, 0)     /**< User Reg R0 */
ENUM_VAL(REG_R1)              /**< User Reg R1 */
ENUM_VAL(REG_R2)              /**< User Reg R2 */
ENUM_VAL(REG_R3)              /**< User Reg R3 */
ENUM_VAL(REG_R4)              /**< User Reg R4 */
ENUM_VAL(REG_R5)              /**< User Reg R5 */
ENUM_VAL(REG_R6)              /**< User Reg R6 */
ENUM_VAL(REG_R7)              /**< User Reg R7 */
ENUM_VAL(REG_R8)              /**< User Reg R8 */
ENUM_VAL(REG_R9)              /**< User Reg R9 */
ENUM_VAL(REG_R10)             /**< User Reg R10 */
ENUM_VAL(REG_R11)             /**< User Reg R11 */
ENUM_VAL(REG_R12)             /**< User Reg R12 */
ENUM_VAL(REG_SP)              /**< Previous Mode SP (R13) */
ENUM_VAL(REG_LR)              /**< Previous Mode LR (R14) */
ENUM_VAL(REG_PC)              /**< Program Counter (R15) */
ENUM_VALASSIGN(REG_FPSCR, 24) /**< Previous Mode FPSCR (dummy) */
ENUM_VAL(REG_CPSR)            /**< Previous Mode CPSR */

ENUM_END(register_enum_t)

/** Abort Type Enums
 *
 * Abort Type used for interfacing with LCD Display routine.
 * The enums must be consecutive, starting from 0
 * Note: The values must align with those defined in NxOS's _abort.h
 */
ENUM_BEGIN
ENUM_VALASSIGN(DISP_ABORT_PREFETCH,0)       /**< Prefetch Abort. */
ENUM_VAL(DISP_ABORT_DATA)                   /**< Data Abort. */
ENUM_VAL(DISP_ABORT_SPURIOUS)               /**< Spurious IRQ. */
ENUM_VAL(DISP_ABORT_ILLEGAL)                /**< Illegal Instruction. */
ENUM_END(abort_type_t)

/*@}*/

#endif /* __DEBUG_INTERNALS_H__ */
