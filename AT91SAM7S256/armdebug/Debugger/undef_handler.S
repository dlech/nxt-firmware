
/* Copyright (C) 2007-2010 the NxOS developers
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


.text
.code 32
.align 0

	.extern dbg__thumb_bkpt_handler
	.extern dbg__arm_bkpt_handler
	.extern default_undef_handler

	.global undef_handler

undef_handler:
/* Remote GDB Debugger relies on BKPT instruction being trapped here
 * In ARMv4t, it is an Illegal (Undefined) Instruction.
 * On triggering, lr (R14) contains the previous mode's pc (R15).
 * Based on example in Hohl, "ARM Assembly Language: Fundamentals and Techniques"
 * Chapter 11, Example 11.1.
 */
 /* We assume that the DEBUG stack holds only one stack frame and we will overwrite it.
  * On entry, LR_undef points to one instruction past the UNDEF instruction.
  *
  * For the purpose of Debugging, the stack frame should present the PC (R15) as the address
  * of the instruction that triggered the Breakpoint. Hence we need to adjust R15
  * to point to the address of the UNDEF instruction. This is what the JTAG debugger
  * does.
  *
  * We will also store UNDEF LR (next instruction pointer) and UNDEF SPSR to the stack.
  *
  * For the handler, once the user registers have been stored in the DEBUG stack, the
  * registers will be used as follows:
  *
  *		R0: UNDEF LR, then UNDEF instruction address, finally UNDEF instruction word / BKPT index
  * 	R1: SPSR
  * 	R2: Mode
  * 	R3: Debug Stack Pointer (for Banked R13-R14 update)
  */
  		ldr		sp, =__debugger_stack__
 		stmfd 	sp, {r0-r15}^			/* Save workspace, previous mode's pc via 'S' flag, R13-R15: placeholders */
		mov		r3, sp					/* Use R3 to write Banked R13-R14, and actual PC of UNDEF instruction */
 		sub		sp, sp, #(4*16)			/* Need to manually update SP(undef) */

		mov		r0, lr					/* Keep Next Instruction address after UNDEF instruction in R0 */
 		mrs		r1, spsr				/* Copy SPSR to r1 */
 		stmfd	sp!, {r0,r1}			/* Save User's Next Instr Pointer (in UNDEF LR) and previous mode's CPSR to stack */

 		tst		r1, #CPSR_THUMB			/* Check for Thumb Mode */
 		subne	r0, r0, #2				/* Is Thumb instruction, adjust PC for UNDEF instruction address */
 		subeq	r0, r0, #4				/* Is ARM instruction, adjust PC for UNDEF instruction address */
 		str		r0, [r3, #-4]!			/* Save PC to stack (R15 slot) */

 		and 	r2, r1, #CPSR_MODE		/* Get previous mode */
 		teq		r2, #MODE_USR
 		beq		_skip_banked_registers	/* Can't switch back if we're in User mode! */

_store_prev_mode_banked_regs:
		/* FIXME: We don't handle FIQ properly! */

 		orr		r2, #(CPSR_FIQ | CPSR_IRQ)	/* Disable Interrupts */
 		msr		cpsr_c, r2					/* Switch to previous mode */
 		stmfd	r3!, {sp, lr}				/* Store Previous Mode's LR (R14), SP (R13) via R3 */
		msr		cpsr_c, #(MODE_UND | CPSR_FIQ | CPSR_IRQ)	/* Revert to Undef Mode */

_skip_banked_registers:
 		tst		r1, #CPSR_THUMB				/* Check for Thumb Mode */
		beq		_is_arm						/* Clear, so it's ARM mode */
_is_thumb:
		ldrh 	r0, [r0]					/* load UNDEF instruction into r0 */
		ldr		r1, =BKPT16_ENUM_MASK		/* Thumb BKPT enum mask */
		bic		r2, r0, r1					/* leave only opcode */
		ldr		r1, =BKPT16_INSTR			/* check for Thumb Breakpoint Instruction */
		teq		r2, r1
		bne		default_undef_handler
		ldr		r1, =BKPT16_ENUM_MASK		/* get Thumb BKPT Enum Mask */
		ldr		r2, =dbg__thumb_bkpt_handler	/* handle BKPT, BKPT index in r0 */
		b		_exit_undef_handler
_is_arm:
		ldr		r0, [r0]					/* load UNDEF instruction into r0 */
		ldr		r1, =BKPT32_ENUM_MASK		/* ARM BKPT enum mask */
		bic		r2, r0, r1					/* leave only opcode */
		ldr		r1, =BKPT32_INSTR			/* check for ARM Breakpoint Instruction */
		teq		r2, r1
		bne		default_undef_handler
		ldr		r1, =BKPT32_ENUM_MASK		/* get ARM BKPT Enum Mask */
		ldr		r2, =dbg__arm_bkpt_handler	/* handle BKPT, BKPT index in r0 */
_exit_undef_handler:
		and		r0, r1, r0					/* Keep index value */
		msr		cpsr_c, #(MODE_ABT | CPSR_FIQ | CPSR_IRQ)		/* Switch to Abort Mode, Disable Interrupts */
        ldr 	sp, =__abort_stack__		/* Reinitialize stack pointer each time a Breakpoint happens */
        bic 	sp, sp, #7
		mov		pc, r2						/* Invoke Debugger */

    .global resume_execution

resume_execution:
/*
 * This routine is called by the Debugger prior to returning control to
 * the executing program.
 * It updates the SPSR_UNDEF with the Debug Stack value, and
 * restores all registers R0-R14 to the previously active mode.
 * Then, it uses the Next Instruction Address Pointer to return
 * execution control to the previously executing program.
 */
/* On Entry, SP(undef) points to the Next Instruction Address.
 * If the instruction which triggered the Breakpoint need to be
 * reexecuted, it should be placed in the Next Instruction Address slot
 * by ABORT mode before coming here
 */
    ldr     lr, =__debugger_stack_bottom__      /* Use LR(undef) for Debug Stack Access */
    add     r1, lr, #(DBGSTACK_USERSP_INDEX*4)  /* Use R1 for Previous Mode SP (R13) and LR (R14) access */
    ldr     r0, [lr, #(DBGSTACK_USERCPSR_INDEX*4)]!      /* LR updated, Retrieve SPSR into R0 */
    msr     spsr, r0                    /* Update SPSR for return to program being debugged */
    and     r0, r0, #CPSR_MODE          /* Get previous mode */
    teq     r0, #MODE_USR
    bne     _restore_prev_mode_banked_regs      /* Can't switch back if we're in User mode! */

    /* Previous mode was User Mode */
    ldmed   lr, {r0-r14}^               /* We use LDMED since LR is pointing to USERCPSR not R0 */
    b       _really_resume_execution

_restore_prev_mode_banked_regs:
    /* FIXME: We don't handle FIQ properly! */
    orr 	r0, #(CPSR_FIQ | CPSR_IRQ)  /* Disable Interrupts */
    msr 	cpsr_c, r0                  /* Switch to previous mode */
    ldmfd   r1, {sp, lr}            /* Restore Previous Mode's LR (R14), SP (R13) via R1 */
    msr 	cpsr_c, #(MODE_UND | CPSR_FIQ | CPSR_IRQ)   /* Revert to Undef Mode */
    ldmed   lr, {r0-r12}               /* We use LDMED since LR is pointing to USERCPSR not R0 */

_really_resume_execution:
    ldmfd   sp, {pc}^               /* Exit to Previous Mode using Next Instruction Address */




