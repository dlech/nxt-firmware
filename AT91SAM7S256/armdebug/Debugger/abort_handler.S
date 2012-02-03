
/* Copyright (C) 2007-2011 the NxOS developers
 *
 * Module Developed by: TC Wan <tcwan@cs.usm.my>
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */
#define __ASSEMBLY__
#include "debug_stub.h"
#include "debug_internals.h"

#define PREFETCH_OFFSET	4
#define DATA_OFFSET		8

/* Trap Abort Exceptions.
 * On triggering, lr (R14) contains the previous mode's pc (R15).
 * Based on example in Hohl, "ARM Assembly Language: Fundamentals and Techniques"
 * Chapter 11, Example 11.1.
 */
/*
 * NOTE: This routine closely mirrors the undef_handler routine, since we will store
 *       the ABORT stack frame in the UNDEF stack.
 *       In addition, since ARMDEBUG uses Abort mode, if the ABORT occurs while the
 *       debugger is running, the value of SP_abort is not valid. This should not happen
 *       in any case (it is a BUG if ARMDEBUG triggers an ABORT).
 *
 * We assume that the DEBUG stack holds only one stack frame and we will overwrite it.
 * On entry, LR_undef contains the PC+4 for Prefetch Abort, and PC+8 for Data Abort.
 *
 * For the purpose of Debugging, the stack frame should present the PC (R15) as the address
 * of the instruction that triggered the Abort. Hence we need to adjust R15
 * to point to the address of the ABORTed instruction.
 *
 * We will also store ABORT LR (next instruction pointer) and ABORT SPSR to the stack.
 *
 * For the handler, once the user registers have been stored in the DEBUG stack, the
 * registers will be used as follows:
 *
 *		R0: ABORT LR, then ABORT instruction address
 * 		R1: SPSR
 * 		R2: PC Offset, then Mode
 * 		R3: DEBUG Stack Pointer (for Banked R13-R14 update)
 *		R4: Abort Type Enum
 */

.text
.code 32
.align 0

	.extern dbg__display_abort_info
	.extern dbg__abort_exception_handler
	.extern default_prefetch_abort_handler
	.extern default_data_abort_handler

	.global prefetch_abort_handler
	.global data_abort_handler

prefetch_abort_handler:
  		ldr		sp, =__debugger_stack__
 		stmfd 	sp, {r0-r15}^			/* Save workspace, previous mode's pc via 'S' flag, R13-R15: placeholders */
		mov		r2, #PREFETCH_OFFSET
		mov		r4, #DISP_ABORT_PREFETCH /* Display Abort Info Type */
		mov		r5, #DBG_ABORT_PREFETCH	/* Debugger Abort Type */
		b		_common_abort_handler

data_abort_handler:
  		ldr		sp, =__debugger_stack__
 		stmfd 	sp, {r0-r15}^			/* Save workspace, previous mode's pc via 'S' flag, R13-R15: placeholders */
		mov		r2, #DATA_OFFSET
		mov		r4, #DISP_ABORT_DATA	/* Display Abort Info Type */
		mov		r5, #DBG_ABORT_DATA		/* Debugger Abort Type */

_common_abort_handler:
		sub		r0, lr, r2				/* R0: Adjust PC to ABORTed instruction address */
 		str		r0, [sp, #-4]			/* Save ABORTed Instruction PC to stack (R15 slot) */
		sub		r3, sp, #4				/* Use R3 to write Banked R13-R14 of ABORT instruction, update R3 to point to R14 slot */

 		mrs		r1, spsr				/* Copy SPSR to r1 */
 		tst		r1, #CPSR_THUMB			/* Check for Thumb Mode */
		addne	r0, r0, #2				/* Is Thumb instruction, adjust PC for ABORT next instruction address */
		addeq	r0, r0, #4				/* Is ARM instruction, adjust PC for ABORT next instruction address */
 		sub		sp, sp, #(4*16)			/* Need to manually update SP(abort) */
 		stmfd	sp!, {r0,r1}			/* Save ABORTed Next Instr Pointer (in R0) and previous mode's CPSR to stack */

 		and 	r2, r1, #CPSR_MODE		/* Get previous mode */
 		teq		r2, #MODE_USR
 		beq		_exit_abort_handler		/* Can't switch back if we're in User mode! */

_store_prev_mode_banked_regs:
		/* FIXME: We don't handle FIQ properly! */

 		orr		r2, #(CPSR_FIQ | CPSR_IRQ)	/* Disable Interrupts */
 		msr		cpsr_c, r2					/* Switch to previous mode */
 		stmfd	r3!, {sp, lr}				/* Store Previous Mode's LR (R14), SP (R13) via R3 */
		msr		cpsr_c, #(MODE_ABT | CPSR_FIQ | CPSR_IRQ)	/* Revert to ABORT Mode */

_exit_abort_handler:
        ldr 	sp, =__abort_stack__		/* Reinitialize stack pointer each time an Abort happens */
        bic 	sp, sp, #7
        mov		r0, r4						/* Copy Display Abort Type Enum to R0 */
		bl		dbg__display_abort_info		/* Display Abort Type to LCD */
		mov		r0, r5						/* Copy Debugger Abort Type Enum to R0 */
        b		dbg__abort_exception_handler	/* Invoke Debugger */


