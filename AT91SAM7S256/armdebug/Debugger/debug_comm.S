/** @file debug_comm.S
 *  @brief GDB Server communications support routines
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
#include "debug_macros.h"
#include "debug_stub.h"
#include "debug_internals.h"

	.extern dbg__sendCommMsg

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


.bss
.align 4

	.global debug_InCommBuf
	.global debug_OutCommBuf
debug_InCommBuf:
	.space	USB_BUFSIZE,0
debug_OutCommBuf:
	.space	USB_BUFSIZE,0

debug_msgRxBufPtr:
	.word	0x0
debug_msgTxBufPtr:
	.word	0x0

debug_msgRxBuf_AppendPtr:
    .word   0x0
debug_msgTxBuf_AppendPtr:
    .word   0x0

    .equ    RXAPPENDPTR_OFFSET, (debug_msgRxBuf_AppendPtr - debug_msgRxBufPtr)
    .equ    TXAPPENDPTR_OFFSET, (debug_msgTxBuf_AppendPtr - debug_msgTxBufPtr)

debug_segmentRxNum:				/* Current Rx Segment Number */
	.word	0x0

/* Comm Channel and NXT Received Message Length is now common to both NxOS and NXT Firmware */
debug_nxtMsgLength:
    .word   0x0

	.global debug_nxtCommChannel
debug_nxtCommChannel:
    .word   0x0

    .global debug_nxtCommOverrun
debug_nxtCommOverrun:
    .word	0x0

    .equ    NXTCOMMCHANNEL_OFFSET, (debug_nxtCommChannel - debug_nxtMsgLength)
    .equ    NXTCOMMOVERRUN_OFFSET, (debug_nxtCommOverrun - debug_nxtMsgLength)

.data
.align 4

nxt_commcmd_header:
	.byte	NXT_GDBMSG_TELEGRAMTYPE, 0x00, 0x00		/* padded to 3 bytes */

.code 32
.text
.align 	4
/* Debugger Communications Routines
 *		It does not make sense to pass information from the Debugger Module to the Comm. link one character
 *		at a time, especially if we're not using a native serial interface (e.g., EIA-232). Consequently
 *		a Message interface has been defined. This can still call getChar() and putChar() subroutines
 *		if so desired, but it'll be a purely internal matter.
 *
 * Message Format
 *		Since we need to use EP1&2 (Bulk channels) to communicate with the PC Host, the messages should
 *		follow the NXT Direct Commands message structure (this will allow for interoperability with NXT Firmware
 *		in addition to NxOS). The maximum length of any USB communications via the Bulk channel is 64 bytes.
 *		There is a one byte Telegram Type field which identifies the type of telegram, followed by the
 *		Telegram header and actual message.
 *
 *		The LEGO Mindstorms Communications Protocol Direct Commands GDB Message format (including all headers)
 *		is as follows:
 *
 *		GDB Command
 *		===========
 *			Byte 0:	Telegram Type Field (0x8d Direct Command, No response required)	| NXT Msg Header
 *			Byte 1:	Segment No (1-255, 0: Last Segment; limit is MSG_NUMSEGMENTS)	|
 *			Byte 2:	Telegram Size (Len of USB Buffer - 3, max is MSG_SEGMENTSIZE)	|
 *			Byte 3-N: Message data													| GDB Command
 *
 *		The GDB Command (of size M) has the following format:
 *              Offset 0: '+'/'-' Command Received Status (Optional)
 *				Offset 1/0: '$'
 *				Offset 2/1: GDB Command char
 *				Offset 3 - (M-4): Command packet info
 *				Offset M-3: '#'
 *				Offset M-2: MSB of Checksum
 *				Offset M-1: LSB of Checksum
 *
 *      To be safe, we assume that the Command Received Status is always sent by the GDB server. Therefore,
 *		The maximum size of a GDB Command packet is MSGBUF_SIZE - 5 ('+'/'-', '$', '#', 2 byte checksum)
 *
 *		GDB Response
 *		============
 *			Byte 0:	Telegram Type Field (0x8d Direct Command, No response required)	| NXT Msg Header
 *			Byte 1:	Segment No (1-255, 0: Last Segment; limit is MSG_NUMSEGMENTS)	|
 *			Byte 2:	Telegram Size (Len of USB Buffer - 3, max is MSG_SEGMENTSIZE)	|
 *			Byte 3-N: Message data													| GDB Response
 *
 *      The GDB Retransmission Request has the following format:
 *              Offset 0: '-' Command Received Status
 *
 *		The GDB Response (of size M) has the following format:
 *				Offset 0: '+' Command Received Status
 *				Offset 1: '$'
 *				Offset 2 - (M-4): Response packet info
 *				Offset M-3: '#'
 *				Offset M-2: MSB of Checksum
 *				Offset M-1: LSB of Checksum
 *
 *		The maximum size of a GDB Response packet is MSGBUF_SIZE - 5 ('+', '$', '#', 2 byte checksum)
 *
 *		Note: The Telegram Size is the actual size of the Message Data portion
 *			  (i.e., excludes the three header bytes, includes the GDB Command/Response Packet checksum bytes
 *					 in the last segment)
 */

 	.global dbg__comm_init
/* dbg__comm_init
 *      Initialize communications channel.
 *      On Entry:
 *          R0: MSG Rx Buf Pointer
 *          R1: MSG Tx Buf Pointer
 */

dbg__comm_init:
	stmfd	sp!, {lr}
	ldr     r2, =debug_msgRxBufPtr
	stmia   r2!, {r0, r1}          /* debug_msgRxBufPtr and debug_msgTxBufPtr */
	stmia   r2!, {r0, r1}          /* debug_msgRxBuf_AppendPtr and debug_msgTxBuf_AppendPtr */
	bl		_dbg__comm_readbuf_reset
    ldr     r1, =debug_nxtMsgLength
    mov		r0, #0
    str     r0, [r1, #NXTCOMMCHANNEL_OFFSET]    /* Clear NXT Channel on INIT */
	ldmfd	sp!, {pc}


_dbg__comm_readbuf_reset:
    ldr     r1, =debug_nxtMsgLength
    mov     r0, #0
    str     r0, [r1]                            /* Clear Received Comm Message Length */
	bx		lr

    .global dbg__copyNxtDebugMsg
/* dbg__copyNxtDebugMsg
 *      Copy NXT Debug Message to our own Buffers, indicate Msg Received status.
 *		Note: This routine is now used by both NXT Firmware and NxOS
 *      On Entry:
 *          R0: NXT Input Buf Pointer
 *          R1: NXT Communications Channel Enum (CmdBit)
 *          R2: NXT Raw Message Length
 *      On Exit:
 *          R0-R3: Destroyed
 */
dbg__copyNxtDebugMsg:
    ldr     r3, =debug_nxtMsgLength
    str     r1, [r3, #NXTCOMMCHANNEL_OFFSET]	/* save Communications Channel first */
    ldr		r1, [r3]							/* Check if there's an unread message in the buffer */
    cmp		r1, #0
    beq		cont_dbg__copyNxtDebugMsg			/* No unread message, so continue */
exit_dbg__NxtDebugMsgOverrun:
	ldr		r1, [r3, #NXTCOMMOVERRUN_OFFSET]
	add		r1, r1, #1
	str		r1, [r3, #NXTCOMMOVERRUN_OFFSET]	/* update message overrun stats */
    b		exit_dbg__copyNxtDebugMsg
cont_dbg__copyNxtDebugMsg:
    str     r2, [r3]
    ldr     r1, =debug_InCommBuf
    _dbg_memcpy r1, r0, r2, r3                  /* r3: scratch register */
exit_dbg__copyNxtDebugMsg:
    bx      lr

/* _dbg_reset_msgTxBuf_AppendPtr
 *      Internal variable to reset pointers.
 *      On Exit:
 *          R0: debug_msgTxBuf_AppendPtr
 *          R1: destroyed
 */
_dbg_reset_msgTxBuf_AppendPtr:
    ldr     r1, =debug_msgTxBufPtr              /* Should not be modified */
    ldr     r0, [r1]
    str     r0, [r1, #TXAPPENDPTR_OFFSET]
    mov     pc, lr

/* _dbg__commHasMsg
 *		Internal Segment Reassembly Routine.
 *		On exit:
 *			r0: !0: (Availale Telegram Message Size), 0: no incoming message/segment
 *			r1: message segment number
 */
_dbg__commHasMsg:
	stmfd	sp!, {lr}
    ldr     r0, =debug_nxtMsgLength
    ldr     r0, [r0]                /* R0 contains the Comm Buffer Size, including the NXT Direct Command Header */

	ldr		r2, =debug_InCommBuf
	ldrb    r1, [r2, #NXT_MSG_TELEGRAMTYPE_OFFSET]
	cmp     r1, #NXT_GDBMSG_TELEGRAMTYPE
	bne     invalid_CommMsg          /* Invalid telegram type, ignore */

	ldrb	r1, [r2, #NXT_MSG_TELEGRAMSIZE_OFFSET]
	sub     r0, r0, r1              /* Comm Buffer Size - Telegram Size = 3 (header size) */
	cmp     r0, #NXT_GDBMSG_START   /* Start offset is equal to header size */
    bne     invalid_CommMsg          /* Invalid Message Length, ignore */

    mov     r0, r1                  /* Telegram Message Size */
	ldrb	r1, [r2, #NXT_MSG_SEGNUM_OFFSET]
    b       _exit_dbg__commHasMsg

invalid_CommMsg:
    bl      _dbg__comm_readbuf_reset /* Next Comm telegram transaction */
    mov     r0, #0
_exit_dbg__commHasMsg:
    ldmfd   sp!, {pc}

/* _copy_msg_from_commbuf
 *      Internal Comm buffer copy routine, handles segment reassembly.
 *      On entry:
 *          r0: number of bytes to copy
 *          r1: segment number
 *      On exit:
 *          r0: cummulative message length
 *          r1: segment number
 *          r2, r3: Destroyed
 */
_copy_msg_from_commbuf:
    stmfd   sp!, {r1,r4,r5,r6,lr}
    movs    r4, r0
    beq     _exit_copy_msg_from_commbuf

    ldr     r6, =debug_msgRxBufPtr              /* Address of Pointers */
    ldr     r5, [r6]                            /* Rx buffer Start Address */
    ldr     r2, [r6, #RXAPPENDPTR_OFFSET]       /* Append Pointer */

    sub     r3, r2, r5                          /* r3: current length of message */
    add     r3, r3, r4                          /* new cummulative length of message */
    cmp     r3, #MSGBUF_SIZE
    movhi   r4, #0                              /* Buffer overflow! */
    strhi   r5, [r6, #RXAPPENDPTR_OFFSET]       /* Reset AppendPtr to beginning of Rx Buffer */
    bhi     _exit_copy_msg_from_commbuf

    ldr     r3, =debug_InCommBuf
    add     r3, r3, #NXT_GDBMSG_START
    _dbg_memcpy r2, r3, r4, r0                  /* r2 updated to point to next empty char slot in Rx buffer */
    sub     r4, r2, r5                          /* r4: cummulative length of message */

    /* Update debug_msgRxBuf_AppendPtr */
    teq     r1, #0                              /* Check if this is last segment (segment 0) */
    streq   r5, [r6, #RXAPPENDPTR_OFFSET]       /* Reset AppendPtr to beginning of Rx Buffer if so */
    strne   r2, [r6, #RXAPPENDPTR_OFFSET]       /* Otherwise, update Append Pointer to receive next segment */

_exit_copy_msg_from_commbuf:
    bl      _dbg__comm_readbuf_reset             /* Next Comm telegram transaction */
    mov     r0, r4                              /* Return cummulative message length in R0 */
    ldmfd   sp!, {r1,r4,r5,r6,pc}               /* Return segment number in R1 */


/* _msgbuf_checksum
 *      Internal routine to calculate checksum character buffer.
 *      On entry:
 *          r0: pointer to character buffer to checksum (assume ASCIIZ terminated)
 *      On exit:
 *          r0: pointer to character buffer after ASCIIZ
 *          r1: checksum (8-bit binary)
 *          r2: message length
 *          r3: destroyed
 */
_msgbuf_checksum:
    mov     r1, #0                              /* clear checksum */
    mov     r2, #0                              /* clear length */
1:  ldrb    r3, [r0], #1                        /* Iterate through buffer */
    add     r1, r1, r3                          /* cummulative sum of char */
    teq     r3, #0
    addne   r2, r2, #1                          /* increment message length */
    bne     1b                                  /* until ASCIIZ found */
    and     r1, #BYTE0                          /* Modulo 256 */
    mov     pc, lr

    .global dbg__getDebugMsg
/* dbg__getDebugMsg
 *      Retrieve pending Debugger Message if available (Non-Blocking).
 *      On entry:
 *          No parameters (assume pointers were initialized previously using dbg__comm_init)
 *      On exit:
 *          r0: >0 = Valid GDB Message Length (incl '$', excluding '#' and checksum),
 *              0 = no valid message (yet), -1 = error
 *          r1: GDB Message Buffer Pointer  (incl '$', excluding '#' and checksum)
 *          r2, r3: Destroyed
 *      Note: If GDB Message were returned, it is ASCIIZ terminated, does not include '#' and checksum
 */
dbg__getDebugMsg:
    stmfd   sp!, {r4,r5,lr}
    bl      _dbg__commHasMsg                     /* r0: message length, r1: segment number */
    teq     r0, #0
    beq     exit_dbg__getDebugMsg               /* no new message, exit with R0 = 0 */

    ldr     r4, =debug_segmentRxNum
    ldr     r2, [r4]                            /* Get current Segment Number */
    add     r2, r2, #1                          /* Expected Segment Number for comparison */
    teq     r1, #0
    streq   r1, [r4]                            /* Update current Segment Number with 0 since it is the last segment */
    beq     _hasMsg2Copy
    cmp     r1, #MSG_NUMSEGMENTS                /* Segment Number < MSG_NUMSEGMENTS? */
    bhs     _invalid_segment
    teq     r1, r2                              /* Valid Segment Number, check against Expected Segment Number */
    beq     _hasMsg2Copy                        /* Segment Number matches Expected Segment Number, update buffers */

_invalid_segment:
    bl      _dbg__comm_readbuf_reset             /* Invalid, Next Comm telegram transaction */
    mov     r0, #0                              /* Reset Segment Number */
    str     r0, [r4]                            /* Update current Segment Number with 0 to prepare for new message */
    b       exit_dbg__getMsgError               /* Exit with error */

_hasMsg2Copy:
    str     r1, [r4]                            /* Update current Segment Number */
    bl      _copy_msg_from_commbuf               /* r0: cummulative message length, r1: segment number */
    teq     r1, #0
    movne   r0, #0                              /* Incomplete message, ignore for now */
    bne     exit_dbg__getDebugMsg               /* Message not complete yet, exit */

    /* Check for valid GDB message */
    mov     r4, r0                              /* keep message length in R4, assume to be within MSGBUF_SIZE */
    ldr     r5, =debug_msgRxBufPtr
    ldr     r5, [r5]                            /* Rx buffer Start Address */

/* Need to account for Packet Acknowledgement */
1:  ldrb    r0, [r5]
    teq     r0, #MSGBUF_CTRLC                 /* Look for Ctrl-C */
    moveq	r0, r4							  /* If found, set R0 to current message length */
    beq		exit_dbg__getDebugMsg			  /* and return  */
    teq     r0, #MSGBUF_NAKCHAR               /* Look for '-' */
    beq		exit_dbg__getMsgError			  /* Error from Host, Retransmit previous message */
    teq     r0, #MSGBUF_ACKCHAR               /* Look for '+' */
    addeq   r5, r5, #1                        /* Adjust Buffer Start Pointer (excl '+') */
    subeq	r4, r4, #1						  /* Adjust Message Length */
    beq     1b                                /* Skip all Packet Acknowledgements */

	/* Note: Here we assume that we won't get a single ACK '+' or NAK '-' character message.
	 *       If we do, it'll be flagged as an error
	 */
    subs    r2, r4, #MSGBUF_CHKSUMOFFSET        /* Look for '#': Message Length - 3 = '#' offset */
    blt		exit_dbg__getMsgError				/* Message Length is too short, exit with error */
    ldrb    r0, [r5, r2]
    teq     r0, #MSGBUF_CHKSUMCHAR
    bne     exit_dbg__getMsgError               /* No checksum char '#', exit with error */

    mov     r1, #0
    strb    r1, [r5, r2]                        /* Zero out '#' char for checksum calc later */

#ifdef CHECK_GDBSTARTCHAR
    /* Checked in dbg__bkpt_waitCMD */
    ldrb    r0, [r5]
    teq     r0, #MSGBUF_STARTCHAR               /* Look for '$' */
    bne     exit_dbg__getMsgError               /* No start char '$', exit with error */
#endif

    add     r0, r5, #1                          /* Checksum packet data (excl '$') */
    bl      _msgbuf_checksum                    /* R2: length (excl '$'), R1: calculated checksum, R0: pointer to checksum in receive buffer */
    mov     r3, r1                              /* Keep calculated checksum in R3 (R1 destroyed by ascii2byte) */
    bl      ascii2byte                          /* R0: received checksum, R1: address of next buffer location */
    teq     r0, r3                              /* Compare calculated checksum in R3 against received checksum in R0 */
    bne     exit_dbg__getMsgError               /* Checksums do not match, exit with error */

    subeq   r0, r4, #MSGBUF_CHKSUMOFFSET        /* Update message length (incl '$') as return parameter */
    add     r2, r2, #1                          /* expected message length (from _msgbuf_checksum) */
    teq     r0, r2
    beq     exit_dbg__getDebugMsg               /* Valid length, return */

exit_dbg__getMsgError:
	/* We must first clear the existing message checksum */
    ldr     r1, =debug_msgTxBufPtr              /* R5: data structure base pointer */
    ldr     r1, [r1]                            /* Tx buffer Start Address */

1:	ldrb	r0, [r1], #1
	teq		r0, #MSGBUF_CHKSUMCHAR
	bne		1b

	mov		r0, #0								/* ASCIIZ */
	strb	r0, [r1, #-1]						/* Pointer R1 is now one past the MSGBUF_CHKSUMCHAR */

	bl		dbg__putDebugMsg					/* Retransmit message */
	mov		r0, #0								/* Flag no message received */

#if 0
    mov     r0, #MSGBUF_MSGERROR
#endif

exit_dbg__getDebugMsg:
    mov     r1, r5                              /* Return GDB Message Buffer Pointer in R1 */
	ldmfd	sp!, {r4,r5,pc}


/* _copy_msg_to_commbuf
 *      Internal Comm buffer copy routine, handles segment fragmentation.
 *      On entry:
 *          r0: number of bytes to copy
 *          r1: segment number
 *      On exit:
 *          r0: cummulative message length
 *          r1: segment number
 *          r2, r3: Destroyed
 */
_copy_msg_to_commbuf:
    stmfd   sp!, {r1,r4,r5,r6,lr}
    ldr     r6, =debug_msgTxBufPtr              /* Address of Pointers */
    ldr     r5, [r6, #TXAPPENDPTR_OFFSET]       /* Retrieve Tx Append Pointer */

    movs    r4, r0
    beq     _exit_copy_msg_to_commbuf

#ifdef CHECK_TXLEN
    add     r0, r4, #NXT_GDBMSG_START   /* offset = header size */
    cmp     r0, #USB_BUFSIZE
    bhi     _exit_copy_msg_to_commbuf    /* We let calling routine detect problem (segment number will increment) */
#endif

    /* Fill in Comm Message Header */
    ldr     r3, =debug_OutCommBuf
    mov     r2, #NXT_GDBMSG_TELEGRAMTYPE
    strb    r2, [r3], #1            /* Telegram type */
    strb    r1, [r3], #1            /* Segment Number */
    strb    r0, [r3], #1            /* Message Length */

    mov     r2, r5                              /* Copy to R2 for updating */
    mov     r1, r4                              /* actual GDB message fragment length (exclude Comm header) */
    _dbg_memcpy r3, r2, r1, r0                  /* This copies over the message fragment, r3, r2 updated */
    mov     r5, r2                              /* Updated Tx Append Pointer, keep in R5 for now */

    add     r0, r4, #NXT_GDBMSG_START           /* Total Comm Buffer Size for Tx (NXT_GDBMSG_START offset = header size) */
    bl      dbg__sendCommMsg                    /* Common interface routine to commnuncations drivers */
    cmp     r0, #TRUE
    ldrne   r5, [r6, #TXAPPENDPTR_OFFSET]       /* Tx failed, Retrieve Original Tx Append Pointer */
    streq   r5, [r6, #TXAPPENDPTR_OFFSET]       /* Tx succeeded, Update Tx Append Pointer to new position */

_exit_copy_msg_to_commbuf:
    ldr     r6, [r6]                            /* Retrieve Tx Buffer Start Address */
    sub     r0, r5, r6                          /* Return calculated cummulative message length (R0) */
    ldmfd   sp!, {r1,r4,r5,r6,pc}               /* Return segment number in R1 */

	.global dbg__putDebugMsg
/* dbg__putDebugMsg
 *		Sends Debugger Message from calling routine after appending checksum (Blocking) .
 *		On entry:
 *			No parameters (assume pointers were initialized previously using dbg__comm_init)
 *		On exit:
 *			r0: status (0: success, -1: error)
 *      Note: GDB Message to be sent must be ASCIIZ terminated, does not include '#' and checksum
 *            Response packets start with '+' followed by '$' (2 bytes prefix)
 */
dbg__putDebugMsg:
	stmfd	sp!, {r4,r5,lr}
	/* Perform Checksum Calculation */
    ldr     r5, =debug_msgTxBufPtr              /* R5: data structure base pointer */
    ldr     r4, [r5]                            /* Tx buffer Start Address */
    str     r4, [r5, #TXAPPENDPTR_OFFSET]       /* Reset Tx buffer Append Pointer */
    add     r0, r4, #2                          /* skip '+' and '$' */
    bl      _msgbuf_checksum                    /* R2: length (excl '+' and '$'), R1: calculated checksum, R0: pointer to checksum in tx buffer */

#ifdef CHECK_TXLEN
    add     r2, r2, #2                          /* r2: returned length from _msgbuf_checksum, added with prefix length */
    sub     r3, r0, r4                          /* r3: calculated length from pointers (incl. prefix length) */
    teq     r2, r3
    bne     exit_dbg__putMsgError
#endif

    mov     r3, #MSGBUF_CHKSUMCHAR
    strb    r3, [r0, #-1]                       /* Insert '#' */
    bl      byte2ascii                          /* On return, R0 points to location after checksum bytes, R1 is original pointer to checksum */
    sub     r4, r0, r4                          /* R4 = Calculated total message length (incl '+' and '$', '#' and checksum bytes */
    cmp     r4, #MSG_SEGMENTSIZE                /* If total message length > MSG_SEGMENTSIZE */
    mov     r1, #0                              /* Initialize Segment Number = 0 (last segment) first */
    mov     r0, #0                              /* Initial cummulative message length */
    mov     r5, #0                              /* Previous cummulative message length */

    /* We assume unsigned message lengths, so the arithmetic MUST NOT result in negative values */
_cont_putMsg:
    cmp     r4, r0
    movls   r0, #0                              /* R0: Exit status (success) */
    bls     exit_dbg__putDebugMsg               /* If Total message length (r4) <= Cummulative message length (r0), we're done */
    add     r2, r0, #MSG_SEGMENTSIZE            /* R2: calculate new Max cummulative message length */
    cmp     r4, r2                              /* Check total message length (R4) against new Max cummulative message length (R2) */
    subls   r0, r4, r0                          /* if total message length (R4) <= new Max cummulative message length (R2), send remainder */
    movls   r1, #0                              /* Flag as last segment (Segment Number = 0) */
    movhi   r0, #MSG_SEGMENTSIZE                /* else send MSG_SEGMENTSIZE bytes */
    addhi   r1, r1, #1                          /* Increment Segment Number */
    cmp     r1, #MSG_NUMSEGMENTS
    bhs     exit_dbg__putMsgError               /* If Segment Number >= MSG_NUMSEGMENTS, flag error */
    bl      _copy_msg_to_commbuf                 /* R0: cummulative message length, R1: segment number */
    teq     r5, r0                              /* Check if we managed to transmit the previous message */
    beq     exit_dbg__putMsgError               /* No, flag error */
    movne   r5, r0                              /* Update previous cummulative message length */
    b       _cont_putMsg

exit_dbg__putMsgError:
    mov     r0, #MSGBUF_MSGERROR
exit_dbg__putDebugMsg:
	ldmfd	sp!, {r4,r5,pc}

	.global dbg__sendAckOrNak
/* dbg__sendAckOrNak
 *		Send Ack (for successful receipt of message)
 *			or Nak (for Retransmission due to received message Checksum error) (Blocking) .
 *		On entry:
 *			No parameters (assume pointers were initialized previously using dbg__comm_init)
 *		On exit:
 *			r0: status (0: success, -1: error)
 *			r1: destroyed
 *      Note: An Ack Or Nak is indicated by '+' or '-', which is prepended with the Comm header and sent (without checksum)
 *			  Sending Ack is only done for Continue and Step commands, where GDB does not expect any replies.
 */
dbg__sendAckOrNak:
	stmfd	sp!, {lr}
    ldr     r1, =debug_msgTxBufPtr              /* R2: data structure base pointer */
    ldr     r0, [r1]                            /* Tx buffer Start Address */
    str     r0, [r1, #TXAPPENDPTR_OFFSET]       /* Reset Tx buffer Append Pointer */

	mov     r1, #0                              /* Initialize Segment Number = 0 (last segment) */
    mov     r0, #1                              /* Retransmission message length = 1 */
    bl      _copy_msg_to_commbuf                 /* R0: cummulative message length, R1: segment number */
    cmp     r0, #1                              /* Check if we managed to transmit the previous message */
    moveq	r0, #0								/* R0: Exit status (success) */
    movne	r0, #MSGBUF_MSGERROR				/* R0: Exit status (error) */
	ldmfd	sp!, {pc}

