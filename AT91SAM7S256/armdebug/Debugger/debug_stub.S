/** @file debug_stub.S
 *  @brief ARM Breakpoint Debugger support routines
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

 /* GDB sparc-stub.c comments header included below to document GDB Server Remote protocol */
 /* This header has been modified to include additional commands not documented in the header stub */

 /****************************************************************************

		THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or it's performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for SPARC by Stu Grossman, Cygnus Support.
 *
 *  This code has been extensively tested on the Fujitsu SPARClite demo board.
 *
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps() is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint().  Breakpoint()
 *  simulates a breakpoint by executing a trap #1.
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    GrrrrRRRR..   set the value of the CPU registers     OK or ENN
 *					where register values are given as
 *					32-bit hex values in the sequence:
 *					User R0, R1, ..., R15, CPSR
 *	  px			get the value of one register (x)      hex data or ENN
 *	  Px=rrrr		set the value of one register (x) to   OK or ENN
 *					32-bit hex value rrrr.
 *					x = ['0','F'] for R0-R15, ['10','17'] for F0-F7 (dummy)
 *                  '18' for FPSCR (dummy), '19' for User CPSR
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL:bb..bb
 *                  Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    D             Detach (equivalent to continue         Ack Only
 *                  at current address)
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 *    zt,AA..AA,k	Remove a Breakpoint of type t at addr  OK or ENN
 *                  AA..AA of kind k
 *    Zt,AA..AA,k	Insert a Breakpoint of type t at addr  OK or ENN
 *                  AA..AA of kind k
 *                  t 0: memory breakpoint
 *                    1: hardware breakpoint
 *                    2: write watchpoint
 *                    3: read watchpoint
 *                    4: access watchpoint
 *                  k: 2 (16-bit Thumb), 3 (32-bit Thumb2)
 *                       or 4 (32-bit ARM) for t=[0,1]
 *                     Num. bytes to watch for t=[2,4]
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/
 /* Modified GDB Server Remote Protocol definition from GDB's sparc-stub.c Comment Header included above
  * Additional commands from GDB Reference Appendix D.2
  *
  * Note: ARMDEBUG can only implement Memory Breakpoints t=0. Hardware breakpoints requires a JTAG debugger.
  *       Currently, watchpoints are not implemented as they require hardware support as well (need verification).
  *
  * GDB requires command parameters to be specified as Big Endian values.
  * However, the read/write register command expect the register contents to be specified in target byte order.
  * The default target byte order is Little Endian for the AT91SAM7xxx processor in the NXT.
  * If Big Endian target byte order is required, the __BIG_ENDIAN__ preprocessor label should be defined.
  */

/* FIXME: The Hex value arguments passed by GDB does not have fixed lengths! Although the standard says
 *        there should be x digits, it does not follow this requirement. e.g., register index.
 */


#define __ASSEMBLY__
#include "debug_stub.h"
#include "debug_internals.h"
#include "debug_macros.h"

	/* Opcode Parser function reference */
	.extern dbg_following_instruction_addr

	/* Hexutils function references */
	.extern	hex2char
	.extern	char2hex
	.extern	byte2ascii
	.extern	halfword2ascii_be
    .extern halfword2ascii_le
	.extern	word2ascii_be
    .extern word2ascii_le
    .extern ascii2hex_varlen_be
	.extern	ascii2byte
	.extern	ascii2halfword_be
    .extern ascii2halfword_le
	.extern	ascii2word_be
    .extern ascii2word_le

/* Macro definitions */

/* _check_msgseparator
 *      Look for separator ','
 *  On entry:
 *    bufferptr: points to the parameter buffer [can't be R0]
 *  On exit:
 *    R0: destroyed
 *    bufferptr: points to the next character location in the parameter buffer
 *    Flags: Updated
 */

    .macro  _check_msgseparator bufferptr
    ldrb    r0, [\bufferptr], #1                /* get separator */
    cmp     r0, #MSGBUF_SEPCHAR
    .endm

/* _check_msgargument
 *      Look for argument ':'
 *  On entry:
 *    bufferptr: points to the parameter buffer [can't be R0]
 *  On exit:
 *    R0: destroyed
 *    bufferptr: points to the next character location in the parameter buffer
 *    Flags: Updated
 */

    .macro  _check_msgargument bufferptr
    ldrb    r0, [\bufferptr], #1                /* get separator */
    cmp     r0, #MSGBUF_ARGCHAR
    .endm

/* _check_msgassignment
 *      Look for assignment '='
 *  On entry:
 *    bufferptr: points to the parameter buffer [can't be R0]
 *  On exit:
 *    R0: destroyed
 *    bufferptr: points to the next character location in the parameter buffer
 *    Flags: Updated
 */

    .macro  _check_msgassignment bufferptr
    ldrb    r0, [\bufferptr], #1                /* get separator */
    cmp     r0, #MSGBUF_SETCHAR
    .endm

.bss
.align 4
debug_InMsgBuf:
	.space	MSGBUF_SIZE,0
debug_OutMsgBuf:
	.space	MSGBUF_SIZE,0

	/* Make Debugger State accessible from other modules */
	.global debug_state
	.global debug_mode
	.global debug_bkpt_type
	.global debug_curr_breakpoint

debug_state:
    .byte   0x0									/* dbg_state_t variable */
debug_mode:
    .byte   0x0                                 /* Boolean variable */
debug_bkpt_type:
    .byte   0x0                                 /* bkpt_type_t variable */
debug_curr_breakpoint:
    .byte   0x0

.data
.align 4
debug_RetransmitFlag:
	.byte	'-',0

debug_AckOnlyFlag:
	.byte	'+',0

debug_ValidResponsePrefix:
	.byte	'+','$',0

debug_ErrorResponsePrefix:
	.byte	'+','$','E',0

debug_SignalResponsePrefix:
	.byte	'+','$','S',0

debug_OkResponse:
	.byte	'+','$','O','K',0

debug_ThreadIDResponse:
	.byte	'+','$','Q','C','0',0			/* 0: Any thread */

debug_FirstThreadInfoResponse:
	.byte	'+','$','m','0',0				/* 0: One default thread */
debug_SubsequentThreadInfoResponse:
	.byte	'+','$','l',0					/* End of Thread List */

/* The CmdIndexTable and CmdJumpTable must be kept in sync */
debug_cmdIndexTable:
	.byte	'g','G','p','P','m','M','D','c','s','k','z','Z','?','q','Q',0

/* Command Handlers
 * On entry:
 *		R0: Input Message Parameter Buffer address pointer (points to contents after '$' and '<cmdchar>')
 */
.align 4
debug_cmdJumpTable:
	.word	_dbg__cmd_GetAllRegs				/* 'g' */
	.word	_dbg__cmd_SetAllRegs	   			/* 'G' */
	.word	_dbg__cmd_GetOneReg				    /* 'p' */
	.word	_dbg__cmd_SetOneReg				    /* 'P' */
	.word	_dbg__cmd_ReadMem					/* 'm' */
	.word	_dbg__cmd_WriteMem					/* 'M' */
	.word	_dbg__cmd_Detach					/* 'D' */
	.word	_dbg__cmd_Continue					/* 'c' */
#ifdef __NXOS__
	.word	_dbg__cmd_Step						/* 's' */
#else
	/* NXT Firmware does not support Stepping */
	.word	_dbg__nop							/* 's' */
#endif
	.word	_dbg__cmd_Kill						/* 'k' */
	.word	_dbg__cmd_RemoveBreakpoint			/* 'z' */
	.word	_dbg__cmd_InsertBreakpoint			/* 'Z' */
	.word	_dbg__cmd_Status					/* '?' */
	.word	_dbg__cmd_Query						/* 'q' */
	.word	_dbg__nop							/* 'Q' */
	.word	0

.code 32
.text
.align 	4
	.extern __breakpoints_num__
	.extern dbg__getDebugMsg			/* Read a message from the communications link */
	.extern	dbg__putDebugMsg			/* Write a message to the communications link */
	.extern dbg__sendAckOrNak			/* Send Ack or Nak to indicate success/failure of message receipt */
	.extern dbg__runloopTasks           /* Platform specific Run Loop processing */


/* The Debugger Interface can handle a total of (n-1) Breakpoint States and 1 Single Stepping State,
 * where n is a power of 2. The value of n is given by __breakpoints_num__ defined in the linker file.
 *
 * In addition, a Debugger Stack contains the User Mode Register Stack Frame + SPSR + Bkpt Instr Addr.
 * These are currently stored in the .stack area in RAM, so there is no fixed address
 * location that is used for this purpose.
 *
 * The Breakpoint feature assumes that the program is executed in RAM. It is not possible
 * to set dynamic breakpoints for programs executed from Flash in the AT91SAM7S which lacks
 * instruction breakpointing support in hardware without using JTAG. The only type of breakpoints
 * that can be supported in Flash based programs are Static (predefined) breakpoints inserted into
 * the code.
 *
 * Each Breakpoint State i is a struct comprising the Breakpoint Address + Memory Contents
 * stored in 8 bytes as:
 *    [High Memory Address]
 *		ADDR [i*8+4]: Memory Contents		(32 bits)
 *		ADDR [i*8]:   Breakpoint Address	(31 bits, b0 = THUMB flag [not implemented yet])
 *	  [Low Memory Address]
 *
 * A Non-zero Breakpoint Address means that the breakpoint is active, whereas the memory contents
 * contains the instruction which resided at that address initially (now replaced by a BKPT <index>
 * instruction).
 * Note: Currently it is not possible to resume execution of a program with breakpoints enabled
 * after a RESET, since the RESET will clear all contents of the stack, destroying the instruction
 * contained in a given breakpoint.
 * Fortunately the NXT will also need to reload the program into RAM so this is not expected to be
 * an issue.
 *
 * The Memory Map for the Debugger State is as follows:
 *
 *	[High Memory Address]			__breakpoints_end__
 *		Breakpoint 07 State
 *		Breakpoint 06 State
 *			...
 *		Breakpoint 02 State
 *		Breakpoint 01 State
 *		Single Step   State			__debugger_stack__ / __breakpoints_start__
 *		Previous Mode R15
 *		Previous Mode R14
 *		Previous Mode R13
 *			...
 *		User Mode R02
 *		User Mode R01
 *		User Mode R00
 *		Previous Mode CPSR (UNDEF SPSR)
 *		UNDEF Next Instr Addr 		__debugger_stack_bottom__
 *  [Low Memory Address]
 *
 *	Each Breakpoint State will initially be zeroed.
 *
 */
 /* FIXME: The Debugger Stack Frame is probably not 100% consistent with the order that
    GDB expects in the g/G messages. CSPR is probably located above R15 */

/****************************************************************************
 *
 * GDB Debugger Init and Breakpoint Handler Routines
 *
 ****************************************************************************/
    .code 32
    .align 4
/* dbg__bkpt_init
 *		GDB set_debug_traps() routine
 *		On entry:
 *			None
 *		On exit:
 *			r0-r3: destroyed
 */
 	.global dbg__bkpt_init
dbg__bkpt_init:
    push    {lr}
	bl		_dbg__clear_breakpoints
	mov		r2, #0
	ldr		r1, =debug_curr_breakpoint
	strb	r2, [r1]
	ldr		r0, =debug_InMsgBuf
	strb	r2, [r0]
	ldr		r1, =debug_OutMsgBuf
	strb	r2, [r1]
	bl		dbg__comm_init				/* Pass R0: Rx Buffer, R1: Tx Buffer to comm submodule */

/* FIXME: Initialize other stuff here */
	_dbg_setstate DBG_INIT
    _dbg_setmode FALSE                  /* Debug Mode = False */
    pop     {lr}
    bx      lr                          /* Must return via BX; may have been called from Thumb mode (NXT Firmware) */


/* _dbg__flush_icache
 *		Flush the Instruction cache
 *		Defined by GDB Stub, but not needed for ARMv4T architecture
 */
_dbg__flush_icache:
	/* nop */
	bx	lr

/* _dbg__switch2undefmode
 *      Common internal routine to return execution to user program
 */
_dbg__switch2undefmode:
    bl  	_dbg__flush_icache
    msr     cpsr_c, #(MODE_UND | CPSR_FIQ | CPSR_IRQ)         /* Configure Undef Mode */
    _dbg_setmode FALSE                  	/* Debug Mode = False */
    ldr     lr, =resume_execution
    mov     pc, lr                          /* Exit via UNDEF mode */

/* dbg__abort_exception_handler
 *		Handle Abort Exceptions
 *		On entry:
 *			r0: Abort Type Enum
 *		On exit:
 *			routine does not 'exit' in the normal sense
 */
	.global	dbg__abort_exception_handler
dbg__abort_exception_handler:
	_dbg_set_bkpt_type r0			/* Set Breakpoint Type given value in R0 */
	b	dbg__bkpt_waitCMD

/* dbg__thumb_bkpt_handler
 *		GDB handle_exception() routine (Thumb Mode)
 *		On entry:
 *			r0: Breakpoint index value
 *		On exit:
 *			routine does not 'exit' in the normal sense
 */
	.global	dbg__thumb_bkpt_handler
dbg__thumb_bkpt_handler:
	/* On entry, r0 contains breakpoint index value */
	_dbg_setcurrbkpt_index r0			/* keep current breakpoint index in memory */
	ldr		r1, =BKPT16_MANUAL_BKPT
	teq		r0, r1
	moveq	r0, #DBG_MANUAL_BKPT_THUMB	/* Manual Thumb Breakpoint, process it */
	beq		_restore_normal_breakpoints

_process_normal_breakpoint_thumb:
	ldr		r1, =__breakpoints_num__
	cmp		r0, r1						/* Sanity check that index is in range */
	bhs		dbg__bkpt_offset_outofrange	/* Exceeded Offset Range */
	mov		r0, #DBG_NORMAL_BKPT_THUMB	/* It is a valid normal breakpoint */
	b		_restore_normal_breakpoints

/* dbg__arm_bkpt_handler
 *		GDB handle_exception() routine (ARM Mode)
 *		On entry:
 *			r0: breakpoint index value
 *		On exit:
 *			routine does not 'exit' in the normal sense
 */
	.global	dbg__arm_bkpt_handler
dbg__arm_bkpt_handler:
	/* On entry, r0 contains breakpoint index value */
	_dbg_setcurrbkpt_index r0			/* keep current breakpoint index in memory */
	ldr		r1, =BKPT32_MANUAL_BKPT
	teq		r0, r1
	moveq	r0, #DBG_MANUAL_BKPT_ARM	/* Manual ARM Breakpoint, process it */
	beq		_restore_normal_breakpoints

_process_normal_breakpoint_arm:
	ldr		r1, =__breakpoints_num__
	cmp		r0, r1						/* Sanity check that index is in range */
	bhs		dbg__bkpt_offset_outofrange	/* Exceeded Offset Range */
	mov		r0, #DBG_NORMAL_BKPT_ARM	/* It is a valid normal breakpoint */
/*	b		_restore_normal_breakpoints */

_restore_normal_breakpoints:
	_dbg_set_bkpt_type r0			/* Set Breakpoint Type given value in R0 */
	bl	_dbg__restore_breakpoints	/* includes restoring single step */
	bl	_dbg__clear_singlestep
	bl	_dbg__flush_icache
/*	b	dbg__bkpt_waitCMD */

dbg__bkpt_inactive:
/*	b	dbg__bkpt_waitCMD */

dbg__bkpt_offset_outofrange:
/*	b	dbg__bkpt_waitCMD */

	.global	dbg__bkpt_waitCMD
/* dbg__bkpt_waitCMD
 *		GDB Stub Remote Command Handler
 */

/****************************************************************************
 *
 * GDB Server Command Processing Routines
 *
 ****************************************************************************/
dbg__bkpt_waitCMD:
	/* We enter this code section when a Breakpoint Triggers */
    _dbg_setmode TRUE                       /* Debug Mode = True */
	msr	cpsr_c, #(MODE_ABT)					/* Re-enable Interrupts */

	_dbg_getstate r0
	cmp		r0, #DBG_CONFIGURED
	blo		dbg__bkpt_waitCMD_cont			/* Not configured yet, don't send Breakpoint Signal Response */
	bl		_dbg__cmd_Status				/* Send signal response to the GDB server */

dbg__bkpt_waitCMD_cont:
	bl      dbg__runloopTasks               /* Execute housekeeping tasks while in ABRT mode */
	bl		dbg__getDebugMsg				/* Read new message from Debugger, buflen in R0, 0 if none, -1 if error, msgbuf pointer in R1 */
	cmp		r0, #0
	beq     dbg__bkpt_waitCMD_cont          /* No message yet, do housekeeping tasks */
	bgt		_proc_command					/* valid message, process it */
	bl		__dbg__procChecksumError		/* Message invalid, checksum error? */
	b		dbg__bkpt_waitCMD_cont

_proc_command:
/* Message now has Ctrl-C or $<packet info>\0 */
    mov     r4, r1							/* Use R4 as buffer pointer */
	ldrb	r0, [r4], #1					/* Look for Ctrl-C or '$' */
	teq		r0, #MSGBUF_CTRLC
	bne		_dbg_check_gdb_command
	/* Ctrl-C detected, do nothing (wait for next command from GDB) */
#if 0
	/* On entering the Debugger via Ctrl-C, _dbg__cmd_Status has already sent a reply, so just keep quiet */
	bl		__dbg__procAckOnly				/* send Ack */
#endif
	b		dbg__bkpt_waitCMD_cont

_dbg_check_gdb_command:
	teq		r0, #MSGBUF_STARTCHAR
	movne	r1, #MSG_ERRFORMAT				/* Message Format invalid (not '$') */
	bne		_dbg__cmdError					/* Shouldn't happen */
	ldrb	r0, [r4], #1					/* Look for command char */
	bl		_dbg__cmdChar2Index				/* Index in R0 */
	mov		r1, #CMDINDEX_OUTOFRANGE
	teq		r0, r1
	bne		_dbg__cmdExists					/* Found valid command, execute it */
_dbg_unknown_command:
	bl		_dbg__nop						/* Command character not recognized, send empty response to GDB server */
	b 		dbg__bkpt_waitCMD_cont

#if 0
	moveq	r1, #MSG_UNKNOWNCMD				/* Out of range, Command character not recognized */
	beq		_dbg__cmdError					/* Send response to GDB server */
#endif

_dbg__cmdExists:
	mov		r3, r0							/* put Command Handler Index in R3 */
	mov		r0, r4							/* R0 now contains Input Message Buffer Parameter Pointer (previously in R4) */
	_dbg_jumpTableHandler	debug_cmdJumpTable, r2, r3		/* Call Command Handler Routine, use R2 as jump address pointer */
	b 		dbg__bkpt_waitCMD_cont

_dbg__cmdError:
	_dbg_outputMsgStatusErr
	bl		dbg__putDebugMsg				/* Send error response to the GDB server */
	b 		dbg__bkpt_waitCMD_cont


/* __dbg__procOkOnly
 *		Send OK to GDB due to Detach
 *		On entry:
 *			None
 *		On exit:
 *			r0-r3: destroyed
 */
__dbg__procOkOnly:
	stmfd	sp!, {lr}
	_dbg_outputMsgStatusOk					/* Acknowledge Detach command from GDB server */
_cont_procOkOnly:
	bl      dbg__putDebugMsg                /* Send OK response to the GDB server */
	cmp		r0, #0
	beq		_cont_procOkOnly_exit			/* Sending of OK succeeded */
	bl      dbg__runloopTasks               /* Service run loop tasks */
	b 		_cont_procOkOnly				/* Retry OK transmission */
_cont_procOkOnly_exit:
    ldmfd   sp!, {pc}


/* __dbg__procAckOnly
 *		Send Ack to GDB due to Continue or Step
 *		On entry:
 *			None
 *		On exit:
 *			r0-r3: destroyed
 */
__dbg__procAckOnly:
	stmfd	sp!, {lr}
    _dbg_outputAckOnlyFlag
	b		_cont_sendAckOrNak				/* Acknowledge Continue or Step command from GDB server */

/* __dbg__procChecksumError
 *		Request Retransmission from GDB due to Checksum error
 *		On entry:
 *			None
 *		On exit:
 *			r0-r3: destroyed
 */
__dbg__procChecksumError:
	stmfd	sp!, {lr}
    _dbg_outputRetransmitFlag
	/* b		_cont_sendAckOrNak	*/		/* Acknowledge Continue or Step command from GDB server */

_cont_sendAckOrNak:
	bl		dbg__sendAckOrNak				/* send Ack or Nak to GDB server */
	cmp		r0, #0
	beq		_cont_sendAckOrNak_exit			/* Sending of Ack or Nak succeeded */
	bl      dbg__runloopTasks               /* Service run loop tasks */
	b 		_cont_sendAckOrNak				/* Retry Ack or Nak transmission */
_cont_sendAckOrNak_exit:
    ldmfd   sp!, {pc}

/* _dbg__cmdChar2Index
 *		Convert Command Character to Jump Table Index
 *		On entry:
 *			r0: command character
 *		On exit:
 *			r0: jump table index (-1 for command not found)
 *			R1: destroyed
 *			R2: destroyed
 *			R3: destroyed
 */
_dbg__cmdChar2Index:
	mov		r1, r0							/* Copy command character to r1 */
	mov		r0, #0							/* Clear return value */
	ldr		r3, =debug_cmdIndexTable		/* Convert command to index using r3 as Index Lookup Address Pointer */
1:	ldrb	r2, [r3, r0]					/* Get table entry */
	teq		r2, #0
	moveq	r0, #CMDINDEX_OUTOFRANGE	    /* End of Index Table, Not found */
	beq		_exit_cmdIndexTable
	teq		r1, r2
	addne	r0, #1							/* Increment Index */
	bne		1b								/* No match, skip to next command char */
_exit_cmdIndexTable:
	bx		lr

/* __dbg__cmdParamLen
 *   Determines the length of the parameter buffer for a given command
 *   On entry:
 *      R0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *   On exit:
 *      R0: Address of parameter buffer (preserved)
 *      R1: length
 */
__dbg__cmdParamLen:
    stmfd   sp!, {r0,r2,lr}     /* R2: scratch register */
    mov     r1, #0
1:  ldrb    r2, [r0], #1
    teq     r2, #0
    addne   r1, r1, #1
    bne     1b
    ldmfd   sp!, {r0,r2,pc}

/* __dbg__procCmdOk
 *      Common subroutine exit stub to return Command Ok Status for Command Handlers
 *      DO NOT CALL THIS STUB DIRECTLY! It Assumes that the return address is in the stack.
 *
 */
__dbg__procCmdOk:
    _dbg_outputMsgStatusOk
    b       __dbg__sendDebugMsgExit

/* __dbg__procUnimplementedError
 *      Common subroutine exit stub to handle Unimplemented GDB Command Error
 *      DO NOT CALL THIS STUB DIRECTLY! It Assumes that the return address is in the stack.
 *		Note: GDB Remote Protocol specifies returning blank message ('+$')
 *
 */
__dbg__procUnimplementedError:
    _dbg_outputMsgValidResponse
    b       __dbg__sendDebugMsgExit

/* __dbg__procCmdParamError
 *		Common subroutine exit stub to handle Command Parameter Error for Command Handlers
 *		DO NOT CALL THIS STUB DIRECTLY! It Assumes that the return address is in the stack.
 *
 */
__dbg__procCmdParamError:
    mov r1, #MSG_UNKNOWNPARAM
    b   __dbg__procErrorMsg

/* __dbg__procCmdReturnInputLengthError
 *      Common subroutine exit stub to handle Command Input Length Error for Command Handlers
 *      DO NOT CALL THIS STUB DIRECTLY! It Assumes that the return address is in the stack.
 *
 */
__dbg__procCmdReturnInputLengthError:
    mov r1, #MSG_ERRINLENGTH
    b   __dbg__procErrorMsg

/* __dbg__procCmdReturnOutputLengthError
 *      Common subroutine exit stub to handle Command Return Output Error for Command Handlers
 *      DO NOT CALL THIS STUB DIRECTLY! It Assumes that the return address is in the stack.
 *
 */
__dbg__procCmdReturnOutputLengthError:
    mov r1, #MSG_ERROUTLENGTH
    b   __dbg__procErrorMsg

/* __dbg__procBreakpointAddrError
 *      Common subroutine exit stub to handle Breakpoint Address Error for Breakpoint Insert/Remove Handlers
 *      DO NOT CALL THIS STUB DIRECTLY! It Assumes that the return address is in the stack.
 *
 */
__dbg__procBreakpointAddrError:
    mov r1, #MSG_UNKNOWNBRKPT
/*    b   __dbg__procErrorMsg */


__dbg__procErrorMsg:
    _dbg_outputMsgStatusErr
    /* b       __dbg__sendDebugMsgExit */

__dbg__sendDebugMsgExit:
    bl      dbg__putDebugMsg                /* Send error response to the GDB server */
    ldmfd   sp!, {pc}

/* _dbg__cmd_Status
 *		Status Command Handler
 *		On entry:
 *			r0: parameter buffer (contents after '$' and '<cmdchar>')
 *      On exit:
 *          r0, r1, r2, r3: destroyed
 */
_dbg__cmd_Status:
	stmfd	sp!, {lr}
    _dbg_get_bkpt_type   r0
_check_data_abort_exception:
	teq		r0, #DBG_ABORT_DATA
	moveq	r1, #MSG_SIG_BUS					/* Bus Error */
	beq		_exit_dmg__cmd_Status
_check_prefetch_abort_exception:
	teq		r0, #DBG_ABORT_PREFETCH
	moveq	r1, #MSG_SIG_ABRT					/* FIMXE: Look for a better Signal number */
	beq		_exit_dmg__cmd_Status
_default_breakpoint_exception:
	mov		r1, #MSG_SIG_DEFAULT				/* Dummy Signal number */

_exit_dmg__cmd_Status:
	_dbg_outputMsgStatusSig
	b	__dbg__sendDebugMsgExit

/* _dbg__cmd_Query
 *		Query Command Handler
 *		On entry:
 *			r0: parameter buffer (contents after '$' and '<cmdchar>')
 *				[varied, see GDB General Packets query docs]
 *				http://sourceware.org/gdb/current/onlinedocs/gdb/General-Query-Packets.html
 *      On exit:
 *          r0, r1, r2, r3: destroyed
 */
_dbg__cmd_Query:
	stmfd	sp!, {r0,r1, lr}
	_dbg_setstate	DBG_CONFIGURED			/* We have exchanged query messages with the GDB server */
	ldmfd	sp!, {r0, r1}					/* Restore parameters needed for subsequent processing */
	bl      __dbg__cmdParamLen
	cmp		r1, #CMD_QUERY_MINPARAMLEN
	beq		_dbg__cmd_Query_default

	ldrb	r2, [r0]						/* Get First Query Param Char */
_dbg__cmd_Query_check_C:
	teq		r2, #CMD_QUERY_CURRTID_CHAR		/* Handle Current Thread ID Query */
	bne		_dbg__cmd_Query_check_fThreadInfo
	cmp		r1, #CMD_QUERY_CURRTID_PARAMLEN
	bne		_dbg__cmd_Query_default
	_dbg_outputMsgCurrTID
	b	__dbg__sendDebugMsgExit
_dbg__cmd_Query_check_fThreadInfo:
	teq		r2, #CMD_QUERY_FTINFO_CHAR		/* Handle fThreadInfo Query */
	bne		_dbg__cmd_Query_check_sThreadInfo
	cmp		r1, #CMD_QUERY_FTINFO_PARAMLEN
	bne		_dbg__cmd_Query_default
	_dbg_outputMsgFirstThreadInfo
	b	__dbg__sendDebugMsgExit
_dbg__cmd_Query_check_sThreadInfo:
	teq		r2, #CMD_QUERY_STINFO_CHAR		/* Handle sThreadInfo ID Query */
	bne		_dbg__cmd_Query_default
	cmp		r1, #CMD_QUERY_STINFO_PARAMLEN
	bne		_dbg__cmd_Query_default
	_dbg_outputMsgSubsequentThreadInfo
	b	__dbg__sendDebugMsgExit

_dbg__cmd_Query_default:
	b		__dbg__procUnimplementedError	/* FIXME: return an empty message to GDB (no modifiable settings) */


/* _dbg__cmd_GetOneReg
 *		Get One Register Value Command Handler
 *		Valid command parameter x is from '0' to 'F' for User Mode Registers R0-R15,
 *			'18' for FPSCR (dummy), and '19' for CPSR
 *		On entry:
 *			r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              x
 *      On exit:
 *          r0, r1, r2, r3, r4: destroyed
 *
 */
_dbg__cmd_GetOneReg:
	stmfd	sp!, {lr}
	bl      __dbg__cmdParamLen
	cmp		r1, #CMD_REG_GETONE_MINPARAMLEN	/* Check for correct length */
	blo		__dbg__procCmdParamError		/* Unexpected input, report error */
 	cmp		r1, #CMD_REG_GETONE_MAXPARAMLEN	/* Check for correct length */
	bhi		__dbg__procCmdParamError		/* Unexpected input, report error */
    bl      ascii2hex_varlen_be             /* convert ASCII reg enum to Hex (in R0), R1 has address of next buffer char */

_dbg__proc_getRegister:
	mov		r4, r0							/* Keep register index safe */
	_dbg_outputMsgValidResponse				/* R0: address of output message buffer data pointer (after response prefix) */
	mov		r1, r4							/* Move register index value to R1 */
	bl		_dbg_outputOneRegValue			/* update output buffer */
	_asciiz r0, r1
	bl		dbg__putDebugMsg				/* Send response to the GDB server */
	ldmfd	sp!, {pc}

/* _dbg_outputOneRegValue
 *		Given Register Enum (0-F: R0-R15, 0x19: CPSR, OtherVal: dummy), output hex char to buffer
 *		On entry:
 *          r0: output message buffer pointer
 *			r1: register enum (0-F, 0x19)
 *		On exit:
 *			r0: updated (points to next character slot at end of Output Buffer)
 *			r1: original output message buffer pointer
 *			r2: destroyed
 */
_dbg_outputOneRegValue:
	stmfd	sp!, {lr}
	cmp		r1, #REG_PC
	addls	r2, r1, #DBGSTACK_USERREG_INDEX	/* Convert register enum to Debug Stack index */
	bls		_retrieve_RegVal

	cmp		r1, #REG_CPSR
	moveq	r2, #DBGSTACK_USERCPSR_INDEX	/* convert register enum to Debug Stack index */
	beq		_retrieve_RegVal

	bhi		_exit_dbg_outputOneRegValue		/* No match (reg enum > REG_CPSR), skip */

#if 0
	cmp		r1, #REG_FPSCR
	bne		_exit_dbg_outputOneRegValue		/* No match, skip */
#endif

_output_dummy_regval:
	/* Output Dummy Register value (for F0-F7, FPSR) */
	/* FIXME: F0-F7 expects output value that is more than 32-bits */
	mov		r1, #0
	b		_output2buffer					/* Output all zeros for F0-F7, FPSCR */

_retrieve_RegVal:
	_getdbgregisterfromindex r2, r1			/* Retrieve Register contents into R1 */

_output2buffer:

#ifdef __BIG_ENDIAN__
	bl		word2ascii_be				    /* Convert and put hex chars into Output Message Buffer */
#else
    bl      word2ascii_le                   /* Convert and put hex chars into Output Message Buffer */
#endif

_exit_dbg_outputOneRegValue:
	ldmfd	sp!, {pc}

/* _dbg__cmd_GetAllRegs
 *		Get All Register Values Command Handler
 *		Output Buffer returns register values in the order: User R0, R1, R2, ..., R15
 *		On entry:
 *			r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              <NULL> (no parameters)
 *      On exit:
 *          r0, r1, r2, r3: destroyed
 */
_dbg__cmd_GetAllRegs:
	stmfd	sp!, {lr}
    bl      __dbg__cmdParamLen
    teq     r1, #CMD_REG_GETALL_PARAMLEN    /* Check for correct length */
	bne		__dbg__procCmdParamError		/* Unexpected input, report error */

    _dbg_outputMsgValidResponse             /* R0: address of output message buffer data pointer (after response prefix) */

	/* We must return R0-R15, then CPSR */
	mov		r3, #REG_R0						/* Output User Register Values first */
1:  mov		r1, r3
	bl		_dbg_outputOneRegValue			/* update output buffer */
	add		r3, r3, #1						/* increment index */
	cmp		r3, #REG_PC
	bls		1b								/* process all the registers */
#if 0
_get_cpsr:
	/* GDB will query for CPSR value specifically */
	mov		r1, #REG_CPSR					/* Output User CPSR Value last */
	bl		_dbg_outputOneRegValue			/* update output buffer */
#endif
    _asciiz r0, r1
	bl		dbg__putDebugMsg				/* Send response to the GDB server */
	ldmfd	sp!, {pc}

/* _dbg_setOneRegValue
 *		Given Register Enum (0-F: R0-R15, 0x19: CPSR, OtherVal: dummy), output hex char to buffer
 *		On entry:
 *          r0: parameter buffer pointer
 *			r1: register enum (0-F, 0x19)
 *		On exit:
 *          r0: (Updated) Address of parameter in buffer
 *          r1, r2, r3: destroyed
 *
 * FIXME: This routine assumes that the input buffer contains exactly 8-Hex digits for each register value.
 */
_dbg_setOneRegValue:
    stmfd   sp!, {lr}
	cmp		r1, #REG_PC
	addls	r2, r1, #DBGSTACK_USERREG_INDEX	/* Convert register enum to Debug Stack index */
	bls		_store_RegVal

	cmp		r1, #REG_CPSR
	moveq	r2, #DBGSTACK_USERCPSR_INDEX	/* convert register enum to Debug Stack index */
	beq		_store_RegVal

	bhi		_exit_dbg_setOneRegValue		/* No match (reg enum > REG_CPSR), skip */

#if 0
	cmp		r1, #REG_FPSCR
	bne		_exit_dbg_setOneRegValue		/* No match, skip */
#endif

_set_dummy_regval:
	/* FIXME: This assumes that we will never get FP reg values (96-bits) in this routine */
	/* Set dummy FPSCR value (ignored) */
	add		r1, r0, #CMD_REG_REGPARAMLEN	/* Just increment the pointer */
	b		_done_store_RegVal

_store_RegVal:
#ifdef __BIG_ENDIAN__
    bl      ascii2word_be
#else
    bl      ascii2word_le
#endif
    /* R0: value, R1: pointer to next char in buffer */
    _setdbgregisterfromindex r2, r0, r3     /* Set Register contents in R0, using index in R2, and scratch register R3 */
_done_store_RegVal:
	mov		r0, r1							/* Copy buffer pointer to next parameter to R0 for return value */
_exit_dbg_setOneRegValue:
    ldmfd   sp!, {pc}


/* _dbg__cmd_SetOneReg
 *      Set One Register Value Command Handler
 *		Valid command parameter x is from '0' to 'F' for User Mode Registers R0-R15,
 *			'18' for FPSCR (dummy), and '19' for CPSR
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              x=rrrr
 *      On exit:
 *          r0, r1, r2, r3, r4: destroyed
 *
 */
_dbg__cmd_SetOneReg:
    stmfd   sp!, {lr}
    bl      __dbg__cmdParamLen
    cmp     r1, #CMD_REG_SETONE_MINPARAMLEN    /* Check for correct length */
    blo     __dbg__procCmdParamError        /* Unexpected input, report error */
    cmp     r1, #CMD_REG_SETONE_MAXPARAMLEN    /* Check for correct length */
    bhi     __dbg__procCmdParamError        /* Unexpected input, report error */

    /* FIXME: We can't set FP regs.
     * Fortunately the values required by FP regs are 96 bits (24 nibbles)
     * so it fails the CMD_REG_SETONE_MAXPARAMLEN check
     */

    bl      ascii2hex_varlen_be             /* convert ASCII reg enum to Hex (in R0), R1 has address of next buffer char */
    cmp		r0, #REG_FPSCR
    beq     __dbg__procCmdParamError        /* Don't allow setting of FPSCR */
	mov		r4, r0							/* Keep enum in R4 */
    _check_msgassignment r1					/* R1 points to next char in buffer */
    bne     __dbg__procCmdParamError        /* Can't find '=' */

_dbg__proc_setRegister:
	mov		r0, r1							/* move parameter buffer pointer to R0 */
    mov     r1, r4                          /* and retrieve enum to R1 to call _dbg_setOneRegValue */
    bl      _dbg_setOneRegValue
    b       __dbg__procCmdOk

/* _dbg__cmd_SetAllReg
 *      Set All Register Values Command Handler
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              rrrrRRRRrrrr... (17 registers)
 *      On exit:
 *          r0, r1, r2, r3, r4: destroyed
 *
 */
_dbg__cmd_SetAllRegs:
	/* Assumes that the registers are in the sequence R0, R1, ... R15 */
    stmfd   sp!, {lr}
    bl      __dbg__cmdParamLen              /* R0: pointer to parameters in buffer */
    teq     r1, #CMD_REG_SETALL_PARAMLEN    /* Check for correct length */
    bne     __dbg__procCmdParamError        /* Unexpected input, report error */
    mov     r4, #REG_R0					    /* R4: register enum, starting with enum for R0 */
1:	mov		r1, r4							/* Copy enum to R1; R0 already points to current parameter */
	bl		_dbg_setOneRegValue				/* R0: next parameter address pointer */
    add     r4, r4, #1                      /* increment index */
    cmp		r4, #REG_PC
    bls		1b

    ldrb    r0, [r0]
    teq     r0, #0                          /* Look for ASCIIZ character to terminate loop */
    beq     __dbg__procCmdOk
	bne		__dbg__procCmdParamError		/* Unexpected input, report error */

/* _dbg__nop
 *		NOP Command Handler (placeholder)
 *		On entry:
 *			r0: parameter buffer (contents after '$' and '<cmdchar>')
 *      On exit:
 *          r0, r1, r2, r3: destroyed
 */
_dbg__nop:
	stmfd	sp!, {lr}
	b      __dbg__procUnimplementedError

/* _dbg__cmd_ReadMem
 *      Read Memory Contents Command Handler
 *      Output Buffer returns memory contents
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              AA..AA,LLLL
 *      On exit:
 *          r0, r1, r2, r3, r4, r5: destroyed
 */
_dbg__cmd_ReadMem:
    stmfd   sp!, {lr}
    bl      __dbg__cmdParamLen
#if 0
    teq     r1, #CMD_MEM_READ_PARAMLEN      /* Check for correct length */
    bne     __dbg__procCmdParamError        /* Unexpected input, report error */
    bl      ascii2word_be                   /* convert ASCII address location to Hex (in R0), R1 has address of next buffer char */
#endif
    bl      ascii2hex_varlen_be             /* convert ASCII address to Hex (in R0), R1 has address of next buffer char */
    mov     r5, r0                          /* Keep Address location in R5 */
    _check_msgseparator r1
    bne     __dbg__procCmdParamError        /* Can't find ',' */
    mov     r0, r1                          /* move buffer pointer to R0 for subsequent processing */
    bl      ascii2hex_varlen_be             /* convert ASCII length to Hex (in R0), R1 has address of next buffer char */
    cmp     r0, #CMD_MEM_MAXREADBYTES       /* Don't overflow our buffer (2 x CMD_MEM_MAXREADBYTES) */
    bhi     __dbg__procCmdReturnOutputLengthError /* Requested Length greater than buffer size, return error */
    mov     r4, r0                          /* Keep numbytes in R4 */
    /* FIXME: Should validate the address? */

    _dbg_outputMsgValidResponse             /* R0: address of output message buffer data pointer (after response prefix) */
1:  ldrb    r1, [r5], #1
    bl      byte2ascii                       /* update output buffer */
    subs    r4, r4, #1
    bne     1b

    _asciiz r0, r1
    bl      dbg__putDebugMsg                /* Send response to the GDB server */
    ldmfd   sp!, {pc}


/* _dbg__cmd_WriteMem
 *      Write Memory Contents Command Handler
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              AA..AA,LLLL::bb..bb
 *      On exit:
 *          r0, r1, r2, r3, r4: destroyed
 */
_dbg__cmd_WriteMem:
    stmfd   sp!, {lr}
    bl      __dbg__cmdParamLen
#if 0
    cmp     r1, #CMD_MEM_WRITE_MINPARAMLEN  /* Check for correct (minimum) length */
    blo     __dbg__procCmdParamError        /* Unexpected input, report error */
    sub     r4, r1, #CMD_MEM_WRITE_MINPARAMLEN  /* R4: Number of ASCII Hex chars for byte writes */
    bl      ascii2word_be                   /* convert ASCII address location to Hex (in R0), R1 has address of next buffer char */
#endif
    bl      ascii2hex_varlen_be             /* convert ASCII address to Hex (in R0), R1 has address of next buffer char */
    mov     r3, r0                          /* Keep Address location in R3 */
    _check_msgseparator r1
    bne     __dbg__procCmdParamError        /* Can't find ',' */
    mov     r0, r1                          /* move buffer pointer to R0 for subsequent processing */
    bl      ascii2hex_varlen_be             /* convert ASCII length to Hex (in R0), R1 has address of next buffer char */
    cmp     r0, r4, asr #1                  /* is Number of bytes to write == (number of ASCII Hex Chars / 2)? */
    bne     __dbg__procCmdParamError        /* Number of bytes does not match argument length */
    cmp     r0, #CMD_MEM_MAXWRITEBYTES      /* Don't overflow our buffer (2 x CMD_MEM_MAXWRITEBYTES) */
    bhi     __dbg__procCmdReturnInputLengthError /* Requested Length greater than buffer size, return error */
    mov     r4, r0                          /* Keep numbytes in R4 */
    /* FIXME: Should validate the address? */
    _check_msgargument r1
    bne     __dbg__procCmdParamError        /* Can't find ':' */

1:  mov     r0, r1
    bl      ascii2byte                      /* convert ASCII Hex value into byte value */
    strb    r0, [r3], #1                    /* Store into memory location */
    subs    r4, r4, #1
    bne     1b
    b       __dbg__procCmdOk

/* _dbg__cmd_Detach
 *      Detach User Program Execution Command Handler
 *      Treat this as being equivalent to 'Continue' without any arguments
 *
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *				<NULL> (No Parameters)
 *      On exit:
 *          r0-r7: destroyed
 *			Note: This routine does not return to caller. Instead it switches
 *				  operating mode to UNDEF and returns to previously active program
 */
_dbg__cmd_Detach:
	stmfd	sp!, {lr}						/* In case unexpected parameters were received */
    bl      __dbg__cmdParamLen
    teq     r1, #CMD_DETACH_PARAMLEN    	/* Check for correct length */
	bne		__dbg__procCmdParamError		/* Unexpected input, report error */

	ldmfd	sp!, {lr}						/* Cleanup stack, since we won't return to the Debugger Run Loop */
	_dbg_setstate	DBG_INIT				/* Reset the Debug State */
    bl		__dbg__procOkOnly				/* send OK to keep GDB server happy */
	b       _dbg__cont_check_breakpoint_type /* Continue from current PC */

/* _dbg__cmd_Continue
 *      Continue User Program Execution Command Handler
 *      Setup breakpoints before resuming execution of program.
 *
 *      If Address is specified, update the next instruction address to specified address
 *
 *      If this is a Normal Breakpoint, then we resume from current (Breakpoint) exception address
 *      Else (it is a Manual Breakpoint or Address Specified)
 *          We need to resume from the next instruction address
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              Optional: AA..AA
 *      On exit:
 *          r0-r7: destroyed
 *			Note: This routine does not return to caller. Instead it switches
 *				  operating mode to UNDEF and returns to previously active program
 */
_dbg__cmd_Continue:
    /* Don't put anything on the stack, we won't return to the Debugger Run Loop */
    bl      __dbg__cmdParamLen
    cmp     r1, #CMD_CONTINUE_MINPARAMLEN	/* Check for correct parameter length */
    bne	    _dbg__cont_fromAddr				/* Address is specified */
   	bl      __dbg__procAckOnly				/* send Ack to keep GDB server happy */
    b       _dbg__cont_check_breakpoint_type          /* Continue from current PC */

_dbg__cont_fromAddr:
    bl      ascii2hex_varlen_be             /* convert ASCII address to Hex (in R0), R1 has address of next buffer char */
    /* Continue from Specified Address */
    _setdbgregister DBGSTACK_NEXTINSTR_INDEX, r0, r1	/* Set Next Instruction Pointer for Resume using contents of R0, and scratch register R1 */
   	bl		__dbg__procAckOnly				/* send Ack to keep GDB server happy */
    b       _dbg__cont_is_manual_bkpt_or_address_specified

_dbg__cont_check_breakpoint_type:
    _dbg_get_bkpt_type   r0
    teq r0, #DBG_MANUAL_BKPT_ARM
    beq _dbg__cont_is_manual_bkpt_or_address_specified
    teq r0, #DBG_MANUAL_BKPT_THUMB
    beq _dbg__cont_is_manual_bkpt_or_address_specified

_dbg__cont_is_normal_breakpoint:
    _getdbgregister DBGSTACK_USERPC_INDEX, r0	/* Retrieve Aborted Instruction PC from the Debug Stack into R0 */
    _setdbgregister DBGSTACK_NEXTINSTR_INDEX, r0, r1	/* Set Next Instruction Pointer for Resume using contents of R0, and scratch register R1 */
	/* GDB knows to Step from a breakpointed instruction before continuing when
	 * Continue is issued, so it is safe to activate all defined breakpoints and continue.
	 */
_dbg__cont_is_manual_bkpt_or_address_specified:
    bl      _dbg__activate_breakpoints      /* Restore exisiting breakpoints */
    b       _dbg__switch2undefmode

/* _dbg__cmd_Step
 *      Step User Program Execution Command Handler
 *      Setup breakpoints before resuming execution of program.
 *
 *      If Address is specified, update the next instruction address to specified address
 *
 *      If this is a Normal Breakpoint, then we need to install a Step Breakpoint at next instruction address
 *          and resume from current (Breakpoint) exception address
 *      Else (it is a Manual Breakpoint or Address specified)
 *          We need to install a Step Breakpoint at the following instruction address (after the next instruction address)
 *          and resume from the next instruction address
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              Optional: AA..AA
 *      On exit:
 *          r0-r7: destroyed
 *          Note: This routine does not return to caller. Instead it switches
 *                operating mode to UNDEF and returns to previously active program
 */
_dbg__cmd_Step:
    /* Don't put anything on the stack, we won't return to the Debugger Run Loop */
    bl      __dbg__cmdParamLen
    cmp     r1, #CMD_STEP_MINPARAMLEN       /* Check for correct parameter length */
    beq     _dbg__step_check_breakpoint_type          /* Step from current PC */

_dbg__step_fromAddr:
    bl      ascii2hex_varlen_be             /* convert ASCII address to Hex (in R0), R1 has address of next buffer char */
    /* Step from Specified Address */
    _setdbgregister DBGSTACK_NEXTINSTR_INDEX, r0, r1	/* Set Next Instruction Pointer for Resume using contents of R0, and scratch register R1 */
    b       _dbg__step_is_manual_bkpt_or_address_specified

_dbg__step_check_breakpoint_type:
    _dbg_get_bkpt_type   r0
    teq 	r0, #DBG_MANUAL_BKPT_ARM
    beq 	_dbg__step_is_manual_bkpt
    teq 	r0, #DBG_MANUAL_BKPT_THUMB
    beq 	_dbg__step_is_manual_bkpt

_dbg__step_is_normal_breakpoint:
    _getdbgregister DBGSTACK_USERPC_INDEX, r0			/* Retrieve Aborted Instruction PC from the Debug Stack into R0 */
    _setdbgregister DBGSTACK_NEXTINSTR_INDEX, r0, r1	/* Set Next Instruction Pointer for Resume usinh contents in R0, and scratch register R1 */
    b       _dbg__step_is_manual_bkpt_or_address_specified

_dbg__step_is_manual_bkpt:
    _getdbgregister DBGSTACK_NEXTINSTR_INDEX, r0         /* Retrieve Next Instruction Pointer for Resume into R1 */
    /* b       _dbg__step_is_manual_bkpt_or_address_specified */

_dbg__step_is_manual_bkpt_or_address_specified:
    bl  	dbg_following_instruction_addr 		/* following instruction address returned in r1 */
    bl  	dbg__install_singlestep        		/* Setup Single Step, next instruction address returned in r1 */
	bl		dbg__activate_singlestep
#if 0
	/* Disable ACK transmit, let the next Breakpoint generate the Signal Response */
	bl		__dbg__procAckOnly					/* send Ack to keep GDB server happy */
#endif
    b       _dbg__switch2undefmode

/* _dbg__cmd_Kill
 *      Kill User Program Execution Command Handler
 *      Kill Program, this is accomplished by rebooting the Brick
 *
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *				<NULL> (No Parameters)
 *      On exit:
 *          r0-r7: destroyed
 *          Note: This routine does not return to caller. Instead it calls
 *				the relevant system routine to reboot the Brick
 */
_dbg__cmd_Kill:
	stmfd	sp!, {lr}						/* In case unexpected parameters were received */
    bl      __dbg__cmdParamLen
    teq     r1, #CMD_KILL_PARAMLEN    		/* Check for correct length */
	bne		__dbg__procCmdParamError		/* Unexpected input, report error */

	bl		__dbg__procAckOnly				/* send Ack to keep GDB server happy */
	b		dbg__reboot						/* Goodbye.... */

/* _dbg__proc_brkpt_params
 *      Process Breakpoint Parameters
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              t,AA..AA,k
 *      On exit:
 *          r0: non-zero = breakpoint address; 0 = parameter error
 *          r1: destroyed
 *          r2: destroyed
 *          r3: destroyed
 */
_dbg__proc_brkpt_params:
    /* FIXME: Add support for watchpoints */
    stmfd   sp!, {lr}
    mov     r3, r0                          /* Keep parameter buffer address in R3 */
    ldrb    r0, [r3], #1                    /* get breakpoint type t */
    bl      char2hex
    cmp     r0, #CMD_BKPT_TYPE_BREAK_MEMORY
    bne     _dbg__proc_brkpt_params_error   /* We only support memory breakpoints for now */
    _check_msgseparator r3
    bne     _dbg__proc_brkpt_params_error   /* Something wrong with the parameters */
    mov     r0, r3                          /* Check Address */
    bl      ascii2hex_varlen_be             /* convert ASCII address to Hex (in R0), R1 has address of next buffer char */
    mov     r3, r0                          /* Keep breakpoint address in R3 */
    _check_msgseparator r1
    bne     _dbg__proc_brkpt_params_error   /* Something wrong with the parameters */
    ldrb    r0, [r1], #1                    /* get breakpoint kind k */
    bl      char2hex
    cmp     r0, #CMD_BKPT_KIND_THUMB
    orreq   r3, r3, #1                      /* Mark Thumb breakpoints */
    beq     _exit_dbg__proc_brkpt_params
    cmp     r0, #CMD_BKPT_KIND_ARM
    beq     _exit_dbg__proc_brkpt_params    /* ARM breakpoint */

_dbg__proc_brkpt_params_error:
    mov     r3, #0                          /* Unrecognized breakpoint type */
_exit_dbg__proc_brkpt_params:
    mov     r0, r3                          /* return breakpoint address */
    ldmfd   sp!, {pc}

/* _dbg__cmd_InsertBreakpoint
 *		Add Breakpoint
 *      On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              t,AA..AA,k
 *      On exit:
 *          r0, r1, r2, r3: destroyed
 */
_dbg__cmd_InsertBreakpoint:
    stmfd   sp!, {lr}
    bl      __dbg__cmdParamLen
    teq     r1, #CMD_BKPT_INSERT_MINPARAMLEN   /* Check for correct length */
    blo     __dbg__procCmdParamError        /* Unexpected input, report error */
    bl      _dbg__proc_brkpt_params         /* R0: Breakpoint Address */
    teq     r0, #0
    beq     __dbg__procBreakpointAddrError  /* Thumb2 instructions, or unknown kind */
    mov     r3, r0                          /* Keep breakpoint address in R3 */
    mov     r0, #0                          /* Empty Breakpoint entry */
    bl      _dbg_find_breakpoint_slot       /* Look for an available breakpoint slot, return index in R0 */
    cmp     r0, #CMD_BKPT_NOTFOUND
    beq     __dbg__procBreakpointAddrError  /* No empty slot! */
    mov     r1, r3                          /* Move breakpoint address to R1 */
	bl     _dbg__install_one_breakpoint		/*	r0: index, r1: instruction address */
    b       __dbg__procCmdOk

/* _dbg__cmd_RemoveBreakpoint
 *		Remove Breakpoint
 *		On entry:
 *          r0: parameter buffer pointer (contents after '$' and '<cmdchar>')
 *              t,AA..AA,k
 *      On exit:
 *          r0, r1, r2, r3: destroyed
 */
_dbg__cmd_RemoveBreakpoint:
    stmfd   sp!, {lr}
    bl      __dbg__cmdParamLen
    teq     r1, #CMD_BKPT_REMOVE_MINPARAMLEN  /* Check for correct length */
    blo     __dbg__procCmdParamError       /* Unexpected input, report error */
    bl      _dbg__proc_brkpt_params        /* R0: Breakpoint Address */
    teq     r0, #0
    beq     __dbg__procBreakpointAddrError /* Thumb2 instructions, or unknown kind */
    bl      _dbg_find_breakpoint_slot      /* Look for matching breakpoint slot, return index in R0 */
    cmp     r0, #CMD_BKPT_NOTFOUND
    beq     __dbg__procBreakpointAddrError /* Specified Breakpoint not found! */
	_index2bkptindex_addr	r0, r1		   /* Calculate Breakpoint Entry Address */
	mov     r0, r1                         /* Move it to R0 for subroutine call */
	bl	    _dbg__clear_one_breakpoint     /* R0: address of breakpoint to clear */
    b       __dbg__procCmdOk

/****************************************************************************
 *
 * Breakpoint Manipulation Routines
 *
 ****************************************************************************/
/* _dbg_find_breakpoint_slot
 *      Find the matching Breakpoint Slot.
 *      This is both used to find empty slots (pass R0=0x0000) or
 *      occupied slots (pass R0=<brkpt addr>)
 *
 *  On Entry:
 *      R0: Breakpoint Address
 *  On Exit:
 *      R0: Matching Index (-1: not found)
 *
 *  NOTE: This routine performs exact match, i.e., breakpoint address MUST be configured
 *        for ARM or Thumb (bit 0 clear/set) as appropriate
 */

_dbg_find_breakpoint_slot:
    stmfd   sp!, {r1,r2,r3, lr}
    mov     r1, #1                      /* Only consider Breakpoints 1-7 */
    ldr     r3, =__breakpoints_num__
1:
    _index2bkptindex_addr   r1, r2      /* Calculate Breakpoint Entry Address */
    ldr     r2, [r2]                    /* Get actual breakpoint entry (instruction address) */
    cmp     r0, r2
    beq     _found_breakpoint_slot
    add     r1, r1, #1                  /* no match, check next */
    cmp     r1, r3
    blo     1b                          /* continue checking only if we don't exceed __breakpoints_num__ */

_notfound_breakpoint_slot:
    mov     r1, #CMD_BKPT_NOTFOUND
_found_breakpoint_slot:
    mov     r0, r1                      /* Return value in R0 */
    ldmfd   sp!, {r1,r2,r3, pc}

/* _dbg__clear_singlestep
 *		Clear the Single Step Breakpoint
 */
_dbg__clear_singlestep:
	ldr		r0, =__breakpoints_start__	/* Single Step Breakpoint is at the beginning of the Breakpoint State Struct */
/*	b		_dbg__clear_one_breakpoint */

/* _dbg__clear_one_breakpoint
 *		On entry, R0 contains the Breakpoint State slot address to be cleared
 *
 */
_dbg__clear_one_breakpoint:
	mov		r1, #0
	mov		r2, #0
	stmea	r0!, {r1, r2}		/* clear Breakpoint state */
	bx		lr

/* _dbg__clear_breakpoints
 *		Routine iterates through the array of breakpoints (incl single step breakpoint) and clears the breakpoint
 */
_dbg__clear_breakpoints:
	stmfd	sp!, {lr}
	ldr		r0, =__breakpoints_start__	/* Single Step Breakpoint is at the beginning of the Breakpoint State Struct */
	ldr		r3, =__breakpoints_end__		/* start from top of the table */
3:	bl		_dbg__clear_one_breakpoint
	cmp		r0, r3
	blo		3b
	ldmfd	sp!, {pc}

	.global dbg__install_singlestep
/* dbg__install_singlestep
 *		Install the Single Step Breakpoint
 *
 *		On entry:
 *			R1: Instruction Address (31 bits, b0 = THUMB flag)
 *		On exit:
 *			R0: Breakpoint index (preserved)
 *			R1: Instruction Address (preserved)
 *			R2: Breakpoint Instruction
 *			R3: Breakpoint Entry address
 */
dbg__install_singlestep:
	mov		r0, #0
/*	b		_dbg__install_one_breakpoint */

/* _dbg__install_one_breakpoint
 *		Install breakpoint entry into Breakpoint State Table
 *
 *		On entry:
 *			R0: Breakpoint index (assumed valid)
 *			R1: Instruction Address (31 bits, b0 = THUMB flag)
 *		On exit:
 *			R0: Breakpoint index (preserved)
 *			R1: Instruction Address (preserved)
 *			R2: Breakpoint Instruction
 *			R3: Breakpoint Entry address
 */
_dbg__install_one_breakpoint:
/* Check for Thumb bit */
	tst		r1, #BKPT_STATE_THUMB_FLAG	/* 1: Thumb instruction */
/* Assume that the address entry is valid, otherwise we should sanitize it (mask out b1) */
	bic		r2, r1, #BKPT_STATE_THUMB_FLAG	/* R2: Instruction Address; clear Thumb Flag for accessing Memory */
	ldreq	r2, [r2]					/* if 0: load ARM instruction from address location */
	ldrneh	r2, [r2]					/* else load Thumb instruction */
	_index2bkptindex_addr	r0, r3		/* Calculate Breakpoint Entry Address */
	stm		r3, {r1, r2}
	bx		lr


/* _dbg__restore_singlestep
 *		Restores the contents of the single step breakpoint to memory
 *
 *		On entry:
 *			None
 *		On exit:
 *			R0-R2: Destroyed
 */
_dbg__restore_singlestep:
	mov		r0, #0						/* single step breakpoint index */
	_index2bkptindex_addr	r0, r1		/* Calculate Single Step Breakpoint Entry Address */
	ldm		r1, {r1, r2}				/* r1: Breakpoint Address, r2: Breakpoint Instruction */
	teq		r1, #0
	bxeq	lr							/* Exit if not active */
/*	b		_dbg__restore_one_breakpoint */

/* _dbg__restore_one_breakpoint
 *		Restores the contents to memory for one breakpoint
 *
 *		On entry:
 *			R0: Breakpoint index (assumed valid) [not used -- can be used for validating BKPT]
 *			R1: Breakpoint Address (assumed valid)
 *			R2: Breakpoint Instruction (assumed valid)
 *		On exit:
 *			R0: Breakpoint index (preserved)
 *			R1: B0 cleared if Thumb Instruction Address
 *			R2: Breakpoint Instruction (preserved)
 */
_dbg__restore_one_breakpoint:
/* Check for Thumb bit */
	tst		r1, #BKPT_STATE_THUMB_FLAG	/* 1: Thumb instruction */
/* Assume that the address entry is valid, otherwise we should sanitize it (mask out b1) */
	streq	r2, [r1]					/* if 0: restore ARM instruction to address location */
	bicne	r1, #BKPT_STATE_THUMB_FLAG	/* else, clear Thumb Flag */
	strneh	r2, [r1]					/* store Thumb instruction */
	bx		lr

/* _dbg__restore_breakpoints
 *		Routine iterates through the array of breakpoints (incl single step breakpoint) and restores the contents to memory
 *		Only Active breakpoints (i.e., Non-zero Address) are processed.
 *
 *		On entry:
 *			None
 *		On exit:
 *			R0-R6: Destroyed
 */
_dbg__restore_breakpoints:
	stmfd	sp!, {lr}
	ldr		r6, =_dbg__restore_one_breakpoint
	b		__dbg__iterate_breakpoint_array

	.global dbg__activate_singlestep
/* dbg__activate_singlestep
 *		Activate the single step breakpoint to memory
 *
 *		On entry:
 *			None
 *		On exit:
 *			R0-R3: Destroyed
 */
dbg__activate_singlestep:
	mov		r0, #0						/* single step breakpoint index */
	_index2bkptindex_addr	r0, r1		/* Calculate Single Step Breakpoint Entry Address */
	ldm		r1, {r1, r2}				/* r1: Breakpoint Address, r2: Breakpoint Instruction */
	teq		r1, #0
	bxeq	lr							/* Exit if not active */
/*	b		_dbg__activate_one_breakpoint */

/* _dbg__activate_one_breakpoint
 *		Activate one breakpoint to memory
 *
 *		On entry:
 *			R0: Breakpoint index (assumed valid)
 *			R1: Breakpoint Address (assumed valid)
 *			R2: Breakpoint Instruction (assumed valid)
 *		On exit:
 *			R0: Breakpoint index (preserved)
 *			R1-R3: Destroyed
 */
_dbg__activate_one_breakpoint:
/* Check for Thumb bit */
	tst		r1, #BKPT_STATE_THUMB_FLAG	/* 1: Thumb instruction */
	bne		_nx_is_thumb_bp
_nx_is_arm_bp:
/* Assume that the address entry is valid, otherwise we should sanitize it (mask out b1) */
	ldr		r3, [r1]					/* if 0: load ARM instruction from address location */
	teq		r2, r3							/* check that the two instructions are identical */
	bne		_dbg__breakpoint_invalid_arm
	ldr		r2, =BKPT32_INSTR				/* ARM BKPT instruction */
	orr		r2, r2, r0						/* Merge Breakpoint index */
	str		r2, [r1]						/* Store it into memory location */
_dbg__breakpoint_invalid_arm:
	bx		lr
_nx_is_thumb_bp:
	bic		r1, #BKPT_STATE_THUMB_FLAG	/* else, clear Thumb Flag */
	ldrh	r3, [r1]						/* load Thumb instruction from address location */
	teq		r2, r3							/* check that the two instructions are identical */
	bne		_dbg__breakpoint_invalid_thumb
	ldr		r2, =BKPT16_INSTR				/* Thumb BKPT instruction */
	orr		r2, r2, r0						/* Merge Breakpoint index */
	strh	r2, [r1]						/* Store it into memory location */
_dbg__breakpoint_invalid_thumb:
	bx		lr

/* _dbg__activate_breakpoints
 *		Routine iterates through the array of breakpoints (incl single step breakpoint) and activates them
 *		Only Active breakpoints (i.e., Non-zero Address) are processed.
 *
 *		On entry:
 *			None
 *		On exit:
 *			R0-R6: Destroyed
 */
_dbg__activate_breakpoints:
	stmfd	sp!, {lr}
	ldr		r6, =_dbg__activate_one_breakpoint
	b		__dbg__iterate_breakpoint_array


/* __dbg__iterate_breakpoint_array
 *	WARNING: DO NOT CALL THIS ROUTINE DIRECTLY.
 *		Internal Routine, assumes that lr has been push to stack
 *
 *		Common routine to iterate through the array of breakpoints (incl single step breakpoint)
 *		Only Active breakpoints (i.e., Non-zero Address entries) are processed.
 *
 *		On Entry:
 *			R0: Breakpoint index
 *			R1: Breakpoint Address
 *			R2: Breakpoint Instruction
 *			R6: Handler Routine
 *		On exit:
 *			R0-R5: Destroyed
 *			R6: Handler routine (preserved)
 *
 */
__dbg__iterate_breakpoint_array:
	ldr		r5, =__breakpoints_end__		/* start from top of the table (Assume __breakpoints_end__ > __breakpoints_start__) */
	ldr		r4, =__breakpoints_start__		/* end address check */
	ldr		r0, =__breakpoints_num__		/* Number of Breakpoints (incl Single Step) (Assume Non-Zero) */
4:	sub		r0, r0, #1						/* Decrement breakpoint index in r0 */
	ldmea	r5!, {r1, r2}					/* r1: Breakpoint Address, r2: Breakpoint Instruction */
	teq		r1, #0							/* Is it active? */
	movne	lr, pc
	bxne	r6								/* active entry */
	cmp		r5, r4
	bhi		4b								/* if (pointer > start of Breakpoint Table address), get next slot */
	ldmfd	sp!, {pc}

