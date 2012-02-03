/** @file debug_test.S
 *  @brief Test Routines to trigger ARM and Thumb Manual Breakpoints
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
#define __ASSEMBLY__
#include "debug_stub.h"

.text
.align 4
.code 32

/**********************************************
 * dbg__test_arm_bkpt Test Routine
 *
 */
	.global dbg__test_arm_bkpt
dbg__test_arm_bkpt:
	stmfd sp!,{lr}
	dbg__bkpt_arm                  /* Trigger ARM Manual Breakpoint */
    ldmfd sp!,{pc}


/**********************************************
 * dbg__test_arm_instrstep Test Routine
 *	Used to test GDB Stepping command
 *	This routine exercises the mov, add, ldr, ldm, b, bl,
 *	and bx instructions which modify the PC (R15)
 *	In addition, conditional instructions are also evaluated.
 *
 */
	.global dbg__test_arm_instrstep
dbg__test_arm_instrstep:
	stmfd	sp!, {lr}
	bl		dbg__test_arm_instr_sub1
	ldr		r1, =test_arm_3			/* R1: pointer to test_arm_3 */
	ldr		r2, =test_arm_2			/* R2: pointer to test_arm_2 */
	mov		pc, r1

test_arm_1:
	subs	r0, r0, #1
	addne	pc, r2, #4				/* If R0 > 0, keep branching to a new location */
	/* else R0 == 0 */
	b		exit_dbg__test_arm_instrstep

test_arm_2:
	sub		r0, r0, #1
	cmp		r0, #5
	bgt		test_arm_1
	ldrle	pc, =exit_dbg__test_arm_instrstep
	b		exit_dbg__test_arm_instrstep

test_arm_3:
	sub		r0, r0, #1
	teq		r0, #8
	beq		test_arm_1
	ldrne	r3, =test_arm_3
	bx		r3

exit_dbg__test_arm_instrstep:
	bl		dbg__test_thumb_instr_sub1
	ldmfd	sp!, {pc}

	.global dbg__test_arm_instr_sub1
dbg__test_arm_instr_sub1:
	mov		r0, #10
	bx		lr

	.global dbg__test_arm_instr_sub1
dbg__test_arm_instr_sub2:
	stmfd	sp!, {r4, lr}
	mov		r0, #TRUE
	ldmfd	sp!, {r4, pc}

/**********************************************
 * dbg__test_thumb_bkpt Test Routine
 *
 */
	.global dbg__test_thumb_bkpt
dbg__test_thumb_bkpt:
	stmfd sp!,{lr}
/*	ldr r0, =_thumb_entry
	orr	r0, r0, #1					@ Set Thumb mode
	mov	lr, pc
	bx	r0
*/
    bl  _thumb_entry
	ldmfd sp!,{pc}

.code 16
.thumb_func
_thumb_entry:
	dbg__bkpt_thumb
	bx	lr


/**********************************************
 * dbg__test_thumb_instrstep Test Routine
 *	Used to test GDB Stepping command
 *
 */
	.global dbg__test_thumb_instrstep
.thumb_func
dbg__test_thumb_instrstep:
	push	{lr}
	bl		dbg__test_thumb_instr_sub1
	bl		dbg__test_thumb_instr_sub2

test_thumb_1:
	sub		r0, #1
	bne		test_thumb_2
	/* else R0 == 0 */
	b		exit_dbg__test_thumb_instrstep

test_thumb_2:
	sub		r0, #1
	cmp		r0, #5
	bls		test_thumb_1
	bhi		test_thumb_3
	beq		test_thumb_2
	b		test_thumb_1

test_thumb_3:
	sub		r0, #1
	cmp		r0, #0xB
	blo		load_test_thumb_1
	ldr		r2, =test_thumb_3+1		/* Need to set Thumb bit */
	b		exit_test_thumb_3
load_test_thumb_1:
	ldr		r2, =test_thumb_1+1		/* Need to set Thumb bit */
exit_test_thumb_3:
	bx		r2

exit_dbg__test_thumb_instrstep:
	bl		dbg__test_arm_instr_sub1
	pop		{r1}
	bx		r1

.thumb_func
dbg__test_thumb_instr_sub1:
	mov		r0, #0x0F
	bx		lr

.thumb_func
dbg__test_thumb_instr_sub2:
	push	{lr}
	mov		r1, #FALSE
	pop		{pc}


.end
