/** @file debug_stub.h
 *  @brief Shared C/ASM header file for debugger stub
 *
 */

/* Copyright (C) 2007-2010 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * See AUTHORS for a full list of the developers.
 *
 * See COPYING for redistribution license
 *
 */

#ifndef __DEBUG_STUB_H__
#define __DEBUG_STUB_H__

#include "_c_arm_macros.h"

/** @name BKPT suppport constants
 *
 * ARM and Thumb Breakpoint Instructions.
 */
/*@{*/

#define __ARM6OR7__

#ifdef __ARM6OR7__
#define BKPT32_INSTR            0xE7200070      /* ARM6 and ARM7 does not trap unused opcodes (BKPT overlap with control instructions), \
                                                   CPU has unpredictable behavior. Ref: Steve Furber, ARM SoC Architecture 2nd Ed, pg. 143 */
#else
#define BKPT32_INSTR            0xE1200070      /* ARM BKPT instruction, will work in ARMv5T and above */
#endif

#define BKPT32_ENUM_MASK        0x000FFF0F      /* ARM BKPT Enum Mask */
#define BKPT32_AUTO_BKPT        0x00080000      /* RESERVED: ARM BKPT Auto-Step Flag (for CONT support) */
#define BKPT32_MANUAL_BKPT      0x0007FF0F      /* Manually inserted ARM Breakpoint */

#define BKPT16_INSTR            0xBE00          /* Thumb BKPT instruction */
#define BKPT16_ENUM_MASK        0x00FF          /* Thumb BKPT Enum Mask */
#define BKPT16_AUTO_BKPT        0x0080          /* RESERVED: Thumb BKPT Auto-Step Flag (for CONT support) */
#define BKPT16_MANUAL_BKPT      0x007F          /* Manually inserted Thumb Breakpoint */
/*@}*/

#ifndef __ASSEMBLY__

/* Define C stuff */
/** @defgroup debug_public */
/*@{*/


/** Initialize Debugger.
 * 		Equivalent to GDB set_debug_traps() routine
 */
FUNCDEF void dbg__bkpt_init(void);

#ifdef __NXOS__

/** Communication Channel Enums
 *
 * Communication Channel Enums.
 */
ENUM_BEGIN
ENUM_VALASSIGN(COMM_USB, 1)     /**< USB Communications */
ENUM_VAL(COMM_BT)              /**< Bluetooth Communications */
ENUM_END(comm_chan_t)

/** Enable switch to Debugger Mode from NxOS operational mode
 * 		Returns  0 if no mode switch needed (already in Debug mode)
 * 				!0 if mode switch will happen
 * 		Used by NxOS only
 */
/* Note: This platform specific routine is found in debug_runlooptasks.S */
FUNCDEF int nxos__handleDebug(U8 *buffer, comm_chan_t channel, U32 length);
#else
/** Switch Mode to Debugger.
 * 		Used by NXT Firmware only
 */
/* Note: This platform specific routine is found in debug_runlooptasks.S */
FUNCDEF UWORD cCommHandleDebug(UBYTE *pInBuf, UBYTE CmdBit, UWORD MsgLength);
#endif

/** Debugger Handler Routine (called by Exception Handler Trap).
 * 		Equivalent to GDB handle_exception() routine
 */
FUNCDEF void dbg__bkpt_handler(void);

/** dbg_breakpoint_arm.
 * 		Equivalent to GDB breakpoint() routine for ARM code
 */
/* FUNCDEF void dbg_breakpoint_arm(void); */
static inline void dbg_breakpoint_arm(void)
{
  asm volatile (".word %a0"
                  :     /* Output (empty) */
                  : "X" (BKPT32_INSTR | BKPT32_MANUAL_BKPT)
               );
}

#if 0                /* Old asm definitions, in case gas does not recognize %a0 operand */

#ifdef __ARM6OR7__
static inline void dbg_breakpoint_arm(void) { asm volatile (".word 0xE727FF7F" /* (BKPT32_INSTR | BKPT32_MANUAL_BKPT) */ ); }
#else
static inline void dbg_breakpoint_arm(void) { asm volatile (".word 0xE127FF7F" /* (BKPT32_INSTR | BKPT32_MANUAL_BKPT) */ ); }
#endif

#endif

/** dbg_breakpoint_thumb.
 * 		Equivalent to GDB breakpoint() routine for Thumb code
 */
/* FUNCDEF void dbg_breakpoint_thumb(void); */
static inline void dbg_breakpoint_thumb(void)
{
  asm volatile (".hword %a0"
                  :     /* Output (empty) */
                  : "X" (BKPT16_INSTR | BKPT16_MANUAL_BKPT)
               );
}

#if 0                /* Old asm definitions, in case gas does not recognize %a0 operand */

static inline void dbg_breakpoint_thumb(void) { asm volatile (".hword 0xBE7F" /* (BKPT16_INSTR | BKPT16_MANUAL_BKPT) */); }

#endif

/*@}*/

#else
/* Define Assembly stuff */

/* dbg__bkpt_arm
 * 		GDB breakpoint() for ARM mode
 */
 	.macro dbg__bkpt_arm
 	.word	(BKPT32_INSTR | BKPT32_MANUAL_BKPT)
 	.endm

/* dbg__bkpt_thumb
 * 		GDB breakpoint() for Thumb mode
 */
 	.macro dbg__bkpt_thumb
 	.hword	(BKPT16_INSTR | BKPT16_MANUAL_BKPT)
 	.endm

#endif
 /*@}*/

#endif /* __DEBUG_STUB_H__ */
