/** @file debug_macros.h
 *  @brief internal macros used by debug_stub routines
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

#ifndef __DEBUG_MACROS_H__
#define __DEBUG_MACROS_H__



/** @addtogroup debug_macros */
/*@{*/

/* _dbg_jumpTableHandler
 *      Call Jump Table Routine based on Index
 *	On entry:
 *	  jumptableaddr is the address (constant) of the Jump Table
 *	  jumpreg is the register used to perform the indirect jump
 *	  indexreg contains jump table index value
 */
	.macro  _dbg_jumpTableHandler   jumptableaddr, jumpreg, indexreg

	ldr	 \jumpreg, =\jumptableaddr
	ldr	 \jumpreg, [\jumpreg, \indexreg, lsl #2]
	mov	 lr, pc
	bx	  \jumpreg	/* Call Command Handler Routine */
	.endm


/* _dbg_thumbDecodeEntry
 *      Load Thumb Instruction Decoder Entry
 *      On entry:
 *        instrreg is the register to load the instruction into
 *        instrmask is the register to load the instruction mask into
 *        codehandler is the register to load the code handling routine into
 *        indexreg contains decode table index value
 *      NOTE: instrreg, instrmask, codehandler must be in increasing register number order
 */
        .macro  _dbg_thumbDecodeEntry   instrreg, instrmask, codehandler, indexreg

        ldr      \instrmask, =debug_thumbDecodeTable                    /* Temporary register */
        add      \instrmask, \instrmask, \indexreg, lsl #3
        ldm      \instrmask, {\instrreg, \codehandler}                  /* LSHW: IID, MSHW: IBM */
        mov      \instrmask, \instrreg, lsr #16
        mov      \instrreg, \instrreg, lsl #16
        mov      \instrreg, \instrreg, lsr #16                          /* Keep HLFWORD0 containing instruction */
        .endm

/* _dbg_armDecodeEntry
 *      Load ARM Instruction Decoder Entry
 *      On entry:
 *        instrreg is the register to load the instruction into
 *        instrmask is the register to load the instruction mask into
 *        codehandler is the register to load the code handling routine into
 *        indexreg contains decode table index value
 *      NOTE: instrreg, instrmask, codehandler must be in increasing register number order
 */
        .macro  _dbg_armDecodeEntry   instrreg, instrmask, codehandler, indexreg

        ldr      \instrmask, =debug_armDecodeTable                    /* Temporary register */
        add      \instrmask, \instrmask, \indexreg, lsl #3
        add      \instrmask, \instrmask, \indexreg, lsl #2              /* 12  byte entries */
        ldm      \instrmask, {\instrreg, \instrmask, \codehandler}
        .endm

/* _asciiz
 *	Terminate string given string buffer pointer in \strptr
 *	scratchreg is used as a scratch register (destroyed)
 *
 */
	.macro	_asciiz	strptr, scratchreg
	mov	\scratchreg, #0				/* ASCIIZ character */
	strb	\scratchreg, [\strptr]			/* Terminate ASCII string */
 	.endm


/* _dbg_stpcpy
 *	_dbg_stpcpy macro
 *	On entry:
 *	  deststrptr: Destination string
 *	  sourcestrptr: Source string
 *	  scratchreg: scratch register for copying
 *	On exit:
 *	  deststrptr: Pointer to ASCIIZ character in destination string
 *	  sourcestrptr: Pointer to next character slot in source string (after ASCIIZ)
 *	  scratchreg: destroyed
 */
	.macro  _dbg_stpcpy	 deststrptr, sourcestrptr, scratchreg
1:	ldrb	\scratchreg, [\sourcestrptr], #1
	strb	\scratchreg, [\deststrptr], #1
	teq	 \scratchreg, #0
	bne	 1b
	sub	 \deststrptr, \deststrptr, #1	/* Adjust Destination string pointer to point at ASCIIZ character */
	.endm

/* _dbg_memcpy
 *      _dbg_stpcpy macro
 *      On entry:
 *        deststrptr: Destination string
 *        sourcestrptr: Source string
 *        sizereg: Number of bytes to copy
 *        scratchreg: scratch register for copying
 *      On exit:
 *        deststrptr: Pointer to next character slot in destination string
 *        sourcestrptr: Pointer to next character slot in source string
 *        sizereg, scratchreg: destroyed
 */
        .macro  _dbg_memcpy      deststrptr, sourcestrptr, sizereg, scratchreg
1:      ldrb    \scratchreg, [\sourcestrptr], #1
        strb    \scratchreg, [\deststrptr], #1
        subs    \sizereg, \sizereg, #1
        bne      1b
        .endm

/* _dbg_CopyMsg2OutputBuf
 *      Copies source message to output buffer
 *      On entry:
 *        R2: source message buffer (ASCIIZ terminated)
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R2-R3: destroyed
 */
        .macro  _dbg_CopyMsg2OutputBuf
        ldr      r0, =debug_OutMsgBuf
        _dbg_stpcpy     r0, r2, r3
        .endm

/* _dbg_CopyMsg2OutputBuf_withParam
 *  Internal Routine called to output message with parameters
 *      Return Message with byte-sized parameter
 *      On entry:
 *        R1: byte-sized param
 *        R2: source message buffer (ASCIIZ terminated)
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R1-R3: destroyed
 */
        .macro  _dbg_CopyMsg2OutputBuf_withParam
        _dbg_CopyMsg2OutputBuf    /* R1 unchanged */
        bl      byte2ascii        /* R0 points to buffer position after byte value */
        _asciiz r0, r1
        .endm

/* _dbg_outputAckOnlyFlag
 *      Return Flag ('+') for Continue or Step
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R2-R3: destroyed
 */
        .macro  _dbg_outputAckOnlyFlag
        ldr      r2, =debug_AckOnlyFlag                  /* ASCIIZ terminated */
        _dbg_CopyMsg2OutputBuf
        .endm


/* _dbg_outputRetransmitFlag
 *      Return Flag ('-') for Checksum Error (retransmission needed)
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R2-R3: destroyed
 */
        .macro  _dbg_outputRetransmitFlag
        ldr      r2, =debug_RetransmitFlag                  /* ASCIIZ terminated */
        _dbg_CopyMsg2OutputBuf
        .endm

/* _dbg_outputMsgValidResponse
 *	Return Message with valid response ('+$')
 *	On exit:
 *	  R0: Pointer to Output Buffer next character slot location
 *        R2-R3: destroyed
 */
	.macro  _dbg_outputMsgValidResponse
	ldr	 r2, =debug_ValidResponsePrefix
        _dbg_CopyMsg2OutputBuf
	.endm

/* _dbg_outputMsgStatusOk
 *	Return Message with Ok ('+$OK') status
 *	On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *	  R2-R3: destroyed
 */
	.macro  _dbg_outputMsgStatusOk
	ldr	 r2, =debug_OkResponse			/* ASCIIZ terminated */
        _dbg_CopyMsg2OutputBuf
	.endm

/* _dbg_outputMsgCurrTID
 *      Return Message with Default Thread ID ('+$QC0')
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R2-R3: destroyed
 */
        .macro  _dbg_outputMsgCurrTID
        ldr      r2, =debug_ThreadIDResponse              /* ASCIIZ terminated */
        _dbg_CopyMsg2OutputBuf
        .endm

/* _dbg_outputMsgFirstThreadInfo
 *      Return Message with Default Current Thread ID ('+$m0')
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R2-R3: destroyed
 */
        .macro  _dbg_outputMsgFirstThreadInfo
        ldr      r2, =debug_FirstThreadInfoResponse        /* ASCIIZ terminated */
        _dbg_CopyMsg2OutputBuf
        .endm

/* _dbg_outputMsgSubsequentThreadInfo
 *      Return Message with Default Current Thread ID ('+$m0')
 *      On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R2-R3: destroyed
 */
        .macro  _dbg_outputMsgSubsequentThreadInfo
        ldr      r2, =debug_SubsequentThreadInfoResponse   /* ASCIIZ terminated */
        _dbg_CopyMsg2OutputBuf
        .endm

/* _dbg_outputMsgStatusErr
 *	Return Message with Error ('+$ENN') status
 *	On entry:
 *	  R1: error code
 *	On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R1-R3: destroyed
 */
	.macro  _dbg_outputMsgStatusErr
        ldr     r2, =debug_ErrorResponsePrefix
	_dbg_CopyMsg2OutputBuf_withParam
	.endm

/* _dbg_outputMsgStatusErrCode
 *	Return Message with Error ('+$ENN') status
 *	On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R1-R3: destroyed
 */
	.macro  _dbg_outputMsgStatusErrCode errcode
	mov	r1, #\errcode
	_dbg_outputMsgStatusErr
	.endm

/* _dbg_outputMsgStatusSig
 *	Return Message with Signal ('+$SNN') status
 *	On entry:
 *	  R1: signal code
 *	On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R1-R3: destroyed
 */
	.macro  _dbg_outputMsgStatusSig
        ldr     r2, =debug_SignalResponsePrefix
        _dbg_CopyMsg2OutputBuf_withParam
	.endm

/* _dbg_outputMsgStatusSigCode
 *	Return Message with Signal ('+$SNN') status
 *	On exit:
 *        R0: Pointer to Output Buffer ASCIIZ location
 *        R1-R3: destroyed
 */
	.macro  _dbg_outputMsgStatusSigCode statuscode
	mov	r1, #\statuscode
	_dbg_outputMsgStatusSig
	.endm


/* _regenum2index
 *      Convert register enum to debugger stack index
 *
 *      On entry:
 *        indexenum: enum representing Register to access
 *        indexreg: register to be used to store the debugger stack index value (0-max index)
 *      On exit:
 *        indexreg contains debugger stack index value (0-max index)
 */
	.macro	_regenum2index indexenum, indexreg
    add     \indexreg, \indexenum, #DBGSTACK_USERREG_INDEX /* Convert register index to Debug Stack index */
    .endm

/* _getdbgregisterfromindex
 *      Retrieve register contents from debugger stack given index
 *
 *      On entry:
 *        indexreg contains debugger stack index value (0-max index)
 *      On exit:
 *        indexreg: Breakpoint index (preserved)
 *        contentsreg: Register Contents for given index
 */
        .macro  _getdbgregisterfromindex  indexreg, contentsreg
        ldr      \contentsreg, =__debugger_stack_bottom__
        ldr      \contentsreg, [\contentsreg, \indexreg, lsl #2]
        .endm

/* _setdbgregisterfromindex
 *      Store register contents to debugger stack given index
 *
 *      On entry:
 *        indexreg contains debugger stack index value (0-max index)
 *        contentsreg: Register Contents for given index
 *        addressreg: Scratch register for address pointer
 *      On exit:
 *        indexreg: Breakpoint index (preserved)
 *        contentsreg: Register Contents for given index
 */
        .macro  _setdbgregisterfromindex  indexreg, contentsreg, addressreg
        ldr      \addressreg, =__debugger_stack_bottom__
        str      \contentsreg, [\addressreg, \indexreg, lsl #2]
        .endm

/* _getdbgregister
 *      Retrieve register contents from debugger stack given immediate index value
 *
 *      On entry:
 *        indexval contains debugger stack index value (0-max index)
 *      On exit:
 *        contentsreg: Register Contents for given index
 */
        .macro  _getdbgregister  indexval, contentsreg
        ldr      \contentsreg, =__debugger_stack_bottom__
        ldr      \contentsreg, [\contentsreg, #(\indexval << 2)]
        .endm

/* _setdbgregister
 *      Store register contents to debugger stack given immediate index value
 *
 *      On entry:
 *        indexval contains debugger stack index value (0-max index)
 *        contentsreg: Register Contents for given index
 *        addressreg: Scratch register for address pointer
 *      On exit:
 *        contentsreg: Register Contents for given index
 *        addressreg: Destroyed
 */
        .macro  _setdbgregister  indexval, contentsreg, addressreg
        ldr      \addressreg, =__debugger_stack_bottom__
        str      \contentsreg, [\addressreg, #(\indexval << 2)]
        .endm

/* _index2bkptindex_addr
 *	Convert Breakpoint index to breakpoing entry address
 *
 *	On entry:
 *	  indexreg contains breakpoint index value
 *	On exit:
 *	  indexreg: Breakpoint index (preserved)
 *	  addrreg: Breakpoint Entry Address
 */
	.macro _index2bkptindex_addr indexreg, addrreg
	ldr	 \addrreg, =__breakpoints_start__
	add	 \addrreg, \addrreg, \indexreg, lsl #3	   /* Calculate Breakpoint Entry Address */
	.endm

/* _dbg_getstate
 *	Get Debugger State
 *	On exit:
 *	  reg: Debugger State enum
 */
	.macro _dbg_getstate	reg
	ldr	 \reg, =debug_state
	ldrb     \reg, [\reg]
	.endm

/* _dbg_setstate
 *	Set Debugger State to given value
 *	On exit:
 *	  r0, r1: destroyed
 */
	.macro _dbg_setstate	state
	mov	 r0, #\state
	ldr	 r1, =debug_state
	strb     r0, [r1]
	.endm

/* _dbg_getmode
 *	Get Debugger Mode
 *	On exit:
 *	  reg: Debugger Mode (Boolean)
 */
	.macro _dbg_getmode	reg
	ldr	 \reg, =debug_mode
	ldrb     \reg, [\reg]
	.endm

/* _dbg_setmode
 *	Set Debugger Mode to given value
 *	On exit:
 *	  r0, r1: destroyed
 */
	.macro _dbg_setmode	mode
	mov	 r0, #\mode
	ldr	 r1, =debug_mode
	strb     r0, [r1]
	.endm

/* _dbg_get_bkpt_type
 *      Get Breakpoint Type
 *      On exit:
 *        reg: Breakpoint Type
 */
        .macro _dbg_get_bkpt_type     reg
        ldr      \reg, =debug_bkpt_type
        ldrb     \reg, [\reg]
        .endm

/* _dbg_set_bkpt_type
 *      Set Breakpoint Type using value in reg
 *      On exit:
 *        reg: destroyed
 *        r1: destroyed
 */
		.macro _dbg_set_bkpt_type     reg
		ldr      r1, =debug_bkpt_type
		strb     \reg, [r1]
		.endm

/* _dbg_set_bkpt_type_val
 *      Set Breakpoint Type to given value
 *      On exit:
 *        r0, r1: destroyed
 */
        .macro _dbg_set_bkpt_type_val   bkpt_type
        mov      r0, #\bkpt_type
        ldr      r1, =debug_bkpt_type
        strb     r0, [r1]
        .endm

/* _dbg_getcurrbkpt_index
 *	Get current breakpoint index
 *	On exit:
 *	  reg: Breakpoint index
 */
	.macro _dbg_getcurrbkpt_index   reg
	ldr	 \reg, =debug_curr_breakpoint
	ldrb \reg, [\reg]
	.endm

/* _dbg_setcurrbkpt_index
 *	Set current breakpoint index
 *	On exit:
 *	  r1: destroyed
 */
	.macro _dbg_setcurrbkpt_index  reg
	ldr	 r1, =debug_curr_breakpoint
	strb \reg, [r1]
	.endm

 /*@}*/

#endif /* __DEBUG_MACROS_H__ */
