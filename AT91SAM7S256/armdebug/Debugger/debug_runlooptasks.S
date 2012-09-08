/** @file debug_runlooptasks.S
 *  @brief GDB Server platform Run Loop
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

/*
 * This file contains platform specific code.
 * This include ABORT Mode Debugger Run Loop operation,
 * as well as Debugger Interfacing code to the platform code.
 */

/*
 * The Debugger has to implement a Run Loop in ABORT mode
 * since the hardware is still running. Consequently,
 * the communications subsystems such as USB (and Bluetooth?)
 * which is used to communicate with the Host needs to be
 * serviced in order for actual data transmission and reception
 * to take place. Currently we're reusing the platform's
 * communication routines to do the actual tx/rx, so it means
 * that it is not possible to set breakpoints in those modules.
 * In addition, since the platform communication modules may
 * handle other tasks, it is currently possible to enter an
 * indeterminate state where certain communication messages trigger
 * a platform response which cannot be handled by the Debugger Run Loop.
 * The alternative is to implement our own communications routines, but
 * that will take even more code.
 *
 * FIXME: It may become necessary to hack the platform communications
 * routines to detect that we're in the Debugger Run Loop and not the
 * normal run loop to avoid system crashes, but the current goal is to
 * have as minimal changes to the platform code as possible.
 *
 * Since there are two Run Loops for the platform, the way in which
 * they interact is as follows:
 *
 * [Platform Run Loop]  - DBG_INIT/ GDB Cmd/ BKPT ->  [Debugger Run Loop]
 *                    \ <------ GO/ STEP/ CONT ----- /
 *        ...                                                 ...
 *        ...                                         Handle GDB Cmd/Resp
 *        ...                                                 ...
 *   {normal runloop                                  {communications /
 *    processing}                                      watchdog routines}
 * ^-------v                                                      v-------^
 *
 * The Platform will invoke dbg__bkpt_init() after hardware and system initialization,
 * before entering the Platform Run Loop. This configures the Debugger, but does not
 * invoke the Debugger Run Loop unless a Manual Breakpoint is found in the platform code.
 *
 * Subsequently, the Debugger Run Loop will be triggered by Breakpoints, or
 * when the communications subsystem receives a GDB Command.
 *
 * The Debugger Run Loop is actually dbg__bkpt_waitCMD(), this file contains
 * the Run Loop Tasks which needs to be invoked periodically by the Run Loop,
 * to minimize the coupling between the ARMDEBUG modules and the Platform.
 *
 * Note: The Debugger Run Loop does not handle Hardware Shutdown, it is
 * assumed that we wouldn't need to do so in Debug Mode.
 *
 */
#define __ASSEMBLY__

#define REBOOT_POWERDOWN
#include "debug_runlooptasks.h"

#include "debug_internals.h"
#include "debug_macros.h"
#include "debug_stub.h"

	.code 32
	.align 4

#ifdef __NXOS__
/****************************************************************************
 *
 * NxOS Run Loop
 *
 ****************************************************************************/
	dbg_interwork dbg__runloopTasks
/* Currently, there's nothing that needs to be done in the NxOS Run Loop */
	push	{lr}
	mov		r0, #1				/* 1 ms delay */
	bl		nx_systick_wait_ms
    pop		{pc}

#else
/****************************************************************************
 *
 * NXT Firmware Run Loop
 *
 ****************************************************************************/
	dbg_interwork dbg__runloopTasks
    push    {lr}
    /* FIXME: Add necessary cXXXCtrl calls here */
    bl      cCommCtrl
    /* OSWatchdogWrite is a NULL function in the NXT Firmware?! */
    pop     {pc}
#endif

#ifdef __NXOS__
/****************************************************************************
 *
 * NxOS Reboot Routine
 *
 ****************************************************************************/
	dbg_interwork dbg__reboot
#ifdef REBOOT_POWERDOWN
	b	    nx_core_halt		/* Shutdown Brick, won't return */
#else
	b	    nx_core_reset		/* Reboot Brick, won't return */
#endif

#else
/****************************************************************************
 *
 * NXT Firmware Reboot Routine
 *
 ****************************************************************************/
	dbg_interwork dbg__reboot
	/* Powerdown Sequence
      	dIOCtrlSetPower((POWERDOWN>>8));
		dIOCtrlTransfer();

	   Reboot Sequence
      	dIOCtrlSetPower((UBYTE)(BOOT>>8));
      	dIOCtrlSetPwm((UBYTE)BOOT);
  		dIOCtrlTransfer();
	*/

	/* We implement the powerdown sequence for now */

#ifdef REBOOT_POWERDOWN
	/* Powerdown sequence */
	ldr		r0, =((POWERDOWN >> 8) & 0xFF)
	ldr		r1, =dIOCtrlSetPower
	mov		lr,pc
	bx		r1
#else
	/* Reboot sequence: this forces SAMBA mode??!! */
	ldr		r0, =((BOOT >> 8) & 0xFF)
	ldr		r1, =dIOCtrlSetPower
	mov		lr,pc
	bx		r1

	ldr		r0, =(BOOT & 0xFF)
	ldr		r1, =dIOCtrlSetPwm
	mov		lr,pc
	bx		r1
#endif

_dbg__reboot_wait:
	ldr		r1, =dIOCtrlTransfer
	mov		lr,pc
	bx		r1

	b		_dbg__reboot_wait						/* Wait for AVR... */
#endif

#ifdef __NXOS__
/****************************************************************************
 *
 * NxOS Abort Info LCD Display Routine
 *
 ****************************************************************************/
/*		On entry:
 *			r0: abort type
 *		On exit:
 *			r0-r3: destroyed
 */
	dbg_interwork dbg__display_abort_info
	push	{lr}
    _getdbgregister DBGSTACK_USERPC_INDEX, r1           /* Retrieve User PC into R2 */
    _getdbgregister DBGSTACK_USERCPSR_INDEX, r2         /* Retrieve User CPSR into R2 */
	bl		nx__abort_info		/* void nx__abort_info(U32 data, U32 pc, U32 cpsr); */
    pop		{pc}

#else
/****************************************************************************
 *
 * NXT Firmware Abort Info LCD Display Routine
 *
 ****************************************************************************/
	dbg_interwork dbg__display_abort_info
/* FIXME: Inteface with NXT Firmware LCD Display routines */
    push    {lr}
    pop     {pc}
#endif

#ifdef __NXOS__
	.extern debug_OutCommBuf
/****************************************************************************
 *
 * NxOS Communications Driver Interface Routine
 *
 ****************************************************************************/
/* dbg__sendCommMsg
 *      Internal send routine (interfaces with drivers).
 *      On entry:
 *          R0: Total Message Buffer length
 *      On exit:
 *           R0: Tx Status (TRUE if data sent)
 *			 R1-R3: Destroyed
 */
	dbg_interwork dbg__sendCommMsg
    stmfd   sp!, {r4, lr}
    mov     r4, r0                              /* Keep Comm Buffer length in R4 */
    /* Check USB bus status, transmit message if possible */
    bl      nx_usb_is_connected                 /* R0 = TRUE (#1) if USB is ready */
    teq     r0, #0								/* FALSE == #0;
    											   We can't check for True condition since values
    											   used by C-Compiler & ARMDEBUG are different */
    beq     dbg__sendCommMsgFailed

    /* Actual transmission (blocking) */
    ldr     r0, =debug_OutCommBuf                /* data pointer parameter */
    mov     r1, r4                              /* Comm buffer length */
    bl      nx_usb_write

1: 	bl      nx_usb_data_written                 /* R0 = True if data has been sent */
    teq     r0, #0								/* FALSE == #0;
    											   We can't check for True condition since values
    											   used by C-Compiler & ARMDEBUG are different */
    /* FIXME: implement timeout */
    beq     1b                                  /* Busy Wait Loop */

    mov     r0, #TRUE
    b		exit_dbg__sendCommMsg
dbg__sendCommMsgFailed:
    mov     r0, #FALSE

exit_dbg__sendCommMsg:
    ldmfd   sp!, {r4, pc}


#else
/****************************************************************************
 *
 * NXT Firmware Communications Driver Interface Routine
 *
 ****************************************************************************/
/* dbg__sendCommMsg
 *      Internal send routine (interfaces with drivers).
 *      On entry:
 *          R0: Total Message Buffer length
 *      On exit:
            R0: Tx Status (TRUE if data sent)
 */
	dbg_interwork dbg__sendCommMsg
    stmfd   sp!, {r4, lr}
    mov     r4, r0                              /* Keep Comm Buffer length in R4 */
    ldr     r0, =debug_nxtCommChannel
    ldr     r0, [r0]                            /* Get Channel Enum */
    teq     r0, #BT_CMD_READY
    beq     dbg__sendBTMsg
    teq     r0, #USB_CMD_READY
    beq     dbg__sendUSBMsg
    b       dbg__sendCommMsgFailed                 /* Channel Enum Doesn't Match, shouldn't happen? */

dbg__sendBTMsg:
    /* NXT BT routines do not have any configuration checks */
    ldr     r0, =debug_OutCommBuf                /* data pointer parameter */
    mov     r1, r4                              /* BT Bytes to Send */
    mov     r2, r4                              /* BT Message Size */
    bl      dBtSendMsg                          /* Send it via Bluetooth (complete message) */
    mov     r0, #TRUE                           /* Always flag Success */
    b       exit_dbg__sendCommMsg

dbg__sendUSBMsg:
    /* Check USB bus status, transmit message if possible */
    bl      dUsbIsConfigured                    /* R0: UByte status, TRUE / FALSE */
    teq     r0, #nxt_UBYTE_TRUE
    bne     dbg__sendCommMsgFailed

    /* Actual transmission (blocking) */
    ldr     r0, =debug_OutCommBuf                /* data pointer parameter */
    mov     r1, r4                              /* Comm buffer length */
    bl      dUsbWrite                           /* call NXT Firmware USB driver, return 0: done, !0: remaining chars */
    teq     r0, #0                              /* Tx done if returned length is 0 */
    moveq   r0, #TRUE                           /* Convert NXT firmware return value to our Status (TRUE/FALSE) */
    beq     exit_dbg__sendCommMsg
dbg__sendCommMsgFailed:
    mov     r0, #FALSE

exit_dbg__sendCommMsg:
    ldmfd   sp!, {r4, pc}
#endif


#ifdef __NXOS__
/****************************************************************************
 *
 * GDB Debugger Invocation Routine for NxOS
 *
 ****************************************************************************/
	.code 32
	.align 4

	.extern dbg__install_singlestep
	.extern dbg__activate_singlestep
	.extern irq_stack_frame_address
/* nxos__handleDebug
 * 		Prepare to switch to Debug Mode
 * int nxos__handleDebug(U8 *buffer, comm_chan_t channel, U32 length);
 *
 * This routine is called from NxOS Fantom library to setup
 * Single Step Breakpoint in preparation for Debugger invocation if we're in
 * normal execution mode.
 *
 * It returns to complete the IRQ handling normally, after which the single
 * step breakpoint will be triggered, and the incoming GDB message will then
 * be processed in the dbg__bkpt_waitCMD() loop.
 *
 * If we're in Debugger Mode already, then just return and let the
 * dbg__bkpt_waitCMD() loop handle it normally.
 *
 * If we're operating in normal NxOS mode, return True (!0)
 * If we're already in Debugger Mode, return False (0)
 */
	dbg_interwork nxos__handleDebug
		push {lr}
		/* This routine is called from nx__irq_handler() via fantom_filter_packet().
		 * The operating mode should already have been configured by the IRQ interrupt handler.
		 *
		 * The IRQ Stack Frame Pointer will contains the LR and SPSR from the topmost interrupted task
		 * if it is non-zero (NxOS supports nested IRQs)
		 */
        bl    dbg__copyNxtDebugMsg  	/* Copy to Debugger Message Buffer, Remember Comm Channel */
 		mov		r0, #FALSE				/* Setup Default Return value (False) */
        _dbg_getmode r1             	/* Get Debug Mode */
        cmp   r1, #(TRUE & BYTE0)		/* Confine it to Byte size */
        /* If Debug Mode is TRUE, this means that we're already running the Debugger */
 		beq		exit_nxos__handleDebug	/* Yes, return False */

		/* Retrieve ISR Return Address */
        ldr     r3, =irq_stack_frame_address
        ldr		r3, [r3]				/* Get Interrupt Stack Pointer */
        teq		r3, #0
        beq		exit_nxos__handleDebug	/* NULL Interrupt Stack Frame Pointer, exit (status: False) */

nxos_switch2debug:
		/* Since the Interrupt Stack Frame Pointer points to the top of the stack frame,
		 * we'll have to use Load Empty Ascending Stack (LDMEA == LDMDB) to access the variables
		 */
        ldmdb   r3, {r1,r2}				/* R1: LR, R2: SPSR */
 		tst		r2, #CPSR_THUMB			/* Check for Thumb Mode */
 		orrne	r1, r1, #1				/* Configure for Thumb Single Step Breakpoint */
	    bl  	dbg__install_singlestep /* Setup Single Step, next instruction address returned in r1 */
		bl		dbg__activate_singlestep
		mov	    r0, #TRUE				/* We're going to switch to Debug Mode (via Single Step Breakpoint) */

exit_nxos__handleDebug:
		pop	  {r1}
    	bx    r1						/* In case we have Interworking from different caller mode */

#else

/****************************************************************************
 *
 * GDB Debugger Invocation Routine for NXT Firmware
 *
 ****************************************************************************/
    .code 16
    .align 2

    .extern dbg__copyNxtDebugMsg
    .global cCommHandleDebug
    .thumb_func
    .type cCommHandleDebug, %function
/* cCommHandleDebug
 * Switch Mode to Debugger.
 *      Used by NXT Firmware only
 *
 * UWORD cCommHandleDebug(UBYTE *pInBuf, UBYTE CmdBit, UWORD MsgLength);
 *
 * This routine is called from cCommInterprete either in normal operation mode (SVC)
 * or else when we're in debug mode (ABORT) which uses the cCommCtrl() routine to handle
 * I/O with the Host.
 *
 * On entry, the message is copied from the NXT buffer into our own buffers.
 *
 * If this is accessed from normal operation mode, we need to switch mode to
 * ABORT mode to handle the incoming message using a Manual Breakpoint instruction.
 * When DEBUG is exited, the execution resumes from the instruction following the Breakpoint.
 */
cCommHandleDebug:
/* Arg Registers are not preserved since this is invoked explicitly */
        push  {lr}                      /* store arg registers */
        bl    dbg__copyNxtDebugMsg  	/* Copy to Debugger Message Buffer, Remember Comm Channel */
        _dbg_getmode r0             	/* Get Debug Mode */
        cmp   r0, #(TRUE & BYTE0)		/* Confine it to Byte size */

        /* If Debug Mode is TRUE, this means that we're already running the Debugger */
		beq	  _cCommHandleDebug_cont
        /* Else, we're in normal operation mode (SVC), or other mode (??!) and need to force a switch to Debug mode */
        dbg__bkpt_thumb
_cCommHandleDebug_cont:
        mov   r0, #0                   /* FIXME: Return Status */
        pop	  {r1}					   /* Can't Pop LR directly */
        bx    r1					   /* Safe code: actually we should be able to Pop PC since the caller is Thumb Mode */

        .ltorg
#endif
